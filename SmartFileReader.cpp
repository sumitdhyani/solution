#include <fstream>
#include <chrono>
#include <iostream>
#include <chrono>

#include "SmartBuffer.hpp"
#include "Solution.h"

int main(int argc, char** argv)
{  
  char line[256];
  auto const io_file =
  [fileHandle = std::make_shared<std::ifstream>(argv[1], std::ifstream::in)]
  (char* out, const uint32_t len) {
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
  };

  auto io_console = 
  [](char* out, const uint32_t len)
  {
    std::ios_base:: sync_with_stdio(false);
    std::cin.tie(NULL); std::cout.tie(NULL);
    std::cin.read(out, len);
    return std::cin.gcount();
  };

  SyncIOReadBuffer smartBuffer(atol(argv[2]));

  uint32_t numLines = 0;
  auto start = std::chrono::high_resolution_clock().now();
  while(smartBuffer.readUntil(line, io_console, '\n'))
  {
    ++numLines;
  }
  auto duration = std::chrono::high_resolution_clock().now() - start;

  std::cout << "No. of lines read = " << numLines << std::endl;
  std::cout << "REserved space = " << atol(argv[2]) << std::endl;
  std::cout << "Duration: " << std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() << " ns";
  return 0;
}