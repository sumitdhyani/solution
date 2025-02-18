#include <cassert>
#include <string.h>
#include <iostream>
#include <Solution.h>

int main()
{
  const char* header = "Symbol, Timestamp, Price, Size, Exchange, Type";
  const char* mockEntry_CSCO = "2021-03-05 10:00:00.123, 228.5, 120, NYSE, Ask";
  const char* mockEntry_MSFT = mockEntry_CSCO;
  bool firstLine_CSCO = true;
  bool firstLine_MSFT = true;
  FileReaderProvider fileReaderProvider =
  [mockEntry_CSCO, &firstLine_CSCO, mockEntry_MSFT, &firstLine_MSFT]
  (const std::string& file)
  {
    FileLineReader flr;
    if (0 == file.compare("CSCO.txt"))
    {
      flr = [mockEntry_CSCO, &firstLine_CSCO](char* buff)
      {
        uint8_t ret = 0;
        if (firstLine_CSCO)
        {
          memcpy(buff, mockEntry_CSCO, strlen(mockEntry_CSCO));
          buff[strlen(mockEntry_CSCO)] = '\n';
          ret = strlen(mockEntry_CSCO) + 1;
          firstLine_CSCO = false;
        }

        return ret;
      };
    }
    else
    {
      flr = [mockEntry_MSFT, &firstLine_MSFT](char* buff)
      {
        uint8_t ret = 0;
        if (firstLine_MSFT)
        {
          memcpy(buff, mockEntry_MSFT, strlen(mockEntry_MSFT));
          buff[strlen(mockEntry_MSFT)] = '\n';
          ret = strlen(mockEntry_MSFT) + 1;
          firstLine_MSFT = false;
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
  inputFiles->push("MSFT.txt");
  inputFiles->push("CSCO.txt");
  entryPoint(numThreads, 
             1024,
             inputFiles,
             std::make_shared<std::mutex>(),
             mergeFiles,
             fileReaderProvider,
             fileWriterProvider);

  std::string str = 
  std::string(header) + "\n"+
  "CSCO, " + mockEntry_CSCO + "\n" +
  "MSFT, " + mockEntry_MSFT + "\n";

  std::cout << "Contents of outfile:" << std::endl;
  std::cout << std::string(outBuffer, totalLen);
  
  assert(totalLen == str.length());
  assert(0 == memcmp(outBuffer, str.c_str(), str.length()));
}