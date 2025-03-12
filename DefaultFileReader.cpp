#include <fstream>
#include <chrono>
#include <iostream>
int main(int argc, char** argv)
{
  char line[256];
  int numLines = 0;
  std::ios_base:: sync_with_stdio(false);
  std::cin.tie(NULL); std::cout.tie(NULL);

  auto start = std::chrono::high_resolution_clock().now();
  while(std::cin.getline(line, sizeof(line)))
  {
    ++numLines;
    std::cout << line << "\n";
  }

  std::cout << std::endl;

  auto duration = std::chrono::high_resolution_clock().now() - start;

  std::cout << "\n\nNo. of lines read = " << numLines << std::endl;
  std::cout << "Duration: " << std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() << " ns";
  return 0;
}