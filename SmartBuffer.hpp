#include <functional>

struct SmartBuffer
{
  typedef std::function<uint32_t(char*, const uint32_t)> DataSourcer;
  enum class LastOperation
  {
    COPY,
    PASTE,
    NONE
  };

  SmartBuffer(const uint32_t size) : 
    m_tail(0),
    m_head(0),
    m_end(size - 1),
    m_size(size),
    m_lastOperation(LastOperation::NONE)
  {
    m_ptr = reinterpret_cast<char*>(malloc(size));
  }

  uint32_t read(char* out, 
                const uint32_t len,
                const DataSourcer& dataSourcer)
  {
    uint32_t ret = 0;
    if (occupiedBytes() >= len)
    {
      copy(out, len);
      ret = len;
    }
    else
    {
      paste(dataSourcer);
      ret = occupiedBytes() >= len? len : occupiedBytes();
      copy(out, ret);
    }

    return ret;
  }

  uint32_t readUntil(char* out,
                     const DataSourcer& dataSourcer,
                     const char ender)
  {
    if (0 == occupiedBytes())
    {
      paste(dataSourcer);
    }
    uint32_t ret = 0;
    uint32_t offset = 0;
    uint32_t occBytes = occupiedBytes();
    for (;
         offset < occBytes && m_ptr[(m_tail + offset) % m_size] != ender;
         ++offset);
    
    // Didn't find the ender
    if ((m_tail + offset) % m_size == m_head)
    {
      if(uint32_t bytesPasted = paste(dataSourcer);
         bytesPasted > 0)//Non-zero no. of bytes read
      {
        return readUntil(out, dataSourcer, ender);
      }
      else// EOF reached
      {
        copy(out, offset+1);
        ret = offset+1;
      }
    }
    else
    {
      copy(out, offset+1);
      ret = offset+1;
    }

    return ret;
  }

  ~SmartBuffer()
  {
    free(m_ptr);
  }

  private:

  void copy(char* out, const uint32_t len)
  {
    if (m_tail < m_head || 
        len <= (m_end - m_tail + 1))
    {
      memcpy(out, m_ptr + m_tail, len);
      m_tail += len;
    }
    else
    {
      const uint32_t l1 = m_end - m_tail + 1;
      const uint32_t l2 = len - l1;
      memcpy(out, m_ptr + m_tail, l1);
      memcpy(out + l1, m_ptr, l2);
      m_tail = l2;
    }

    m_lastOperation = LastOperation::COPY;
    if (0 == occupiedBytes())
    {
      m_head = m_tail = 0;
    }
  }

  uint32_t paste(const std::function<uint32_t(char*, const uint32_t)>& dataSourcer)
  {
    uint32_t bytesReadFromSourcer = 0;
    if (m_head < m_tail)
    {
      bytesReadFromSourcer = dataSourcer(m_ptr + m_head, m_tail - m_head);
      m_head += bytesReadFromSourcer;
    }
    else
    {
      uint32_t lengthTillEnd = m_end - m_head + 1;
      bytesReadFromSourcer = dataSourcer(m_ptr + m_head, lengthTillEnd);
      m_head += bytesReadFromSourcer;
      if (bytesReadFromSourcer == lengthTillEnd)
      {
        uint32_t temp = dataSourcer(m_ptr, freeBytes());
        bytesReadFromSourcer += temp;
        m_head = temp;
      }
    }
    
    m_lastOperation = LastOperation::PASTE;
    return bytesReadFromSourcer;
  }

  uint32_t occupiedBytes()
  {
    if (m_tail == m_head)
    {
      return m_lastOperation == LastOperation::COPY || m_lastOperation == LastOperation::NONE ? 0 : m_size;
    }
    else if (m_tail < m_head)
    {
      return m_head - m_tail;
    }
    else
    {
      return m_size - (m_tail - m_head);
    }
  }

  uint32_t freeBytes()
  {
    return m_size - occupiedBytes();
  }

  LastOperation m_lastOperation;
  uint32_t m_tail;
  uint32_t m_head;
  const uint32_t m_end;
  const uint32_t m_size;
  char* m_ptr;
};
