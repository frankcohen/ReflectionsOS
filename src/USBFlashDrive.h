#ifndef _USBFLASHDRIVE_
#define _USBFLASHDRIVE_

#include "config.h"
#include "secrets.h"

#include "SD.h"
#include "SPI.h"

class USBFlashDrive
{
  public:
    USBFlashDrive();
    void begin();
    void loop();
    
  private:
};

#endif // _USBFLASHDRIVE_
