#include <cassert>
#include <string.h>
#include <iostream>
#include <Solution.h>

int main()
{
  const char* header = "Symbol, Timestamp, Price, Size, Exchange, Type";
  const char* mockEntry = "2021-03-05 10:00:00.123, 228.5, 120, NYSE, Ask";
  bool firstLine = true;
  FileReaderProvider fileReaderProvider =
  [mockEntry, &firstLine](const std::string& file)
  {
    FileLineReader flr;
    if (0 == file.compare("CSCO.txt"))
    {
      flr = [](char* buff)
      {
        return 0;
      };
    }
    else
    {
      flr = [mockEntry, &firstLine](char* buff)
      {
        uint8_t ret = 0;
        if (firstLine)
        {
          memcpy(buff, mockEntry, strlen(mockEntry));
          buff[strlen(mockEntry)] = '\n';
          ret = strlen(mockEntry) + 1;
          firstLine = false;
        }

        return ret;
      };
    }

    return flr;
  };

  uint32_t totalLen = 0;
  char outBuffer[1024];
  FileWriterProvider fileWriterProvider =
  [header, &outBuffer, &totalLen](const std::string&)
  {
    return [&outBuffer, &totalLen, header](const char* buff, uint32_t len)
    {
      memcpy(outBuffer + totalLen, buff, len);
      totalLen += len; 
    };
  };

  uint8_t numThreads = 1;
  auto inputFiles = std::make_shared<std::queue<std::string>>();
  inputFiles->push("CSCO.txt");
  inputFiles->push("MSFT.txt");
  entryPoint(numThreads, 
             1024,
             inputFiles,
             std::make_shared<std::mutex>(),
             mergeFiles,
             fileReaderProvider,
             fileWriterProvider);

  std::string expected = 
  std::string(header) + "\n"+
  "MSFT, " + mockEntry + "\n";

  std::cout << "Contents of outfile:" << std::endl;
  std::cout << std::string(outBuffer, totalLen);
  
  assert(totalLen == expected.length());
  assert(0 == memcmp(outBuffer, expected.c_str(), expected.length()));
}