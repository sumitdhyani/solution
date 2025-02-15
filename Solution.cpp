#include <functional>
#include <memory>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <string>

typedef std::function<bool(std::string&)> FileLineReader;
typedef std::function<void(const std::string&)> FileLineWriter;
typedef std::function<FileLineReader(const std::string&)> FileReaderProvider;
typedef std::function<FileLineWriter(const std::string&)> FileWriterProvider;

typedef std::function<void(const std::string& file1, 
                           const std::string& file2,
                           const std::string& outFile,
                           const FileReaderProvider&,
                           const FileWriterProvider&,
                           const std::string&)> FileMerger;

                           //typedef std::function<>
int main(int argc, char** argv)
{
  if (argc < 3)
  {
    std::cout << "INvalid no. of args" << std::endl;
    return 1;
  }

  FileReaderProvider frp =
  [](const std::string& filename)
  {
    auto fileHandle = std::make_shared<std::ifstream>(filename, std::ifstream::in);
    FileLineReader flr =
    [fileHandle]
    (std::string& buff)
    {
      static bool headerRead = false;
      if (!headerRead)
      {
        std::getline(*fileHandle, buff);
        headerRead = true;
      }

      return (bool)std::getline(*fileHandle, buff);
    };

    return flr;
  };

  FileWriterProvider fwp =
  [](const std::string& filename)
  {
    auto fileHandle = std::make_shared<std::ofstream>(filename, std::ofstream::out);
    FileLineWriter flw =
    [fileHandle]
    (const std::string& buff)
    {
      static bool headerWritten = false;
      if (!headerWritten)
      {
        (*fileHandle) << "Timestamp, Price, Size, Exchange, Type" << std::endl;
        headerWritten = true;
      }

      (*fileHandle) << buff << std::endl;
    };

    return flw;
  };

  auto reader = frp(argv[1]);
  auto writer = fwp(argv[2]);

  std::string buff;
  buff.reserve(256);
  while (reader(buff))
  {
    writer(buff);
  }

  return 0;
}