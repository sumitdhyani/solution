#include <gtest/gtest.h>
#include <string.h>
#include <iostream>
#include <Solution.h>

// In all the test cases, outBuffer is the
// buffer that cotains the contents of the out file
class NegativeCaseTests : public ::testing::Test {
    virtual void SetUp() override        
    {}

    virtual void TearDown() override
    {}
};

TEST_F(NegativeCaseTests, MergingEmptyFiles)
{
  const char* header = "Symbol, Timestamp, Price, Size, Exchange, Type";

  FileReaderProvider fileReaderProvider =
  [](const std::string&, const uint32_t)
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

  ASSERT_EQ(totalLen, expected.length());
  ASSERT_EQ(0, memcmp(outBuffer, expected.c_str(), expected.length()));
  EXPECT_EQ(inputFiles->size(), 1);
  EXPECT_EQ(inputFiles->front().compare("MultiplexedFile.txt"), 0);
}

TEST_F(NegativeCaseTests, UnequalSizedFiles)
{
  const char* header = "Symbol, Timestamp, Price, Size, Exchange, Type\n";
  const char* mockEntry = "2021-03-05 10:00:00.123, 228.5, 120, NYSE, Ask\n";
  bool firstLine = true;
  FileReaderProvider fileReaderProvider =
  [mockEntry, &firstLine](const std::string& file, const uint32_t)
  {
    FileLineReader flr;
    if (0 == file.compare("CSCO.txt"))
    {
      flr = [](char* buff)
      {
        return 0;
      };
    }
    else
    {
      flr = [mockEntry, &firstLine](char* buff)
      {
        uint8_t ret = 0;
        if (firstLine)
        {
          memcpy(buff, mockEntry, strlen(mockEntry));
          ret = strlen(mockEntry);
          firstLine = false;
        }

        return ret;
      };
    }

    return flr;
  };

  uint32_t totalLen = 0;
  char outBuffer[1024];
  FileWriterProvider fileWriterProvider =
  [header, &outBuffer, &totalLen](const std::string&)
  {
    return [&outBuffer, &totalLen, header](const char* buff, uint32_t len)
    {
      memcpy(outBuffer + totalLen, buff, len);
      totalLen += len; 
    };
  };

  uint8_t numThreads = 1;
  auto inputFiles = std::make_shared<std::queue<std::string>>();
  inputFiles->push("CSCO.txt");
  inputFiles->push("MSFT.txt");
  entryPoint(numThreads, 
             1024,
             inputFiles,
             std::make_shared<std::mutex>(),
             fileReaderProvider,
             fileWriterProvider);

  std::string expected = 
  std::string(header) +  "MSFT, " + mockEntry;

  std::cout << "Contents of outfile:" << std::endl;
  std::cout << std::string(outBuffer, totalLen);

  assert(totalLen == expected.length());
  assert(0 == memcmp(outBuffer, expected.c_str(), expected.length()));
  EXPECT_EQ(inputFiles->size(), 1);
  EXPECT_EQ(inputFiles->front().compare("MultiplexedFile.txt"), 0);
}