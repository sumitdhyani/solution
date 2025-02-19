#include <iostream>
#include <fstream>
#include <string.h>
#include <filesystem>
#include "Solution.h"

FileLineReader getFileLineReader(const std::string& filename)
{
  auto fileHandle = std::make_shared<std::ifstream>(filename, std::ifstream::in);
  FileLineReader flr =
  [fileHandle]
  (char *buff)
  {
    uint8_t ret = 0;
    if (nullptr == buff)
    {
      fileHandle->close();
    }
    else if(fileHandle->is_open() &&
            fileHandle->getline(buff, 256); buff[0] != '\0')
    {
      if (memcmp(buff, "Symbol", strlen("Symbol")) == 0 ||
          memcmp(buff, "Timestamp", strlen("Timestamp")) == 0)
      {
        fileHandle->getline(buff, 256);
      }

      uint8_t len = strlen(buff);
      if (len > 0)
      {
        buff[len] = '\n';
        ret = len + 1;
      }
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

  entryPoint(numThreads, maxAllowedMemory, inputFiles, mutex, getFileLineReader, getFileWriter);
  
  return 0;
}