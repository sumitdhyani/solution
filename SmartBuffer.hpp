#pragma once
#include <functional>
#include <string.h>

template <class SizeType>
struct SyncIOReadBuffer
{
  typedef std::function<SizeType(char*, const SizeType&)> DataSourcer;
  enum class LastOperation
  {
    COPY,
    PASTE,
    NONE
  };

  SyncIOReadBuffer(const SizeType& size) : 
    m_readBuff(reinterpret_cast<char*>(malloc(size))),
    m_tail(0),
    m_head(0),
    m_size(size),
    m_lastOperation(LastOperation::NONE)
  {
  }

  SizeType read(char* const& out, 
                const SizeType& len,
                const DataSourcer& dataSourcer)
  {
    SizeType ret = 0;
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

  SizeType readUntil(char* const& out,
                     const DataSourcer& dataSourcer,
                     const char& ender)
  {
    SizeType ret = 0;
    SizeType offset = 0;
    SizeType occBytes = occupiedBytes();
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
        copy(out, occBytes);
          // Source the data from IO Interface
        if(SizeType bytesPasted = paste(dataSourcer);
          bytesPasted > 0)// Non-zero no. of bytes read
        {
          ret = readUntil(out + occBytes, dataSourcer, ender);
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

  SizeType readUntil(char* const& out,
                     const DataSourcer& dataSourcer,
                     const std::function<bool(const char&)>& ender)
  {
    SizeType ret = 0;
    SizeType offset = 0;
    SizeType occBytes = occupiedBytes();
    if(!occBytes)
    {
      occBytes = paste(dataSourcer);
    }

    if (occBytes)
    {
      for (;
           offset < occBytes && !ender(m_readBuff[(m_tail + offset) % m_size]);
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
        copy(out, occBytes);
        // Source the data from IO Interface
        if(SizeType bytesPasted = paste(dataSourcer);
           bytesPasted > 0)// Non-zero no. of bytes read
        {
          ret = readUntil(out + occBytes, dataSourcer, ender);
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
  // Assumes that len <= occupiedBytes, so the caller of this function has to
  // take care of that
  void copy(char* const& out, const SizeType& len)
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
      const SizeType l1 = m_size - m_tail;
      const SizeType l2 = len - l1;
      memcpy(out, m_readBuff + m_tail, l1);
      memcpy(out + l1, m_readBuff, l2);
      m_tail = l2;
    }

    m_lastOperation = LastOperation::COPY;
    if (!occupiedBytes())
    {
      m_head = m_tail = 0;
    }
  }

  // Read from IOInterface, takes into account fragmentation in free memory
  SizeType paste(const DataSourcer& dataSourcer)
  {
    SizeType bytesReadFromSourcer = 0;
    if (auto free = freeBytes(); free)
    {
      SizeType lengthTillEnd = m_size - m_head;
      SizeType toRead = std::min(lengthTillEnd, free);
      
      bytesReadFromSourcer = pasteFromInterface(dataSourcer, toRead);
      free -= bytesReadFromSourcer;
      if (bytesReadFromSourcer == toRead && free)
      {
        bytesReadFromSourcer += pasteFromInterface(dataSourcer, free);
      }
    }

    return bytesReadFromSourcer;
  }

  // Read from IOInterface, assumes that the provided memory is
  // contiguous
  SizeType pasteFromInterface(const DataSourcer& dataSourcer, const SizeType& len)
  {
    SizeType ret = 0;
    if(len)
    {
      ret = dataSourcer(m_readBuff + m_head, len);
      if (ret)
      {
        m_head += ret;
        if(m_size == m_head)
        {
          m_head = 0;
        }

        m_lastOperation = LastOperation::PASTE;
      }
    }

    return ret;
  }


  SizeType occupiedBytes()
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

  SizeType freeBytes()
  {
    return m_size - occupiedBytes();
  }

  LastOperation m_lastOperation;
  SizeType m_tail;
  SizeType m_head;
  const SizeType m_size;
  char* const m_readBuff;
};

template <class SizeType>
struct SyncIOLazyWriteBuffer
{
  typedef std::function<void(char*, const SizeType&)> DataWriter;
  enum class LastOperation
  {
    FLUSH,
    PUT,
    NONE
  };

  SyncIOLazyWriteBuffer(const SizeType& size, const DataWriter& dataWriter) : 
    m_outBuff(reinterpret_cast<char*>(malloc(size))),
    m_tail(0),
    m_head(0),
    m_size(size),
    m_dataWriter(dataWriter),
    m_lastOperation(LastOperation::NONE)
  {
  }

  void write(const char* out, const SizeType& len)
  {
    SizeType remainingLen = len;
    while (freeBytes() < remainingLen)
    {
      SizeType freeBytesBeforePut = freeBytes();
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
  void put(const char* outData, const SizeType& len)
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
      const SizeType l1 = m_size - m_head;
      const SizeType l2 = len - l1;
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
  
  SizeType occupiedBytes()
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

  SizeType freeBytes()
  {
    return m_size - occupiedBytes();
  }

  LastOperation m_lastOperation;
  const DataWriter m_dataWriter;
  SizeType m_tail;
  SizeType m_head;
  const SizeType m_size;
  char* const m_outBuff;
};

#include <optional>

template <class ErrType, class SizeType> 
struct AsyncIOReadBuffer
{
  using MaybeErr = std::optional<ErrType>;
  using ReadHandler = std::function<void(const SizeType&, const MaybeErr&)>;
  using DataSourcer = std::function<void(char* const&, const SizeType&, const ReadHandler&)>;

  enum class LastOperation
  {
    COPY,
    PASTE,
    NONE
  };

  AsyncIOReadBuffer(const SizeType& size) :
  m_readBuff(reinterpret_cast<char*>(malloc(size))),
  m_tail(0),
  m_head(0),
  m_size(size),
  m_lastOperation(LastOperation::NONE)
  {
  }

  ~AsyncIOReadBuffer()
  {
    free(m_readBuff);
  }

  void read(char* const& out,
            const SizeType& len,
            const DataSourcer& dataSourcer,
            const ReadHandler& readhandler)
  {
    SizeType ret = 0;
    if (occupiedBytes() >= len)
    {
      copy(out, len);
      readhandler(len, std::nullopt);
    }
    else
    {
      paste(dataSourcer,
            [readhandler](const SizeType& len, const MaybeErr& err)
            {
              const SizeType occBytes = occupiedBytes();
              const SizeType ret = occBytes >= len? len : occBytes;
              copy(out, ret);
              readhandler(ret, err);
            }
      );
    }
  }

  SizeType readUntil(char* const& out,
                     const char& ender,
                     const DataSourcer& dataSourcer,
                     const ReadHandler& readhandler)
  {
    SizeType ret = 0;
    SizeType offset = 0;
    SizeType occBytes = occupiedBytes();
    if(!occBytes)
    {
      paste(dataSourcer, 
            [out, ender, dataSourcer, readhandler](const SizeType& len, const MaybeErr& err)
            {
              if(!err)
              {
                readUntil(out, ender, dataSourcer, readhandler);
              }
              else
              {
                const SizeType occBytes = occupiedBytes();
                const SizeType ret = occBytes >= len? len : occBytes;
                copy(out, ret);
                readhandler(ret, err);
              }
            }
      );
    }
    else
    {
      for (;
          offset < occBytes && m_readBuff[(m_tail + offset) % m_size] != ender;
          ++offset);

      // Found ender
      if (ender == m_readBuff[(m_tail + offset) % m_size])
      {
        copy(out, offset+1);
        readhandler(offset+1, std::nullopt);
      }
      // Didn't find the ender
      else
      {
        auto const callback =

        paste(dataSourcer, 
              [out, ender, dataSourcer, readhandler](const SizeType& len, const MaybeErr& err)
              {
                if(!err)
                {
                  readUntil(out, ender, dataSourcer, readhandler);
                }
                else
                {
                  SizeType occBytes = occupiedBytes();
                  for (SizeType offset = 0; offset < occBytes && m_readBuff[(m_tail + offset) % m_size] != ender; ++offset);
                  copy(out, offset + 1);
                  readhandler(offset + 1, err);
                }
              }
        );
      }
    }
  }


  AsyncIOReadBuffer(const AsyncIOReadBuffer&) = delete;
  AsyncIOReadBuffer& operator =(const AsyncIOReadBuffer&) = delete;
  AsyncIOReadBuffer(AsyncIOReadBuffer&&) = delete;
  AsyncIOReadBuffer& operator =(AsyncIOReadBuffer&&) = delete;

  private:

  SizeType occupiedBytes()
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

  SizeType freeBytes()
  {
    return m_size - occupiedBytes();
  }

  void copy(char* const& out, const SizeType& len)
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
      const SizeType l1 = m_size - m_tail;
      const SizeType l2 = len - l1;
      memcpy(out, m_readBuff + m_tail, l1);
      memcpy(out + l1, m_readBuff, l2);
      m_tail = l2;
    }

    m_lastOperation = LastOperation::COPY;
    if (m_head == m_tail)
    {
      m_head = m_tail = 0;
    }
  }

  SizeType paste(const DataSourcer& dataSourcer, const ReadHandler& readHandler)
  {
    SizeType bytesReadFromSourcer = 0;
    if (m_head < m_tail)
    {
      readFromInterface(m_readBuff + m_head, m_tail - m_head, readHandler);
    }
    else
    {
      SizeType lengthTillEnd = m_size - m_head;
      readFromInterface(m_readBuff + m_head,
                        lengthTillEnd,
                        [this, readHandler, lengthTillEnd, dataSourcer](const SizeType& len, const MaybeErr& err)
                        {
                          if (len < lengthTillEnd)
                          {
                            readHandler(len, err);
                          }
                          else
                          {
                            readFromInterface(m_readBuff + m_head, freeBytes(), dataSourcer, readHandler);
                          }
                        }
      );
    }
    
    if (bytesReadFromSourcer)
    {
        m_lastOperation = LastOperation::PASTE;
    }

    return bytesReadFromSourcer;
  }

  void readFromInterface(char* const& out, const SizeType& len, const DataSourcer& dataSourcer, const ReadHandler& readhandler)
  {
    dataSourcer(out,
                len,
                [this, readhandler](const SizeType& len, const MaybeErr& err) { onBytesReadFromInterface(len, err, readhandler); }
    );
  }

  void onBytesReadFromInterface(const SizeType& len, const MaybeErr& err, const ReadHandler& readhandler)
  {
    m_head = (m_head + len) % m_size;

    if (len)
    {
      m_lastOperation = LastOperation::PASTE;
    }

    readhandler(len, err);
  }

  char* const m_readBuff;
  const SizeType m_size;
  SizeType m_tail;
  SizeType m_head;
  LastOperation m_lastOperation;

};

template <class ErrType, class SizeType>
struct AsyncIOWriteBuffer
{
  using WriteCallback = std::function<void(const SizeType&)>;
  using ErrProcessor = std::function<void(const ErrType&)>;
  using WriteResutHandler = std::function<void(const SizeType&, const std::optional<ErrType>&)>;
  using DataWriter = std::function<void(const char*, const SizeType&, const WriteResutHandler&)>;
  
  enum class LastOperation
  {
    FLUSH,
    PUT,
    NONE
  };

  AsyncIOWriteBuffer(const SizeType& size,
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

  void write(const char* out, const SizeType& len)
  {
    if(m_interfaceInvalidated)
    {
      return;
    }

    if (m_pendingWriteCompletions)
    {
      SizeType remainingLen = len;
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
  void put(const char* outData, const SizeType& len)
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
      const SizeType l1 = m_size - m_head;
      const SizeType l2 = len - l1;
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

  void writeToInterface(const char* out, const SizeType& len)
  {
    ++m_pendingWriteCompletions;
    m_dataWriter(out,
                 len,
                 [this](const SizeType& len, const std::optional<ErrType>& err)
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

  SizeType occupiedBytes()
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

  SizeType freeBytes()
  {
    return m_size - occupiedBytes();
  }

  LastOperation m_lastOperation;
  const DataWriter m_dataWriter;
  const ErrProcessor m_errProcessor;
  const WriteCallback m_writeCallback;
  SizeType m_tail;
  SizeType m_head;
  const SizeType m_size;
  char* const m_outBuff;
  SizeType m_pendingWriteCompletions;
  bool m_interfaceInvalidated;
};