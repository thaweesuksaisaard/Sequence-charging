#include "RTC.h"

RTClib myRTC;
DS3231 ds;
bool RTC::init(){
  if(Wire.begin())
  {
    return true;
  } else
  {
    return false;
  }
}

void RTC::get_rtc(String &date, String &times, bool &success){
  success = false;
  DateTime now = myRTC.now();
  sprintf(_date, "%02d/%02d/%02d", now.day(), now.month(), now.year());
  sprintf(_times, "%02d/%02d/%02d", now.hour(), now.minute(), now.second());
  date = String(_date);
  times = String(_times);
  success = true;
}

void RTC::get_rtc_separate(byte &date, byte &month, uint16_t &year, byte &hour, byte &minute, byte &seconds)
{
	 DateTime now = myRTC.now();
	 date = now.day();
	 month = now.month();
	 year = now.year();
	 
	 hour = now.hour();
	 minute = now.minute();
	 seconds = now.second();
}

void RTC::set_rtc(byte date, byte month, byte year, byte hour, byte minute, byte seconds){
  ds.setHour(hour);
  ds.setMinute(minute);
  ds.setSecond(seconds);
  //ds.setDoW(3);
  ds.setDate(date);
  ds.setMonth(month);
  ds.setYear(year);
}
