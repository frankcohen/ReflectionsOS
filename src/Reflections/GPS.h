#ifndef _GPS_
#define _GPS_

#include "config.h"
#include "secrets.h"

#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include "time.h"

#define GPSBaud 9600

class GPS
{
  public:
    GPS();
    void begin();
    void loop();
    bool test();

    void printHeader();
    void printLocation();
    void printFloat(float val, bool valid, int len, int prec);
    void printInt(unsigned long val, bool valid, int len);
    void printDateTime(TinyGPSDate &d, TinyGPSTime &t);
    void printStr(const char *str, int len);

    bool isActive();

  private:
    TinyGPSPlus gps;
    bool active;
    unsigned long gpstime;
};

#endif // _GPS_