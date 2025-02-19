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

  uint32_t totalLen = 0;
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
             fileReaderProvider,
             fileWriterProvider);
  std::string expected = std::string(header) + "\n";

  std::cout << "Contents of outfile:" << std::endl;
  std::cout << std::string(outBuffer, totalLen);

  assert(totalLen == expected.length());
  assert(0 == memcmp(outBuffer, expected.c_str(), expected.length()));
}