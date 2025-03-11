#include <iostream>
#include <fstream>
#include <string.h>
#include <filesystem>
#include "Solution.h"
#include "MDEntry.h"
// Algorithm overview:
// Since the files are already sorted, the algorithm is to merge pairs of files
// using the "merge" step used in the merge sort, i.e., select the "lesser" of the 
// curr lines in the 2 files and then push it into the outfile. So, in every thread:
// 1. If the input file queue has only 1 file, then stop, this file is theresult file
// 2. Pop 2 files from input file queue
// 3. Merge the files, put the merge result in an outfile 
// 4. push this intermediate outfile agin in the input queue
// 5. Repeat steps 1- 4
// 
// One thing to note is that the input file queue is accessed in multiple threads
// So pusing into and popping from this file is to be done within a critical section 


// The intermediate files are kept in the format:
// Symbol, Timestamp, Price, Size, Exchange, Type
// But the original given input files are in the format:
// Timestamp, Price, Size, Exchange, Type
// To cater this difference, we need this wrapper
// "symbol" for intermediate files is passed as ""
// but for original input files, it's passed as the name
// of the symbol the input file is made for
uint32_t readNextLine(const FileLineReader& reader,
                      char* buff,
                      const std::string& symbol)
{
  
  uint32_t ret = 0;

  uint32_t offSet = symbol.length() ? symbol.length() + 2 : 0;
  char* fileBuff = buff + offSet;
  if (uint32_t bytesReadFromFile = reader(fileBuff); bytesReadFromFile)
  {
    ret = offSet + bytesReadFromFile;
    if (fileBuff[bytesReadFromFile - 1] != '\n')// EOF was reached
    {
      fileBuff[bytesReadFromFile] = '\n';
      ++ret;
    }

    if (offSet)
    {
      memcpy(buff, symbol.c_str(), symbol.length());
      memcpy(buff + symbol.length(), ", ", 2);
    }
  }

  return ret;
}

// Get the filename for intermediate files
std::string generateRandomString(const int len) {
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::string res;
  res.reserve(len);

  for (int i = 0; i < len; ++i) {
      res += alphanum[rand() % (sizeof(alphanum) - 1)];
  }
  
  return res;
}

bool mergeFiles(const std::function<std::optional<MergeFilePair>()> filenameFetcher, 
                const FileReaderProvider fileReaderProvider,
                const FileWriterProvider fileWriterProvider,
                const MergeNotificationHandler outFileNotifier,
                const uint64_t maxHeapSize)
{
  // Allocate in big chunks instead of allocaing 
  // small chunks several times to avoid repeated system calls and hence
  // improve performance
  // 
  // "buff" is the buffer that holds the intermidate result of the merge
  auto fileNames = filenameFetcher();
  if (!fileNames)
  {
    return false;
  }

  auto const& [fn1, fn2] = fileNames.value();
  std::string symbol1 = "";
  std::string symbol2 = "";
  symbol1.reserve(fn1.length() + 1);
  symbol2.reserve(fn2.length() + 1);

  if (std::size_t pos = fn1.find_first_of("."); 
      strcmp(".csv", fn1.c_str() + pos) != 0
    )
  {
    symbol1 = std::move(fn1.substr(0, pos));
  }

  if (std::size_t pos = fn2.find_first_of("."); 
      strcmp(".csv", fn2.c_str() + pos) != 0
    )
  {
    symbol2 = std::move(fn2.substr(0, pos));
  }

  auto fileReader1 = fileReaderProvider(fn1, maxHeapSize/2);
  auto fileReader2 = fileReaderProvider(fn2, maxHeapSize/2);

  //Lines read from each of the files
  char l1[256];
  char l2[256];

  // Bytes in the curr line in each file
  readNextLine(fileReader1, l1, symbol1);
  readNextLine(fileReader2, l2, symbol2);

  uint32_t nl1 = readNextLine(fileReader1, l1, symbol1);
  uint32_t nl2 = readNextLine(fileReader2, l2, symbol2);
  std::string outFile = generateRandomString(20) + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())) + ".csv";
  auto fileWriter = fileWriterProvider(outFile);

  const char* str = "Symbol, Timestamp, Price, Size, Exchange, Type\n";
  fileWriter(str, strlen(str));

  // Just like the "merge" step of merge sort, keep picking the
  // "lesser" fron element until EOF is obtained in one of the files
  while (nl1 && nl2)
  {
    MDEntrry md1(reinterpret_cast<const char*>(l1));
    MDEntrry md2(reinterpret_cast<const char*>(l2));

    if (md1 <=> md2 < 0)
    {
      fileWriter(l1, nl1);
      nl1 = readNextLine(fileReader1, l1, symbol1);
    }
    else
    {
      fileWriter(l2, nl2);
      nl2 = readNextLine(fileReader2, l2, symbol2);
    }
  }

  while (nl1)
  {
    fileWriter(l1, nl1);
    nl1 = readNextLine(fileReader1, l1, symbol1);
  }

  while (nl2)
  {
    fileWriter(l2, nl2);
    nl2 = readNextLine(fileReader2, l2, symbol2);
  }

  // Notify the merge just happenned
  outFileNotifier(fn1, fn2, outFile);
  return true;
}

/**
 * This function is the entry point for all the worker threads
 * This is the core logic for merging  2 files
 * It will pop files from 
 *
 * @param  inputFiles               A queue containing the names of files to be merged
 * @param  mutex                    A for synschronized access tot "inpuFiles"
 * @param  FileReaderProvider       A "FileReaderProvider"(explained in Solution.h)
 * @param  FileWriterProvider       A "FileWriterProvider"(explained in Solution.h)
 * @param  maxHeapSize              Max memory the thread in which it runs can allocate
 *                                  to hold intermediate resuts, before they ae written to an outfile
*/
void mergeAllFiles(std::shared_ptr<std::queue<std::string>> inputFiles,
                   std::shared_ptr<std::mutex> mutex,
                   FileReaderProvider fileReaderProvider,
                   FileWriterProvider fileWriterProvider,
                   const uint64_t maxHeapSize)
{
  const std::function<std::optional<std::tuple<std::string, std::string>>()> mergeFileFetcher =
  [inputFiles, mutex]()
  {
    std::optional<MergeFilePair> res = std::nullopt;
    {
      std::unique_lock<std::mutex> lock(*mutex);
      if(inputFiles->size() >= 2)
      {
        // Read-write the input-file queue in a critical section
        std::string f1 = inputFiles->front();
        inputFiles->pop();
        std::string f2 = inputFiles->front();
        inputFiles->pop();

        res = std::make_tuple(f1, f2);
      }
    }

    return res;
  };

  MergeNotificationHandler mergeNotificationHandler =
  [inputFiles, mutex](const std::string& f1, const std::string& f2, const std::string& outFile)
  {
    //.csv files are intermediate files, delete them after merge
    if(f1.substr(f1.find_first_of(".")).compare(".csv") == 0)
    {
      std::remove(f1.c_str());
    }

    //.csv files are intermediate files, delete them after merge
    if(f2.substr(f2.find_first_of(".")).compare(".csv") == 0)
    {
      std::remove(f2.c_str());
    }

    // Read-write the input-file queue in a critical section
    std::unique_lock<std::mutex> lock(*mutex);
    inputFiles->push(outFile);
  };

  while(mergeFiles(mergeFileFetcher, fileReaderProvider, fileWriterProvider, mergeNotificationHandler, maxHeapSize));
}

void entryPoint(uint8_t numThreads,
    uint64_t maxHeapSize,
    std::shared_ptr<std::queue<std::string>> remainingFiles,
    std::shared_ptr<std::mutex> mutex,
    FileReaderProvider frp,
    FileWriterProvider fwp)
{
  std::thread* threads = numThreads > 1 ? new std::thread[numThreads-1] : nullptr;

  // 1 thread is this thread so, numThreads - 1
  for (uint8_t i = 0; i < numThreads - 1; i++)
  {
    threads[i] = std::thread([remainingFiles, mutex, frp, fwp, maxHeapSize, numThreads](){ mergeAllFiles(remainingFiles, mutex, frp, fwp, maxHeapSize/numThreads);});
  }

  mergeAllFiles(remainingFiles, mutex, frp, fwp, maxHeapSize/numThreads);

  for (uint8_t i = 0; i < numThreads - 1; i++)
  {
    threads[i].join();
  }

  if (threads)
  {
    delete[] threads;
  }

  // Rename the last file to MultiplexedFile.txt as this is the final file
  std::rename(remainingFiles->front().c_str(), "MultiplexedFile.txt");
  remainingFiles->front() = "MultiplexedFile.txt";
}

