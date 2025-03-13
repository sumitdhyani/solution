#include <iostream>
#include <chrono>
#include <string>
#include "SmartBuffer.hpp"
#include "fstream"

// int main(int argc, char** argv)
// {
//   std::ifstream fs(argv[1], std::ifstream::in);
//   uint64_t numLines = 0;
//   SyncIOReadBuffer buff(4096);

//   char currLine[1014];
//   auto start = std::chrono::high_resolution_clock().now();
//   while(buff.readUntil((char*)currLine,
//                        [&fs](char* out, const uint32_t len)
//                        {
//                         fs.read(out, len);
//                         return fs.gcount();
//                        },
//                        '\n')
//   )
//   {
//     ++numLines;
//   }
//   auto duration = std::chrono::high_resolution_clock().now() - start;
//   char* currWriteHead = currLine;

//   std::string durationStr = std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()) + " ns"; 
//   currWriteHead += sprintf(currWriteHead, "\n\nDuration: %s", durationStr.c_str());

//   std::cout << numLines << std::endl;
//   std::cout << currLine << std::endl;
//   return 0;
// }

int main(int argc, char** argv)
{
  std::ifstream fs(argv[1], std::ifstream::in);
  uint64_t numLines = 0;
  char currLine[1014];
  auto start = std::chrono::high_resolution_clock().now();
  while(fs.getline(currLine, 1024))
  {
    ++numLines;
  }
  auto duration = std::chrono::high_resolution_clock().now() - start;
  char* currWriteHead = currLine;
  std::string durationStr = std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()) + " ns"; 
  currWriteHead += sprintf(currWriteHead, "\n\nDuration: %s", durationStr.c_str());
  std::cout << numLines << std::endl;
  std::cout << currLine << std::endl;
  return 0;
}