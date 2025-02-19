#pragma once

#include <stdint.h>
struct MDTimeStamp
{
  
  public:
  // "raw" should be in the format:
  // Symbol, Timestamp, Price, Size, Exchange, Type

  MDTimeStamp(const char* timestamp);

  int compareYear(const char* y1, const char* y2) const;

  int compareMonth(const char* m1, const char* m2) const;
 
  int compareday(const char* d1, const char* d2) const;

  int compareHour(const char* h1, const char* h2) const;

  int compareMinute(const char* min1, const char* min2) const;

  int compareSec(const char* s1, const char* s2) const;

  int compareMs(const char* ms1, const char* ms2) const;

  int operator <=>(const MDTimeStamp& other) const;

  private:
  const char* m_yyyy;
  const char* m_mm;
  const char* m_dd;
  const char* m_hh;
  const char* m_min;
  const char* m_ss;
  const char* m_ms;
};

class MDEntrry
{

  static const char* findTimeStampStart(const char* raw);
  public:

  MDEntrry(const char* raw);

  int operator<=>(const MDEntrry& other) const;

  private:
  MDTimeStamp m_timestamp;
  const char* m_raw;
};
