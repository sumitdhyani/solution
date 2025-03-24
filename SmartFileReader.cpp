#include <fstream>
#include <chrono>
#include <iostream>
#include <chrono>
#include <string>
#include "SmartBuffer.hpp"

int main(int argc, char** argv)
{  
  char line[256];
  // auto const io_file =
  // [fileHandle = std::make_shared<std::ifstream>(argv[1], std::ifstream::in)]
  // (char* out, const uint32_t len) {
  //   uint32_t ret = 0;
  //   if (fileHandle->is_open() && len)
  //   {
  //     fileHandle->read(out, len);
  //     ret = fileHandle->gcount();
  //     if(!ret)
  //     {
  //       fileHandle->close();
  //     }
  //   }

  //   return ret;
  // };

  auto io_console_reader = 
  [](char* out, const uint32_t len)
  {
    std::cin.read(out, len);
    return std::cin.gcount();
  };

  auto io_console_writer = 
  [](char* out, const uint32_t len)
  {
    std::cout.write(out, len);
  };

  SyncIOReadBuffer<uint32_t> smartReadBuffer(atol(argv[1]));
  SyncIOLazyWriteBuffer<uint32_t> smartWriteBuffer(atol(argv[1]), io_console_writer);

  // SyncIOStackReadBuffer<4096> smartReadBuffer;
  // SyncIOLazyStackWriteBuffer<4096> smartWriteBuffer(io_console_writer);

  uint32_t numLines = 0;
  uint32_t lineLen = 0;
  auto start = std::chrono::high_resolution_clock().now();
  while(lineLen = smartReadBuffer.readUntil(line, io_console_reader, [](const char& ch) { return ch == '\n'; }))
  {
    smartWriteBuffer.write((char*)line, lineLen);
    ++numLines;
  }
  auto duration = std::chrono::high_resolution_clock().now() - start;

  char endingBuff[1024];
  char* currWriteHead = endingBuff;
  currWriteHead += sprintf(currWriteHead, "\n\nNo. of lines read = %u\n", numLines);
  currWriteHead += sprintf(currWriteHead, "Reserved space = %s\n", argv[1]);
  std::string durationStr = std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()) + " ns"; 
  currWriteHead += sprintf(currWriteHead, "Duration: %s", durationStr.c_str());
  smartWriteBuffer.write(endingBuff, currWriteHead - endingBuff + 1);
  return 0;
}