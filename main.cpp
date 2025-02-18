#include <iostream>
#include <fstream>
#include <string.h>
#include <filesystem>
#include "Solution.h"

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