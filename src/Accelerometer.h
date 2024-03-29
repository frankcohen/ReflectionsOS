#ifndef _ACCELEROMETER_
#define _ACCELEROMETER_

#include "config.h"
#include "secrets.h"

#include "Storage.h"
#include "Haptic.h"
#include "Utils.h"

#include "LIS3DHTR.h"
#include <Wire.h>
#include "SD.h"
#include "SPI.h"

#define tollerance 0.10
#define windowtime 1000
#define scaler 200
#define scalemax 300
#define scalemin -300

// Version number for .ages file
#define ages_version 1

// For example, next, previous, cancel, and accept are gesturetypes
#define gesturetypes 4
#define type1 "next"
#define type2 "previous"
#define type3 "cancel"
#define type4 "accept"

// Records a template for each gesturetype.
#define maxgestures 5

// Number of frames for each gesture template
#define maxframes 50

// Milliseconds delay between frames
#define framedelay 40

class Accelerometer
{
  public:
    Accelerometer();
    void begin();
    void loop();
    bool test();
    void setTraining( bool mode );
    bool detectStartOfGesture();
    bool saveGestures();
    bool loadGestures();
    
  private:
    bool getAccelValues();
    float timeWarp( int gestureIndex, int typen );
    float DTWgpt( float seq1[][3], float seq2[][3], int len );
    float DTWdistance(float x1, float y1, float z1, float x2, float y2, float z2);
    float DTWmin(float a, float b, float c);

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
    bool firstnotice;
    int typecounter;

    float accxt[maxframes];
    float accyt[maxframes];
    float acczt[maxframes];

    float accx[maxgestures][maxframes][gesturetypes];
    float accy[maxgestures][maxframes][gesturetypes];
    float accz[maxgestures][maxframes][gesturetypes];

    float aavgs[ gesturetypes ];
    float tophigh[ gesturetypes ];

    float dtwlow[ gesturetypes ];
    float dtwttl[ gesturetypes ];
    float dtwavg[ gesturetypes ];
    float cost[maxframes][maxframes];
    float seq1[maxframes][3];   // current recording
    float seq2[maxframes][3];   // saved gestures

    int gesturenumber;
    int gesturecount;

    bool firsttime = true;

    float nx0, nx1, nx2, nx3;
    float ny0, ny1, ny2, ny3;
    float nz0, nz1, nz2, nz3;
    float nx, ny, nz;

    int8_t state = 0;
    int8_t co = 0;

    float dx = 0;
    float dy = 0;
    float dz = 0;

    float threshold = 0;

};

#endif // _ACCELEROMETER_
