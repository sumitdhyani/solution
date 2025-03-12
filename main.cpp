#include <iostream>
#include <fstream>
#include <tuple>
#include <string.h>
#include <filesystem>
#include "Solution.h"
#include "SmartBuffer.hpp"


// Assumption: maxBuffSize > max length of a line
FileLineReader getFileLineReader(const std::string& filename, const uint32_t maxBuffSize)
{
  auto fileHandle = std::make_shared<std::ifstream>(filename, std::ifstream::in);
  auto smartBuffer = std::make_shared<SyncIOReadBuffer>(maxBuffSize);  
  
  FileLineReader flr = 
  [fileHandle, smartBuffer](char* out)
  {
    return
    smartBuffer->readUntil(out, 
                           [fileHandle](char* out, const uint32_t len) {
                              uint32_t ret = 0;
                              if (fileHandle->is_open() && len)
                              {
                                fileHandle->read(out, len);
                                ret = fileHandle->gcount();
                                if(!ret)
                                {
                                  fileHandle->close();
                                }
                              }

                              return ret;
                           },
                           '\n'
    );
  };

  return flr;
}

FileWriter getFileWriter(const std::string& filename)
{
  auto fileHandle = std::make_shared<std::ofstream>(filename, std::ofstream::out);
  FileWriter fw =
  [fileHandle, filename] 
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