#include <memory>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <tuple>
#include <queue>
#include <stdlib.h>
#include <string.h>
#include <filesystem>
#include "Solution.h"

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
  
  uint8_t offSet = 0;
  if (symbol.length() != 0)
  {
    memcpy(buff, symbol.c_str(), symbol.length());
    memcpy(buff + symbol.length(), ", ", 2);
    offSet = symbol.length() + 2;
    buff += offSet;
  }

  uint32_t bytesReadFromFile = reader(buff);
  return bytesReadFromFile? bytesReadFromFile + offSet : 0;
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

class MDEntrry
{
  struct MDTimeStamp
  {
    
    public:
    // "raw" should be in the format:
    // Symbol, Timestamp, Price, Size, Exchange, Type

    MDTimeStamp(const char* timestamp)
    {
      m_yyyy = timestamp;
      m_mm   = m_yyyy + 5;
      m_dd   = m_mm + 3;
      m_hh   = m_dd + 3;
      m_min  = m_hh + 3;
      m_ss   = m_min + 3;
      m_ms   = m_ss + 3;
    }

    int compareYear(const char* y1, const char* y2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 4 && !comp)
      {
        if (y1[i] < y2[i])
        {
          comp = -1;
        }
        else if (y1[i] > y2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int compareMonth(const char* m1, const char* m2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 2 && !comp)
      {
        if (m1[i] < m2[i])
        {
          comp = -1;
        }
        else if (m1[i] > m2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int compareday(const char* d1, const char* d2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 2 && !comp)
      {
        if (d1[i] < d2[i])
        {
          comp = -1;
        }
        else if (d1[i] > d2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int compareHour(const char* h1, const char* h2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 2 && !comp)
      {
        if (h1[i] < h2[i])
        {
          comp = -1;
        }
        else if (h1[i] > h2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int compareMinute(const char* min1, const char* min2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 2 && !comp)
      {
        if (min1[i] < min2[i])
        {
          comp = -1;
        }
        else if (min1[i] > min2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int compareSec(const char* s1, const char* s2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 2 && !comp)
      {
        if (s1[i] < s2[i])
        {
          comp = -1;
        }
        else if (s1[i] > s2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int compareMs(const char* ms1, const char* ms2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 3 && !comp)
      {
        if (ms1[i] < ms2[i])
        {
          comp = -1;
        }
        else if (ms1[i] > ms2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int operator <=>(const MDTimeStamp& other) const
    {
      if (int comp = compareYear(m_yyyy, other.m_yyyy); comp != 0)
      {
        return comp;
      }
      else if(comp = compareMonth(m_mm, other.m_mm); comp != 0)
      {
        return comp;
      }
      else if(comp = compareday(m_dd, other.m_dd); comp != 0)
      {
        return comp;
      }
      else if(comp = compareHour(m_hh, other.m_hh); comp != 0)
      {
        return comp;
      }
      else if(comp = compareMinute(m_min, other.m_min); comp != 0)
      {
        return comp;
      }
      else if(comp = compareSec(m_ss, other.m_ss); comp != 0)
      {
        return comp;
      }
      else
      {
        return compareMs(m_ms, other.m_ms);
      }
      
    }
    
    const char* m_yyyy;
    const char* m_mm;
    const char* m_dd;
    const char* m_hh;
    const char* m_min;
    const char* m_ss;
    const char* m_ms;
  };

  static const char* findTimeStampStart(const char* raw)
  {
    uint8_t i = 0;
    while (raw[i] != ',')
    {
      ++i;
    }

    return raw+i+2;
  }

  public:

  MDEntrry(const char* raw) : m_raw(raw), m_timestamp(findTimeStampStart(raw))
  {}

  int operator<=>(const MDEntrry& other) const
  {
    if (int comp = (m_timestamp <=> other.m_timestamp); 0 != comp)
    {
      return comp;
    }
    else
    {
      uint8_t i = 0;
      while (0 == comp &&
            m_raw[i] != ',' &&
            other.m_raw[i] != ',')
      {
        if (m_raw[i] < other.m_raw[i])
        {
          comp = -1;
        }
        else if (m_raw[i] > other.m_raw[i])
        {
          comp = 1;
        }

        ++i;
      }

      if (0 == comp)
      {
        if (m_raw[i] == ',')
        {
          comp = other.m_raw[i] == ','? 0 : -1;
        }
        else
        {
          comp = m_raw[i] == ','? 0 : 1;
        }
      }

      return comp;
    }
  }

  private:
  MDTimeStamp m_timestamp;
  const char* m_raw;
};

/**
 * This function is the entry point for all the worke2 threads
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
                   FileMerger fileMerger,
                   FileReaderProvider fileReaderProvider,
                   FileWriterProvider fileWriterProvider,
                   const uint64_t maxHeapSize)
{
  const std::function<std::optional<std::tuple<std::string, std::string>>()> mergeFileFetcher =
  [inputFiles, mutex]()
  {
    std::optional<MergeFilePair> res = std::nullopt;
    if(inputFiles->size() >= 2)
    {
    // Read-write the input-file queue in a critical section
      std::unique_lock<std::mutex> lock(*mutex);
      std::string f1 = inputFiles->front();
      inputFiles->pop();
      std::string f2 = inputFiles->front();
      inputFiles->pop();

      res = std::make_tuple(f1, f2);
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

  std::function<uint32_t()> fetchNumRemainingFiles =
  [inputFiles, mutex]()
  {
    // Read-write the input queue in a critical section
    std::unique_lock<std::mutex> lock(*mutex);
    return inputFiles->size();
  };

  while(fetchNumRemainingFiles() >= 2)
  {
    fileMerger(mergeFileFetcher, fileReaderProvider, fileWriterProvider, mergeNotificationHandler, maxHeapSize);
  }
}

void entryPoint(uint8_t numThreads,
    uint64_t maxHeapSize,
    std::shared_ptr<std::queue<std::string>> remainingFiles,
    std::shared_ptr<std::mutex> mutex,
    FileMerger fm,
    FileReaderProvider frp,
    FileWriterProvider fwp)
{
  std::thread* threads[8];

  for (uint8_t i = 0; i < numThreads; i++)
  {
    threads[i] = new std::thread([remainingFiles, mutex, fm, frp, fwp, maxHeapSize, numThreads](){ mergeAllFiles(remainingFiles, mutex, fm, frp, fwp, maxHeapSize/numThreads);});
  }

  for (uint8_t i = 0; i < numThreads; i++)
  {
    threads[i]->join();
  }

  for (uint8_t i = 0; i < numThreads; i++)
  {
    delete threads[i];
  }

  std::rename(remainingFiles->front().c_str(), "MultiplexedFile.txt");
}

FileLineReader getFileLineReader(const std::string& filename)
{
  auto fileHandle = std::make_shared<std::ifstream>(filename, std::ifstream::in);
  FileLineReader flr =
  [fileHandle]
  (char *buff)
  {
    uint8_t ret = 0;
    if(fileHandle->getline(buff, 256); buff[0] != '\0')
    {
      if (memcmp(buff, "Symbol", strlen("Symbol")) == 0 ||
          memcmp(buff, "Timestamp", strlen("Timestamp")) == 0)
      {
        fileHandle->getline(buff, 256);
      }

      uint8_t len = strlen(buff);
      buff[len] = '\n';
      ret = len + 1;
    }

    return ret;
  };

  return flr;
}

FileWriter getFileWriter(const std::string& filename)
{
  auto fileHandle = std::make_shared<std::ofstream>(filename, std::ofstream::out);
  FileWriter fw =
  [fileHandle]
  (const char* buff, const uint32_t len)
  {
    fileHandle->write(buff, len);
  };

  return fw;
}

bool mergeFiles(const std::function<std::optional<MergeFilePair>()> filenameFetcher, 
                const FileReaderProvider fileReaderProvider,
                const FileWriterProvider fileWriterProvider,
                const MergeNotificationHandler outFileNotifier,
                const uint64_t maxHeapSize)
{
  char* buff = reinterpret_cast<char*>(malloc(maxHeapSize));
  char* curr = buff;
  uint64_t bytesRemaining  = maxHeapSize;
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

  bool f1IsIntermediateFile = false;
  bool f2IsIntermediateFile = false;


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

  auto fileReader1 = fileReaderProvider(fn1);
  auto fileReader2 = fileReaderProvider(fn2);

  //Lines read from each of the files
  char l1[256];
  char l2[256];

  // Bytes in the curr line in each file
  uint32_t nl1 = readNextLine(fileReader1, l1, symbol1);
  uint32_t nl2 = readNextLine(fileReader2, l2, symbol2);
  std::string outFile = generateRandomString(20) + ".csv";
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
      if(bytesRemaining < nl1)
      {
        fileWriter(buff, curr - buff);
        curr = buff;
        bytesRemaining = maxHeapSize;
      }

      memcpy(curr, l1, nl1);
      curr += nl1;
      bytesRemaining -= nl1;
      nl1 = readNextLine(fileReader1, l1, symbol1);
    }
    else
    {
      if(bytesRemaining < nl2)
      {
        fileWriter(buff, curr - buff);
        curr = buff;
        bytesRemaining = maxHeapSize;
      }

      memcpy(curr, l2, nl2);
      curr += nl2;
      bytesRemaining -= nl2;
      nl2 = readNextLine(fileReader2, l2, symbol2);
    }
  }

  if (curr > buff)
  {
    fileWriter(buff, curr - buff);
  }

  curr = buff;
  bytesRemaining = maxHeapSize;

  if (!nl1)
  {
    while (nl2 != 0)
    {
      if(bytesRemaining < nl2)
      {
        fileWriter(buff, curr - buff);
        curr = buff;
        bytesRemaining = maxHeapSize;
      }

      memcpy(curr, l2, nl2);
      curr += nl2;
      bytesRemaining -= nl2;
      nl2 = readNextLine(fileReader2, l2, symbol2);
    }
  }

  if (!nl2)
  {
    while (nl1 != 0)
    {
      if(bytesRemaining < nl1)
      {
        fileWriter(buff, curr - buff);
        curr = buff;
        bytesRemaining = maxHeapSize;
      }

      memcpy(curr, l1, nl1);
      curr += nl1;
      bytesRemaining -= nl1;
      nl1 = readNextLine(fileReader1, l1, symbol1);
    }
  }

  if (curr > buff)
  {
    fileWriter(buff, curr - buff);
  }

  free(buff);
  outFileNotifier(fn1, fn2, outFile);
  return true;
}

int main(int argc, char** argv)
{
  if (argc < 5)
  {
    std::cout << "Invalid no. of args\nGive the command in the format \"<path to executable> <numThreads> <max open files> <maxAllowed memory to use> <space separated list of files>\"" << std::endl;
    return 1;
  }

  uint32_t numThreads = std::min((decltype(std::thread::hardware_concurrency()))atoi(argv[1]), std::thread::hardware_concurrency());
  uint32_t maxOpenFiles = atoll(argv[2]);
  
  if(maxOpenFiles < numThreads*2)
  {
    numThreads = maxOpenFiles/2;
  }

  uint64_t maxAllowedMemory = atoll(argv[3]) * 1024 * 1024 * 1024;

  uint32_t numCores = std::thread::hardware_concurrency();

  auto mutex = std::make_shared<std::mutex>();
  auto inputFiles = std::make_shared<std::queue<std::string>>();
  for (uint32_t i = 4; i < argc; i++)
  {
    inputFiles->push(argv[i]);
  }

  entryPoint(numThreads, maxAllowedMemory, inputFiles, mutex, mergeFiles, getFileLineReader, getFileWriter);
  
  return 0;
}