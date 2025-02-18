#include <cassert>
#include <string.h>
#include <iostream>
#include <Solution.h>

int main()
{
  const char* header = "Symbol, Timestamp, Price, Size, Exchange, Type";

  FileReaderProvider fileReaderProvider =
  [](const std::string&)
  {
    return [](char* buff)
    {
      return 0;
    };
  };

  uint32_t totalLen;
  char outBuffer[256];
  FileWriterProvider fileWriterProvider =
  [header, &outBuffer, &totalLen](const std::string&)
  {
    return [&outBuffer, &totalLen](const char* buff, uint32_t len)
    {
      memcpy(outBuffer, buff, len);
      totalLen += len;
    };
  };

  uint8_t numThreads = 2;
  auto inputFiles = std::make_shared<std::queue<std::string>>();
  inputFiles->push("file1.txt");
  inputFiles->push("file2.txt");
  entryPoint(numThreads, 
             1024,
             inputFiles,
             std::make_shared<std::mutex>(),
             mergeFiles,
             fileReaderProvider,
             fileWriterProvider);
  assert(0 == memcmp(outBuffer, header, strlen(header)));
  assert(totalLen == strlen(header) + 1);
  assert(outBuffer[totalLen-1] == '\n');
}