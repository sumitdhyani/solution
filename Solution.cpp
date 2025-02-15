#include <functional>
#include <memory>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <tuple>
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

class MDEntrry
{
  struct MDTimeSTamp
  {
    
    public:
    // "raw" should be in the format:
    // Symbol, Timestamp, Price, Size, Exchange, Type

    MDTimeSTamp(const char* timestamp)
    {
      m_yyyy = timestamp;
      m_mm   = m_yyyy + 5;
      m_dd   = m_mm + 3;
      m_hh   = m_dd + 3;
      m_min  = m_hh + 3;
      m_ss   = m_hh + 3;
      m_ms   = m_ss + 3;
    }

    int compareYear(const char* y1, const char* y2)
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

    int compareMonth(const char* m1, const char* m2)
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

    int compareday(const char* d1, const char* d2)
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

    int compareHour(const char* h1, const char* h2)
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

    int compareMinute(const char* min1, const char* min2)
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

    int compareSec(const char* s1, const char* s2)
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

    int compareMs(const char* ms1, const char* ms2)
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

    int operator <=>(const MDTimeSTamp& other)
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

  MDEntrry(std::string&& raw) : m_timestamp(raw.c_str() + raw.find_first_of(",") + 1), m_raw(raw)
  {
  }

  int operator<=>(const MDEntrry& other)
  {
    if (int comp = (m_timestamp <=> other.m_timestamp); 0 != comp)
    {
      return comp;
    }
    else
    {
      comp = 0;
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
  MDTimeSTamp m_timestamp;
  std::string m_raw;
};

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