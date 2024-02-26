#ifndef _ACCELLEROMETER_
#define _ACCELLEROMETER_

#include "config.h"
#include "secrets.h"

#include "Storage.h"
#include "Haptic.h"
#include "Utils.h"

#include "LIS3DHTR.h"
#include <Wire.h>
#include "SD.h"
#include "SPI.h"

#define tollerance 1.05
#define windowtime 1000
#define scaler 200
#define scalemax 1000
#define scalemin -1000

#define maxgestures 10
#define maxframes 50
#define framedelay 40
#define detectdelay 500

class Accellerometer
{
  public:
    Accellerometer();
    void begin();
    void loop();
    bool test();
    void setTraining( bool mode );
    
  private:
    float DTW_THRESHOLD( int gesturenum );
    bool getAccelValues();
    bool detectStartOfGesture();
    bool saveGesture();
    bool saveRatings();
    bool loadGesture();
    float DTWNORM( float dx, float dy, float dz);
    float DTWMIN( float &a, float &b ,float &c );

    LIS3DHTR<TwoWire> LIS;

    bool trainingMode;
    unsigned long movetimer;
    float olddtw;
    float newdtw;
    unsigned long detecttimer;
    unsigned long recordtimer;
    int recordi;
    bool gesturesloaded;
    bool currentreading;
    bool gesturestart;
    int cmpcount;
    bool recordingTemplate;

    float accxt[maxframes];
    float accyt[maxframes];
    float acczt[maxframes];

    float accx[maxgestures][maxframes];
    float accy[maxgestures][maxframes];
    float accz[maxgestures][maxframes];

    int reward[ maxgestures ];

    int gesturenumber;
    int gesturecount;

    bool firsttime = true;
    int movecount = 1;

    float nx0, nx1, nx2, nx3, nx4;
    float ny0, ny1, ny2, ny3, ny4;
    float nz0, nz1, nz2, nz3, nz4;
    float nx, ny, nz;

    long timer = 0;

    int8_t state = 0;
    int8_t co = 0;

    float DTW[ maxframes ][ maxframes ];

    float dx = 0;
    float dy = 0;
    float dz = 0;

    float threshold = 0;

};

#endif // _ACCELLEROMETER_
