#include <fstream>
#include <chrono>
#include <iostream>
int main(int argc, char** argv)
{
  std::ifstream fileHandle(argv[1]);
  char line[256];
  int numLines = 0;


  auto start = std::chrono::high_resolution_clock().now();
  while(fileHandle.getline(line, sizeof(line)))
  {
    ++numLines;
  }
  auto duration = std::chrono::high_resolution_clock().now() - start;

  std::cout << "No. of lines read = " << numLines << std::endl;
  std::cout << "Duration: " << std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() << " ns";
  return 0;
}