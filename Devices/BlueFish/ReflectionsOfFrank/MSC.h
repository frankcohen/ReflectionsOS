#ifndef _MSC_
#define _MSC_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"
#include "Logger.h"

class MSC
{
  public:
    MSC();
    void begin();
    void loop();
    bool test();

  private:

};

#endif // _MSC_

