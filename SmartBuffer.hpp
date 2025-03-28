#include <functional>
#include <string.h>
struct SyncIOReadBuffer
{
  typedef std::function<uint32_t(char*, const uint32_t&)> DataSourcer;
  enum class LastOperation
  {
    COPY,
    PASTE,
    NONE
  };

  SyncIOReadBuffer(const uint32_t& size) : 
    m_readBuff(reinterpret_cast<char*>(malloc(size))),
    m_tail(0),
    m_head(0),
    m_size(size),
    m_lastOperation(LastOperation::NONE)
  {
  }

  uint32_t read(char* const& out, 
                const uint32_t& len,
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

  uint32_t readUntil(char* const& out,
                     const DataSourcer& dataSourcer,
                     const char& ender)
  {
    uint32_t ret = 0;
    uint32_t offset = 0;
    uint32_t occBytes = occupiedBytes();
    if(!occBytes)
    {
      occBytes = paste(dataSourcer);
    }

    if (occBytes)
    {
      for (;
          offset < occBytes && m_readBuff[(m_tail + offset) % m_size] != ender;
          ++offset);
      
      // Found ender
      if (ender == m_readBuff[(m_tail + offset) % m_size])
      {
        copy(out, offset+1);
        ret = offset+1;
      }
      // Didn't find the ender
      else
      {
        // Source the data from IO Interface
        if(uint32_t bytesPasted = paste(dataSourcer);
          bytesPasted > 0)// Non-zero no. of bytes read
        {
          ret = readUntil(out, dataSourcer, ender);
        }
        else// EOF reached, but there's still some data in the buffer
        {
          ret = occupiedBytes();
          copy(out, ret);
        }
      }
    }

    return ret;
  }

  ~SyncIOReadBuffer()
  {
    free(m_readBuff);
  }

  SyncIOReadBuffer(const SyncIOReadBuffer&) = delete;
  SyncIOReadBuffer& operator =(const SyncIOReadBuffer&) = delete;
  SyncIOReadBuffer(SyncIOReadBuffer&&) = delete;
  SyncIOReadBuffer& operator =(SyncIOReadBuffer&&) = delete;

  private:

  void copy(char* const& out, const uint32_t& len)
  {
    if (!len)
    {
        return;
    }

    if (m_tail < m_head || 
        len <= (m_size - m_tail))
    {
      memcpy(out, m_readBuff + m_tail, len);
      m_tail = (m_tail + len) % m_size;
    }
    else
    {
      const uint32_t l1 = m_size - m_tail;
      const uint32_t l2 = len - l1;
      memcpy(out, m_readBuff + m_tail, l1);
      memcpy(out + l1, m_readBuff, l2);
      m_tail = l2;
    }

    m_lastOperation = LastOperation::COPY;
    if (0 == occupiedBytes())
    {
      m_head = m_tail = 0;
    }
  }

  uint32_t paste(const DataSourcer& dataSourcer)
  {
    uint32_t bytesReadFromSourcer = 0;
    if (m_head < m_tail)
    {
      bytesReadFromSourcer = dataSourcer(m_readBuff + m_head, m_tail - m_head);
      m_head += bytesReadFromSourcer;
    }
    else
    {
      uint32_t lengthTillEnd = m_size - m_head;
      bytesReadFromSourcer = dataSourcer(m_readBuff + m_head, lengthTillEnd);
      m_head += bytesReadFromSourcer;
      if (bytesReadFromSourcer == lengthTillEnd)
      {
        uint32_t temp = dataSourcer(m_readBuff, freeBytes());
        bytesReadFromSourcer += temp;
        m_head = temp;
      }
    }
    
    if (bytesReadFromSourcer)
    {
        m_lastOperation = LastOperation::PASTE;
    }

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
  const uint32_t m_size;
  char* const m_readBuff;
};

struct SyncIOLazyWriteBuffer
{
  typedef std::function<void(char*, const uint32_t&)> DataWriter;
  enum class LastOperation
  {
    FLUSH,
    PUT,
    NONE
  };

  SyncIOLazyWriteBuffer(const uint32_t& size, const DataWriter& dataWriter) : 
    m_outBuff(reinterpret_cast<char*>(malloc(size))),
    m_tail(0),
    m_head(0),
    m_size(size),
    m_dataWriter(dataWriter),
    m_lastOperation(LastOperation::NONE)
  {
  }

  void write(const char* out, const uint32_t& len)
  {
    uint32_t remainingLen = len;
    while (freeBytes() < remainingLen)
    {
      uint32_t freeBytesBeforePut = freeBytes();
      put(out, freeBytes());
      remainingLen -= freeBytesBeforePut;
      out += freeBytesBeforePut;
      flush();
    }

    put(out, remainingLen);
  }

  ~SyncIOLazyWriteBuffer()
  {
    flush();
    free(m_outBuff);
  }

  SyncIOLazyWriteBuffer(const SyncIOLazyWriteBuffer&) = delete;
  SyncIOLazyWriteBuffer& operator =(const SyncIOLazyWriteBuffer&) = delete;
  SyncIOLazyWriteBuffer(SyncIOLazyWriteBuffer&&) = delete;
  SyncIOLazyWriteBuffer& operator =(SyncIOLazyWriteBuffer&&) = delete;

  private:

  // Call this only when freeBytes() <= len
  void put(const char* outData, const uint32_t& len)
  {
    if (!len)
    {
        return;
    }

    if (m_head < m_tail ||
        len <= m_size - m_head)
    {
      memcpy(m_outBuff + m_head, outData, len);
      m_head += len;
    }
    else
    {
      const uint32_t l1 = m_size - m_head;
      const uint32_t l2 = len - l1;
      memcpy(m_outBuff + m_head, outData, l1);
      memcpy(m_outBuff, outData + l1, l2);
      m_head = l2;
    }

    m_lastOperation = LastOperation::PUT;
  }

  void flush()
  {
    if (!occupiedBytes())
    {
      return;
    }

    if (m_tail < m_head)
    {
      m_dataWriter(m_outBuff + m_tail, occupiedBytes());
    }
    else
    {
      m_dataWriter(m_outBuff + m_tail, m_size - m_tail);
      m_dataWriter(m_outBuff, m_head);
    }

    m_tail = m_head = 0;
    m_lastOperation = LastOperation::FLUSH;
  }
  
  uint32_t occupiedBytes()
  {
    if (m_tail == m_head)
    {
      return m_lastOperation == LastOperation::PUT? m_size : 0;
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
  const DataWriter m_dataWriter;
  uint32_t m_tail;
  uint32_t m_head;
  const uint32_t m_size;
  char* const m_outBuff;
};


#include <optional>
template <class ErrType>
struct AsyncIOWriteBuffer
{
  using WriteCallback = std::function<void(const uint32_t&)>;
  using ErrProcessor = std::function<void(const ErrType&)>;
  using WriteResutHandler = std::function<void(const uint32_t&, const std::optional<ErrType>&)>;
  using DataWriter = std::function<void(const char*, const uint32_t&, const WriteResutHandler&)>;
  
  enum class LastOperation
  {
    FLUSH,
    PUT,
    NONE
  };

  AsyncIOWriteBuffer(const uint32_t& size,
                     const DataWriter& dataWriter,
                     const ErrProcessor& errProcessor,
                     const WriteCallback& writeCallback
                     ) : 
    m_outBuff(reinterpret_cast<char*>(malloc(size))),
    m_tail(0),
    m_head(0),
    m_size(size),
    m_pendingWriteCompletions(0),
    m_interfaceInvalidated(false),
    m_dataWriter(dataWriter),
    m_writeCallback(writeCallback),
    m_errProcessor(dataWriter),
    m_lastOperation(LastOperation::NONE)
  {
  }

  void write(const char* out, const uint32_t& len)
  {
    if(m_interfaceInvalidated)
    {
      return;
    }

    if (m_pendingWriteCompletions)
    {
      uint32_t remainingLen = len;
      while (auto freeBytesBeforePut = freeBytes() < remainingLen)
      {
        put(out, freeBytesBeforePut);
        remainingLen -= freeBytesBeforePut;
        out += freeBytesBeforePut;
        flush();
      }

      put(out, remainingLen);
    }
    else
    {
      writeToInterface(out, len);
    }
  }

  ~AsyncIOWriteBuffer()
  {
    free(m_outBuff);
  }

  AsyncIOWriteBuffer(const AsyncIOWriteBuffer&) = delete;
  AsyncIOWriteBuffer& operator =(const AsyncIOWriteBuffer&) = delete;
  AsyncIOWriteBuffer(AsyncIOWriteBuffer&&) = delete;
  AsyncIOWriteBuffer& operator =(AsyncIOWriteBuffer&&) = delete;

  private:

  void onWriteComplete()
  {
    if (!(--m_pendingWriteCompletions))
    {
      flush();
    }
  }

  // Call this only when freeBytes() >= len
  void put(const char* outData, const uint32_t& len)
  {
    if (!len)
    {
        return;
    }

    if (m_head < m_tail ||
        len <= m_size - m_head)
    {
      memcpy(m_outBuff + m_head, outData, len);
      m_head += len;
    }
    else
    {
      const uint32_t l1 = m_size - m_head;
      const uint32_t l2 = len - l1;
      memcpy(m_outBuff + m_head, outData, l1);
      memcpy(m_outBuff, outData + l1, l2);
      m_head = l2;
    }

    m_lastOperation = LastOperation::PUT;
  }

  void flush()
  {
    if (!occupiedBytes())
    {
      return;
    }

    if (m_tail < m_head)
    {
      writeToInterface(m_outBuff + m_tail, occupiedBytes());
      m_tail = m_head = 0;
    }
    else
    {
      writeToInterface(m_outBuff + m_tail, m_size - m_tail);
      m_tail = 0;
    }

    m_lastOperation = LastOperation::FLUSH;
  }

  void writeToInterface(const char* out, const uint32_t& len)
  {
    ++m_pendingWriteCompletions;
    m_dataWriter(out,
                 len,
                 [this](const uint32_t& len, const std::optional<ErrType>& err)
                 {
                   if (err)
                   {
                    // Call errCallback only once
                    m_interfaceInvalidated = true;
                    if(!(--m_pendingWriteCompletions))
                    {
                      m_errProcessor(err);
                    }
                   }
                   else
                   {
                    onWriteComplete();
                    m_writeCallback(len);
                   }
                 });
  }

  uint32_t occupiedBytes()
  {
    if (m_tail == m_head)
    {
      return m_lastOperation == LastOperation::PUT? m_size : 0;
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
  const DataWriter m_dataWriter;
  const ErrProcessor m_errProcessor;
  const WriteCallback m_writeCallback;
  uint32_t m_tail;
  uint32_t m_head;
  const uint32_t m_size;
  char* const m_outBuff;
  uint32_t m_pendingWriteCompletions;
  bool m_interfaceInvalidated;
};