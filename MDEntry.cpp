#include "MDEntry.h"

MDTimeStamp::MDTimeStamp(const char* timestamp)
{
  m_yyyy = timestamp;
  m_mm   = m_yyyy + 5;
  m_dd   = m_mm + 3;
  m_hh   = m_dd + 3;
  m_min  = m_hh + 3;
  m_ss   = m_min + 3;
  m_ms   = m_ss + 3;
}

int MDTimeStamp::compareYear(const char* y1, const char* y2) const
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

int MDTimeStamp::compareMonth(const char* m1, const char* m2) const
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

int MDTimeStamp::compareday(const char* d1, const char* d2) const
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

int MDTimeStamp::compareHour(const char* h1, const char* h2) const
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

int MDTimeStamp::compareMinute(const char* min1, const char* min2) const
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

int MDTimeStamp::compareSec(const char* s1, const char* s2) const
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

int MDTimeStamp::compareMs(const char* ms1, const char* ms2) const
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

int MDTimeStamp::operator <=>(const MDTimeStamp& other) const
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


MDEntrry::MDEntrry(const char* raw) : m_raw(raw), m_timestamp(findTimeStampStart(raw))
{}

int MDEntrry::operator<=>(const MDEntrry& other) const
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

const char* MDEntrry::findTimeStampStart(const char* raw)
{
  uint8_t i = 0;
  while (raw[i] != ',')
  {
    ++i;
  }

  return raw+i+2;
}
