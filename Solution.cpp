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

typedef std::function<uint32_t(char[256])> FileLineReader;
typedef std::function<void(const char*, const uint32_t)> FileWriter;
typedef std::function<FileLineReader(const std::string&)> FileReaderProvider;
typedef std::function<FileWriter(const std::string&)> FileWriterProvider;

typedef std::function<void(const std::function<std::optional<std::tuple<std::string,std::string>>()> filenameFetcher, 
                           const std::string& file2,
                           const std::string& outFile,
                           const FileReaderProvider& fileReaderProvider,
                           const FileWriterProvider& fileWriterProvider,
                           const std::string& outFileName,
                           const std::function<void(const std::string&)> outFileNotifier,
                           const uint64_t maxHeapSize)> FileMerger;

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
      m_ss   = m_hh + 3;
      m_ms   = m_ss + 3;
    }

    int compareYear(const char* y1, const char* y2) const
    {
      uint8_t comp = 0;
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
      uint8_t comp = 0;
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
      uint8_t comp = 0;
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
      uint8_t comp = 0;
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
      uint8_t comp = 0;
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
      uint8_t comp = 0;
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
      uint8_t comp = 0;
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
        return compareSec(m_ms, other.m_ms);
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
    (char buff[256])
    {
      char* currPtr = reinterpret_cast<char*>(buff);
      char currCh = '\0';
      for (;currCh != '\n' && currCh != EOF; *(currPtr++) = currCh)
      {
        currCh = fileHandle->get();
      }
      
      uint32_t ret = 0;
      if (EOF != currCh)
      {
        *currPtr = '\n';
        ret = currPtr - buff + 1;
      }

      return ret;
    };

    return flr;
  };

  FileWriterProvider fwp =
  [](const std::string& filename)
  {
    auto fileHandle = std::make_shared<std::ofstream>(filename, std::ofstream::out);
    FileWriter flw =
    [fileHandle]
    (const char* buff, const uint32_t len)
    {
      static bool headerWritten = false;
      if (!headerWritten) 
      {
        (*fileHandle) << "Symbol, Timestamp, Price, Size, Exchange, Type\n";
        headerWritten = true;
      }

      fileHandle->write(buff, len);
    };

    return flw;
  };


  std::queue<std::string> remainingFiles;
  for (uint32_t i = 0; i <= argc; i++)
  {
    remainingFiles.push(argv[i]);
  }

  FileMerger fm =
  []
  (const std::function<std::optional<std::tuple<std::string,std::string>>()>& filenameFetcher, 
   const std::string& file2,
   const std::string& outFile,
   const FileReaderProvider& fileReaderProvider,
   const FileWriterProvider& fileWriterProvider,
   const std::string& outFileName,
   const std::function<void(const std::string&)>& outFileNotifier,
   const uint64_t maxHeapSize)
  {
    char* buff = reinterpret_cast<char*>(malloc(maxHeapSize));
    char* curr = buff;
    uint64_t bytesRemaining  = maxHeapSize;
    auto fileNames = filenameFetcher();
    if (!fileNames)
    {
      return;
    }

    auto const& [fn1, fn2] = fileNames.value();
    auto fileReader1 = fileReaderProvider(fn1);
    auto fileReader2 = fileReaderProvider(fn2);

    //Lines read from each of the files
    char l1[256];
    char l2[256];

    uint32_t nl1 = fileReader1(l1);
    uint32_t nl2 = fileReader2(l2);
    auto fileWriter = fileWriterProvider(outFile);

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
        nl1 = fileReader1(l1);
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
        nl2 = fileReader1(l2);
      }
    }

    if (curr > buff)
    {
      fileWriter(buff, curr - buff);
    }

    free(buff);

  };


  

  return 0;
}