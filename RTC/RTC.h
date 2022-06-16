#ifndef RTC_H
#define RTC_H
#include <Wire.h>
#include "DS3231.h"

class RTC{
  public:
    bool init();
    void get_rtc(String &date, String &times, bool &success);
	void get_rtc_separate(byte &date, byte &month, uint16_t &year, byte &hour, byte &minute, byte &seconds);
    void set_rtc(byte date, byte month, byte year, byte hour, byte minute, byte seconds);
  private:
    char _date[15];
    char _times[15];
    //char _temp[15];
    bool _success;
};
#endif
