/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 This is a small version of the logger, to avoid using memory. See previous
 commits in the repository for the larger version.

 Prior notes from larger version:

 Example log entry:
CALLIOPE-7A,info,Logger started correctly
then the node.js function adds the date
20230808-11:45AM,CALLIOPE-7A,info,Logger started correctly

Implemented the log receiver service in node.js using:
// GET to make a log entry
var timestamp = require('log-timestamp');
router.get( "/logit", (req, res) => {
  console.log( req.query.message );
  res.send( "<html><body>Logged</body></html>" );
}); 

requires timestamp module, install using:
npm install log-timestamp

Example,
https://myserver.com/api/logit?message=thisismyfirstlogentry5

Article and comments at:
https://www.reddit.com/r/esp32/comments/16mj3zh/scalable_logger_esp32_wifi_sdnand_for_multiple/

Issure reports:
https://github.com/espressif/arduino-esp32/issues/9465
The logger fails when it uses SD.remove(“/REFLECTIONS/log1”). It works fine with log1.txt. I opened a bug 
report at: https://github.com/espressif/arduino-esp32/issues/9465. And I changed the logger.cpp code to use 
log1.txt as a file name.

*/

#ifndef _LOGGER_
#define _LOGGER_

#include "config.h"
#include "secrets.h"

#include "Arduino.h"

class LOGGER
{
  public:
    LOGGER();
    void begin();
    void loop();

    void info( String message );
    void warning( String message );
    void error( String message );
    void critical( String message );
    void clearLog( String logname );

    void setEchoToSerial( bool echo );
    void setEchoToServer( bool echo );    // Non-functional in the small version

  private:
    void logit( String msgtype, String msg );
    bool echoSerial;
    bool echoServer;
    
};

#endif // _LOGGER_
