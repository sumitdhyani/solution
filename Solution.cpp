#include <functional>
#include <memory>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <tuple>
#include <string>
#include <queue>
#include <optional>
#include <stdlib.h>
#include <string.h>
#include <filesystem>

typedef std::function<uint8_t(char* buff)> FileLineReader;
typedef std::function<void(const char*, const uint32_t)> FileWriter;
typedef std::function<FileLineReader(const std::string&)> FileReaderProvider;
typedef std::function<FileWriter(const std::string&)> FileWriterProvider;

typedef std::function<bool(const std::function<std::optional<std::tuple<std::string,std::string>>()>, 
                           const FileReaderProvider&,
                           const FileWriterProvider&,
                           const std::function<void(const std::string&, const std::string&, const std::string&)>,
                           const uint64_t)> FileMerger;

uint32_t readNextLine(const FileLineReader& reader,
                      char* buff,
                      const std::string& symbol)
{
  
  uint8_t offSet = 0;
  if (symbol.length() != 0)
  {
    memcpy(buff, symbol.c_str(), symbol.length());
    memcpy(buff + symbol.length(), ", ", 2);
    offSet = symbol.length() + 2;
    buff += offSet;
  }

  uint32_t bytesReadFromFile = reader(buff);
  return bytesReadFromFile? bytesReadFromFile + offSet : 0;
}

std::string generateRandomString(const int len) {
  static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
  std::string res;
  res.reserve(len);

  for (int i = 0; i < len; ++i) {
      res += alphanum[rand() % (sizeof(alphanum) - 1)];
  }
  
  return res;
}

class MDEntrry
{
  struct MDTimeStamp
  {
    
    public:
    // "raw" should be in the format:
    // Symbol, Timestamp, Price, Size, Exchange, Type

    MDTimeStamp(const char* timestamp)
    {
      m_yyyy = timestamp;
      m_mm   = m_yyyy + 5;
      m_dd   = m_mm + 3;
      m_hh   = m_dd + 3;
      m_min  = m_hh + 3;
      m_ss   = m_min + 3;
      m_ms   = m_ss + 3;
    }

    int compareYear(const char* y1, const char* y2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 4 && !comp)
      {
        if (y1[i] < y2[i])
        {
          comp = -1;
        }
        else if (y1[i] > y2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int compareMonth(const char* m1, const char* m2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 2 && !comp)
      {
        if (m1[i] < m2[i])
        {
          comp = -1;
        }
        else if (m1[i] > m2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int compareday(const char* d1, const char* d2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 2 && !comp)
      {
        if (d1[i] < d2[i])
        {
          comp = -1;
        }
        else if (d1[i] > d2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int compareHour(const char* h1, const char* h2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 2 && !comp)
      {
        if (h1[i] < h2[i])
        {
          comp = -1;
        }
        else if (h1[i] > h2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int compareMinute(const char* min1, const char* min2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 2 && !comp)
      {
        if (min1[i] < min2[i])
        {
          comp = -1;
        }
        else if (min1[i] > min2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int compareSec(const char* s1, const char* s2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 2 && !comp)
      {
        if (s1[i] < s2[i])
        {
          comp = -1;
        }
        else if (s1[i] > s2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int compareMs(const char* ms1, const char* ms2) const
    {
      int comp = 0;
      uint8_t i = 0;
      while (i < 3 && !comp)
      {
        if (ms1[i] < ms2[i])
        {
          comp = -1;
        }
        else if (ms1[i] > ms2[i])
        {
          comp = 1;
        }
        else
        {
          ++i;
        }
      }

      return comp;
    }

    int operator <=>(const MDTimeStamp& other) const
    {
      if (int comp = compareYear(m_yyyy, other.m_yyyy); comp != 0)
      {
        return comp;
      }
      else if(comp = compareMonth(m_mm, other.m_mm); comp != 0)
      {
        return comp;
      }
      else if(comp = compareday(m_dd, other.m_dd); comp != 0)
      {
        return comp;
      }
      else if(comp = compareHour(m_hh, other.m_hh); comp != 0)
      {
        return comp;
      }
      else if(comp = compareMinute(m_min, other.m_min); comp != 0)
      {
        return comp;
      }
      else if(comp = compareSec(m_ss, other.m_ss); comp != 0)
      {
        return comp;
      }
      else
      {
        return compareMs(m_ms, other.m_ms);
      }
      
    }
    
    const char* m_yyyy;
    const char* m_mm;
    const char* m_dd;
    const char* m_hh;
    const char* m_min;
    const char* m_ss;
    const char* m_ms;
  };

  static const char* findTimeStampStart(const char* raw)
  {
    uint8_t i = 0;
    while (raw[i] != ',')
    {
      ++i;
    }

    return raw+i+2;
  }

  public:

  MDEntrry(const char* raw) : m_raw(raw), m_timestamp(findTimeStampStart(raw))
  {}

  int operator<=>(const MDEntrry& other) const
  {
    if (int comp = (m_timestamp <=> other.m_timestamp); 0 != comp)
    {
      return comp;
    }
    else
    {
      uint8_t i = 0;
      while (0 == comp &&
            m_raw[i] != ',' &&
            other.m_raw[i] != ',')
      {
        if (m_raw[i] < other.m_raw[i])
        {
          comp = -1;
        }
        else if (m_raw[i] > other.m_raw[i])
        {
          comp = 1;
        }

        ++i;
      }

      if (0 == comp)
      {
        if (m_raw[i] == ',')
        {
          comp = other.m_raw[i] == ','? 0 : -1;
        }
        else
        {
          comp = m_raw[i] == ','? 0 : 1;
        }
      }

      return comp;
    }
  }

  private:
  MDTimeStamp m_timestamp;
  const char* m_raw;
};

int main(int argc, char** argv)
{
  if (argc < 3)
  {
    std::cout << "Invalid no. of args" << std::endl;
    return 1;
  }

  FileReaderProvider frp =
  [](const std::string& filename)
  {
    auto fileHandle = std::make_shared<std::ifstream>(filename, std::ifstream::in);
    FileLineReader flr =
    [fileHandle]
    (char *buff)
    {
      uint8_t ret = 0;
      if(fileHandle->getline(buff, 256); buff[0] != '\0')
      {
        if (memcmp(buff, "Symbol", strlen("Symbol")) == 0 ||
            memcmp(buff, "Timestamp", strlen("Timestamp")) == 0)
        {
          fileHandle->getline(buff, 256);
        }

        uint8_t len = strlen(buff);
        buff[len] = '\n';
        ret = len + 1;
      }

      return ret;
    };

    return flr;
  };

  FileWriterProvider fwp =
  [](const std::string& filename)
  {
    auto fileHandle = std::make_shared<std::ofstream>(filename, std::ofstream::out);
    FileWriter fw =
    [fileHandle]
    (const char* buff, const uint32_t len)
    {
      fileHandle->write(buff, len);
    };

    return fw;
  };


  

  FileMerger fm =
  []
  (const std::function<std::optional<std::tuple<std::string,std::string>>()>& filenameFetcher, 
   const FileReaderProvider& fileReaderProvider,
   const FileWriterProvider& fileWriterProvider,
   const std::function<void(const std::string&, const std::string&, const std::string&)>& outFileNotifier,
   const uint64_t maxHeapSize)
  {
    char* buff = reinterpret_cast<char*>(malloc(maxHeapSize));
    char* curr = buff;
    uint64_t bytesRemaining  = maxHeapSize;
    auto fileNames = filenameFetcher();
    if (!fileNames)
    {
      return false;
    }

    auto const& [fn1, fn2] = fileNames.value();
    std::string symbol1 = "";
    std::string symbol2 = "";
    symbol1.reserve(fn1.length() + 1);
    symbol2.reserve(fn2.length() + 1);

    bool f1IsIntermediateFile = false;
    bool f2IsIntermediateFile = false;

    
    if (std::size_t pos = fn1.find_first_of("."); 
        0 == strcmp(".csv", fn1.c_str() + pos)
       )
    {
      f1IsIntermediateFile = true;
    }
    else
    {
      symbol1 = std::move(fn1.substr(0, pos));
    }

    if (std::size_t pos = fn2.find_first_of("."); 
        0 == strcmp(".csv", fn2.c_str() + pos)
       )
    {
      f1IsIntermediateFile = true;
    }
    else
    {
      symbol2 = std::move(fn2.substr(0, pos));
    }

    auto fileReader1 = fileReaderProvider(fn1);
    auto fileReader2 = fileReaderProvider(fn2);

    //Lines read from each of the files
    char l1[256];
    char l2[256];

    uint32_t nl1 = readNextLine(fileReader1, l1, symbol1);
    uint32_t nl2 = readNextLine(fileReader2, l2, symbol2);
    std::string outFile = generateRandomString(20) + ".csv";
    auto fileWriter = fileWriterProvider(outFile);

    const char* str = "Symbol, Timestamp, Price, Size, Exchange, Type\n";
    fileWriter(str, strlen(str));
    while (nl1 && nl2)
    {
      MDEntrry md1(reinterpret_cast<const char*>(l1));
      MDEntrry md2(reinterpret_cast<const char*>(l2));

      if (md1 <=> md2 < 0)
      {
        if(bytesRemaining < nl1)
        {
          fileWriter(buff, curr - buff);
          curr = buff;
          bytesRemaining = maxHeapSize;
        }

        memcpy(curr, l1, nl1);
        curr += nl1;
        bytesRemaining -= nl1;
        nl1 = readNextLine(fileReader1, l1, symbol1);
      }
      else
      {
        if(bytesRemaining < nl2)
        {
          fileWriter(buff, curr - buff);
          curr = buff;
          bytesRemaining = maxHeapSize;
        }

        memcpy(curr, l2, nl2);
        curr += nl2;
        bytesRemaining -= nl2;
        nl2 = readNextLine(fileReader2, l2, symbol2);
      }
    }

    if (curr > buff)
    {
      fileWriter(buff, curr - buff);
    }

    curr = buff;
    bytesRemaining = maxHeapSize;

    if (!nl1)
    {
      while (nl2 != 0)
      {
        if(bytesRemaining < nl2)
        {
          fileWriter(buff, curr - buff);
          curr = buff;
          bytesRemaining = maxHeapSize;
        }

        memcpy(curr, l2, nl2);
        curr += nl2;
        bytesRemaining -= nl2;
        nl2 = readNextLine(fileReader2, l2, symbol2);
      }
    }

    if (!nl2)
    {
      while (nl1 != 0)
      {
        if(bytesRemaining < nl1)
        {
          fileWriter(buff, curr - buff);
          curr = buff;
          bytesRemaining = maxHeapSize;
        }

        memcpy(curr, l1, nl1);
        curr += nl1;
        bytesRemaining -= nl1;
        nl1 = readNextLine(fileReader1, l1, symbol1);
      }
    }

    if (curr > buff)
    {
      fileWriter(buff, curr - buff);
    }

    free(buff);
    outFileNotifier(fn1, fn2, outFile);
    return true;
  };

  std::function<void(std::queue<std::string>&)> mergeAllFiles = [fm, frp, fwp](std::queue<std::string>& fileNames) {
    const std::function<
    std::optional<std::tuple<std::string, std::string>>()
    > mergeFileFetcher =
    [&fileNames]()
    {
      std::optional<std::tuple<std::string,std::string>> res;
      if(fileNames.size() >= 2)
      {
        std::string f1 = fileNames.front();
        fileNames.pop();
        std::string f2 = fileNames.front();
        fileNames.pop();

        res = std::make_tuple(f1, f2);
      }
      else
      {
        res = std::nullopt;
      }

      return res;
    };

    std::function<void(const std::string&, const std::string, const std::string&)> onOutFileCreated =
    [&fileNames](const std::string& f1, const std::string& f2, const std::string& outFile)
    {
      //csv files are intermediate files
      if(f1.substr(f1.find_first_of(".")).compare(".csv") == 0)
      {
        std::remove(f1.c_str());
      }

      //csv files are intermediate files
      if(f2.substr(f2.find_first_of(".")).compare(".csv") == 0)
      {
        std::remove(f2.c_str());
      }

      fileNames.push(outFile);
    };

    std::function<uint32_t()> fetchNumRemainingFiles =
    [&fileNames]()
    {
      return fileNames.size();
    };

    while(fetchNumRemainingFiles() >= 2)
    {
      fm(mergeFileFetcher, frp, fwp, onOutFileCreated, 2*1024*1024 );
    }
  };

  std::queue<std::string> remainingFiles;
  for (uint32_t i = 1; i < argc; i++)
  {
    remainingFiles.push(argv[i]);
  }

  mergeAllFiles(remainingFiles);
  std::rename(remainingFiles.front().c_str(), "MultiplexedFile.txt");
  return 0;
}