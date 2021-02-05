#include <SoftwareSerial.h>
#include <TinyGPS++.h>

static const int RXPin = 35, TXPin = -1;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps; // The TinyGPS++ object
SoftwareSerial ss(RXPin, TXPin); // The serial connection to the GPS device

void setup() 
{
  Serial.begin(115200);
  Serial.println( "Reflections GPS utility" );      
  ss.begin(GPSBaud);
}

void loop()
{
  while (ss.available() > 0)
  if (gps.encode(ss.read()))
  {
    displayInfo();
  }
}

// GPS displayInfo
void displayInfo() 
{
  if (gps.location.isValid()) {
    double latitude = (gps.location.lat());
    double longitude = (gps.location.lng());
  
    Serial.print( "lat = " );
    Serial.print( latitude );
    Serial.print( " lon = " );
    Serial.println( longitude );
  }
}
  
