/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.

 Create a tar file that Storage will unpack:
 tar -C ${FINAL_PATH} -cf ${finalPathArchive} files

*/

#include "Storage.h"

// This is really stupid, and very important. The following #define
// needs to exist before the #include for the ESP32-targz library
// ALSO, don't move this into the Storage.h or you get a compile error
#define DEST_FS_USES_SD
#include "ESP32-targz.h"

extern const char* root_ca;   // Defined in secrets.h
extern LOGGER logger;   // Defined in ReflectionsOfFrank.ino

const int httpsPort = 443;

/*
* Helper for TarUnpacker
*/

int globalFileCount = 0;    // Globals for Board Initialization Utility
int globalFileCountOld = 0;
extern Arduino_GFX *gfx;
int16_t st_x, st_y;
uint16_t st_w, st_h;

void CustomTarStatusProgressCallback( const char* name, size_t size, size_t total_unpacked )
{
  Serial.printf("[TAR] %-32s %8d bytes - %8d Total bytes\n", name, size, total_unpacked );

  gfx->fillScreen( COLOR_BLUE );

  String text = String( globalFileCount++ );

  /*
  if ( text.length() >= 12 ) 
  {
    // Extract the substring starting from the character at (length - 12) to the end
    text = text.substring( text.length() - 12 );
  } 
  */
  
  gfx->setFont( &ScienceFair14pt7b );
  gfx->setTextColor( COLOR_TEXT_YELLOW );
  gfx->getTextBounds( text.c_str(), 0, 0, &st_x, &st_y, &st_w, &st_h);

  st_y = 100;
  st_x = 40;

  gfx->setCursor( ( gfx->width() - st_w ) / 2, st_y );
  gfx->println( text );
}

/*
* Helper for TarUnpacker
*/

int pre_percent;

void CustomProgressCallback( uint8_t progress ){
  if(pre_percent != progress){
    pre_percent = progress;
    Serial.print("Extracted : ");
    Serial.print(progress);
    Serial.println(" %");
  }
}

char tmp_path[255] = {0};

void myTarMessageCallback(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vsnprintf(tmp_path, 255, format, args);
  va_end(args);
}

Storage::Storage(){}

void Storage::sizeNAND()
{
  uint32_t block_count = SD.totalBytes();
  Serial.print("OK, block_count = " );
  Serial.print( block_count );
  Serial.print( ", Card size = ");
  Serial.print((block_count / (1024*1024)) * 512);
  Serial.println(" MB");
    
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

void Storage::begin()
{   
  lvlcnt = 0;
  DeletedCount = 0;
  FolderDeleteCount = 0;
  FailCount = 0;
  rootpath = "/";

  String mef = "/";
  mef += NAND_BASE_DIR;
  if ( ! SD.exists( mef ) )
  {
    Serial.print( F( "Creating directory " ) );
    Serial.println( mef );
    createDir( SD, mef.c_str() );
  }
}

void Storage::availSpace()
{
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void Storage::loop()
{
}

/* Mirrors files on Cloud City server to local storage */

void Storage::replicateServerFiles()
{  
  String thefile;
  String arcfile = "";

  globalFileCount = 1;
  globalFileCountOld = 0;

  if ( WiFi.status() != 3 )
  {
    Serial.println( F( "Wifi not connected, skipping replication") );
    return;
  }

  Serial.println( F( "Replicating Server Files") );
    
  //Serial.println( getFileListString() );

  /* Fixme later: JSON data size limited */
  DynamicJsonDocument doc( 1000 );

  DeserializationError error = deserializeJson(doc, getFileListString() );
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    smartDelay(500);
    return;
  }

  JsonObject fileseq = doc.as<JsonObject>();
  serializeJsonPretty(fileseq, Serial);

  for (JsonObject::iterator it = fileseq.begin(); it!=fileseq.end(); ++it)
  {
    String itkey = it->key().c_str();
    String itvalue = it->value().as<const char*>();

    Serial.println( itkey );

    JsonObject group = doc[ itkey ];

    String thesize = "";
    long lngsize = 0;

    for (JsonObject::iterator groupit = group.begin(); groupit!=group.end(); ++groupit)
    {
      String groupkey = groupit->key().c_str();
      String groupvalue = groupit->value().as<const char*>();
      
      if ( groupkey.equals( "file" ) ) { arcfile = groupvalue; }

      if ( groupkey.equals( "size" ) )
      {
        thesize = groupvalue;
        lngsize = thesize.toInt();
      }
    }

    thefile = "/";
    thefile += arcfile;

    Serial.print( "getFileSaveToSD ");
    Serial.println( thefile );

    if ( ! getFileSaveToSD( arcfile ) )
    {
      Serial.println( "getFileSaveToSD failed" );
      return;
    }

    // If it is a tar, then unpack it

    if ( thefile.endsWith( TAR_FILENAME ) )
    {
      Serial.print( "Extractomatic " );
      Serial.println( thefile );

      extract_files( thefile );
    }
  }

  Serial.println("Replicate done");  
}

/*
* Extracts TAR files to local storage
*/

void Storage::extract_files( String tarfilename )
{
  char tarFolder[100];
  String mef = "/";
  //mef += NAND_BASE_DIR;
  //mef += "/";
  mef += tarfilename.substring( mef.length(), tarfilename.length() - 4 );
  mef.toCharArray( tarFolder, 100 );

  Serial.print( "extract dir = ");
  Serial.println( tarFolder );

  char fnbuff[100];
  tarfilename.toCharArray( fnbuff, tarfilename.length() + 1 );

  TarUnpacker *TARUnpacker = new TarUnpacker();

  TARUnpacker->setupFSCallbacks( targzTotalBytesFn, targzFreeBytesFn ); // prevent the partition from exploding, recommended
  TARUnpacker->setTarProgressCallback( BaseUnpacker::defaultProgressCallback ); // prints the untarring progress for each individual file
  TARUnpacker->setTarStatusProgressCallback( CustomTarStatusProgressCallback ); // print the filenames as they're expanded
  TARUnpacker->setTarMessageCallback( myTarMessageCallback /*BaseUnpacker::targzPrintLoggerCallback*/ ); // tar log verbosity

  if( !TARUnpacker -> tarExpander( SD, fnbuff, SD, tarFolder ) )
  {
     Serial.print("tarExpander failed with return code #%d");
     Serial.println( TARUnpacker->tarGzGetError() );
  }

  Serial.println("Extracting Complete");
}

// Recursively removes files in directory, including any files in sub directories

boolean Storage::removeFiles(fs::FS &fs, const char * dirname, uint8_t levels)
{
  Serial.printf("Removing files from directory: %s\n", dirname);

  File root = fs.open(dirname);
  if(!root){
      Serial.println("removeFiles - Failed to open directory");
      return false;
  }
  if(!root.isDirectory()){
      Serial.println("Not a directory");
      return false;
  }

  File file = root.openNextFile();
  while(file)
  {
    if( file.isDirectory() )
    {
      Serial.print("  DIR : ");
      Serial.print(file.path());
      Serial.print(", levels ");
      Serial.println( levels );

      if(levels)
      {
        removeFiles(SD, file.path(), levels - 1);
      }
    } 
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.path());
      Serial.print("  SIZE: ");
      Serial.println(file.size());

      deleteFile(SD, file.path() );
    }
    file = root.openNextFile();
  }
  return true;
}

/*
 * Recursively removes all files and directories
 * Thank you to @jenschr https://gist.github.com/jenschr/5713c927c3fb8663d662
 */

void Storage::rm( File dir, String tempPath )
{
  while(true) 
  {
    File entry =  dir.openNextFile();
    String localPath;

    if (entry) {
      if ( entry.isDirectory() )
      {
        localPath = tempPath + entry.name() + rootpath + '\0';
        char folderBuf[localPath.length()];
        localPath.toCharArray(folderBuf, localPath.length() );

        Serial.print( "rm entry = ");
        Serial.print( entry.name() );
        Serial.print( ", folderBuf = ");
        Serial.println( folderBuf );

        rm(entry, folderBuf);

        if ( strlen( folderBuf ) > 0 )
        {
          folderBuf[ strlen( folderBuf ) - 1] = 0;  // move null-terminator in
        }

        Serial.print( "rm 2 folderBuf = ");
        Serial.println( folderBuf );

        if( SD.rmdir( folderBuf ) )
        {
          Serial.print("rm deleted folder ");
          Serial.println(folderBuf);
          FolderDeleteCount++;
        } 
        else
        {
          Serial.print("rm unable to delete folder ");
          Serial.println(folderBuf);
          FailCount++;
        }
      } 
      else
      {
        localPath = tempPath + entry.name() + '\0';
        char charBuf[localPath.length()];
        localPath.toCharArray(charBuf, localPath.length() );

        Serial.print( "rm 3 entry = ");
        Serial.print( entry.name() );
        Serial.print( ", charBuf = ");
        Serial.println( charBuf );

        if( SD.remove( charBuf ) )
        {
          Serial.print("rm deleted ");
          Serial.println(localPath);
          DeletedCount++;
        } 
        else
        {
          Serial.print("rm failed to delete ");
          Serial.println(localPath);
          FailCount++;
        }

      }
    } 
    else {
      // break out of recursion
      break;
    }
  }
}

boolean Storage::listDir(fs::FS &fs, const char * dirname, uint8_t levels, bool monitor){
    Serial.printf("Listing directory: %s\n", dirname);

    File rootlist = fs.open(dirname);
    if( ! rootlist )
    {
        Serial.print( F( "Failed to open directory w: " ) );
        Serial.println( dirname );
        return false;
    }
    if( !rootlist.isDirectory() )
    {
      return false;
    }

    File filelist = rootlist.openNextFile();
    while ( filelist )
    {
        if ( filelist.isDirectory() )
        {
          if ( monitor )
          {
            Serial.print( F( "  DIR : " ) );
            Serial.println( filelist.name() );
          }
          if( levels )
          {
            listDir( fs, filelist.path(), levels - 1, monitor );
          }
        } 
        else
        {
          if ( monitor )
          {
            Serial.print("  FILE: ");
            Serial.print(filelist.name());
            Serial.print("  SIZE: ");
            Serial.println(filelist.size());
          }
        }
        filelist = rootlist.openNextFile();
    }

    rootlist.close();
    return true;
}

boolean Storage::createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println( F( "Dir created" ) );
        return true;
    } else {
        Serial.println( F( "mkdir failed" ) );
        return false;
    }
}

boolean Storage::removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if( fs.rmdir(path) )
    {
      Serial.println( F( "Dir removed" ) );
      return true;
    }
    else
    {
      Serial.println( F( "removeDir failed" ) );
      return false;
    }
}

boolean Storage::readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println( F( "Failed to open file for reading" ) );
        return false;
    }

    Serial.print( F( "Read from file: " ) );
    while( file.available() )
    {
      Serial.print( file.read() );
    }
    
    Serial.println(" ");

    file.close();
    return true;
}

boolean Storage::writeFile(fs::FS &fs, const char * path, const char * message){
    //Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println( F( "Failed to open file for writing" ) );
        return false;
    }
    if(file.print(message)){
        Serial.println( F( "File written" ) );
    } else {
        Serial.println( F( "Write failed" ) );
    }
    file.close();
    return true;
}

boolean Storage::appendFile(fs::FS &fs, const char * path, const char * message){
    //Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println( F( "Failed to open file for appending" ) );
        return false;
    }
    if(file.print(message)){
        Serial.println( F( "Message appended" ) );
        return true;
    } else {
        Serial.println( F( "Append failed" ) );
        return false;
    }
    file.close();
    return true;
}

boolean Storage::renameFile(fs::FS &fs, const char * path1, const char * path2){
    //Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        //Serial.println( F( "File renamed" ) );
        return true;
    } else {
        Serial.println( F( "Rename failed" ) );
        return false;
    }
}

boolean Storage::deleteFile(fs::FS &fs, const char * path)
{
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println( F( "File deleted" ) );
        return true;
    } else {
        Serial.print( F( "Delete failed" ) );
        Serial.println( path );
        return false;
    }
}

boolean Storage::testFileIO(fs::FS &fs, const char * path)
{
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        //Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
        return true;
    } else {
        Serial.println( F( "Failed to open file for reading" ) );
        return false;
    }

    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println( F( "Failed to open file for writing" ) );
        return false;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    //Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
    return true;
}

void Storage::setMounted( bool mounted )
{
  SDMounted = mounted;
}

/*
 * Unit test for NAND storage
 */

boolean Storage::testNandStorage()
{
  if ( ! SDMounted )
  {
    return false;
  }

  //uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  //Serial.printf("SD Card Size: %lluMB\n", cardSize);

  if ( ! listDir(SD, "/", 0, false) )
  {
    Serial.println( F( "listDir failed 0" ) );
    return false;
  }
  if ( ! createDir(SD, "/mydir") ) 
  {
    Serial.println( F( "createDir failed" ) );
    return false;
  }
  if ( ! listDir(SD, "/", 0, false) )
  {
    Serial.println( F( "listDir failed 2" ) );
    return false;
  }
  if ( ! removeDir(SD, "/mydir") )
  {
    Serial.println( F( "removeDir failed" ) );
    return false;
  }
  if ( ! listDir(SD, "/", 2, false) )
  {
    Serial.println( F( "listDir failed 3" ) );
    return false;
  }
  if ( ! writeFile(SD, "/hello.txt", "Hello ") )
  {
    Serial.println( F( "writeFile failed" ) );
    return false;
  }
  if ( ! appendFile(SD, "/hello.txt", "World!\n") )
  {
    Serial.println( F( "appendFile failed" ) );
    return false;
  }
  if ( ! readFile(SD, "/hello.txt") )
  {
    Serial.println( F( "readFile failed" ) );
    return false;
  }
  /*
  if ( ! deleteFile(SD, "/foo.txt") )
  {
    Serial.println( F( "deleteFile failed" ) );
    return false;
  }

  if ( ! renameFile(SD, "/hello.txt", "/foo.txt") )
  {
    Serial.println( F( "renameFile failed" ) );
    return false;
  }

  if ( ! readFile(SD, "/foo.txt") )
  {
    Serial.println( F( "readFile failed" ) );
    return false;
  }
  */

  //Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  //Serial.printf("Used space: %llu\n", SD.usedBytes() );
  //Serial.printf("Card size: %llu\n", SD.cardSize() );

  return true;
}

bool Storage::fileAvailableForDownload()
{
  // get the file list
  // for files not represented on SD, compare the file size, when different download
  return false;
}

void Storage::smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // feel free to do something here
  } while (millis() - start < ms);
}

boolean Storage::getFileSaveToSD( String thedoc )
{
        HTTPClient http;

        String ccurl = cloudCityURL;
        ccurl += "api/download?name=";
        ccurl += thedoc;

        Serial.print( F( " ccurl = " ) );
        Serial.println( ccurl );
        
        http.begin( ccurl, root_ca );

        int httpCode = http.GET();

        if( httpCode != HTTP_CODE_OK )
        {
          Serial.print( F( "Server response code: " ) );
          Serial.println( httpCode );
          return false;
        }

        // get length of document (is -1 when Server sends no Content-Length header)
        int len = http.getSize();
        Serial.print( "Size: " );
        Serial.println( len );

        // create buffer for read
        uint8_t buff[130] = { 0 };

        // get tcp stream
        WiFiClient * stream = http.getStreamPtr();

        String mef = "/";
        //mef += NAND_BASE_DIR;
        //mef += "/";
        mef += thedoc;

        File myFile = SD.open( mef, FILE_WRITE );
        if ( myFile )
        {
          Serial.print( F( "Write: " ) );
          Serial.println( mef );
        }
        else
        {
          Serial.print( F( "Error opening new file for writing: " ) );
          Serial.println( mef );
          return false;
        }

        int startTime = millis();
        int bytesReceived = 0;
        boolean buffirst = true;

        // read data from server
        while( http.connected() && (len > 0 || len == -1))
        {
          size_t size = stream->available();
          if(size)
          {
              int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size) );
              bytesReceived += c;

              // Use in case of endian problem
              /*
              byte swapper = 0;
              for ( int n = 0; n<130; n = n+2 )
              {
                swapper = buff[n];
                buff[n] = buff[n+1];
                buff[n+1] = swapper;
              }
              */
              myFile.write( buff, c );
              if(len > 0)
              {
                      len -= c;
              }
          }
          smartDelay(1);
        }

        myFile.close();
        http.end();

        Serial.print( F( "Bytes received " ) );
        Serial.print( bytesReceived );
        Serial.print( F( " in " ) );
        Serial.print( ( millis() - startTime ) / 1000 );
        Serial.print( F( " seconds " ) );
        if ( ( ( millis() - startTime ) / 1000 ) > 0 )
        {
                Serial.print( bytesReceived / ( ( millis() - startTime ) / 1000 ) );
                Serial.print( F( " bytes/second" ) );
        }
        Serial.println( F( " " ) );

        return true;
}

/*
  Client to Cloud City service on server
  https://cloudcity.starlingwatch.com/api/listfiles
  Responds with JSON encoded list of files and sizes
  SSL public key is in config.h
*/

String Storage::getFileListString()
{
  WiFiClientSecure client;
  String flresponse = "";
  int bytesreceived = 0;
  int beginval = 0;
  int startTime = millis();
  String response;

  client.setCACert( root_ca );

  if ( client.connect( cloudCityHostURL, httpsPort) )
  {
    client.print(String("GET ") + "/api/listfiles" + " HTTP/1.1\r\n" +
                "Host: " + cloudCityHostURL + "\r\n" +
                "User-Agent: ESP32\r\n" +
                "Connection: close\r\n\r\n");

    String resline;
    
    while (client.connected()) 
    {
      resline = client.readStringUntil('\n');
      if (resline == "\r") 
      {
        Serial.println("Headers received");
        break;
      }
    }

    while (client.available()) {
      response += client.readStringUntil('\n');
    }

    Serial.print( "Response = " );
    Serial.println( response );

    bytesreceived = response.length();

    /*
    Serial.println( F( "getFileListString" ) );
    Serial.print( F( " cloudCityListFiles = " ) );
    Serial.println( cloudCityListFiles );
    */

  }
  else
  {
    Serial.print( F( "getFileListString, No contact with server" ) );
    Serial.println( beginval );
    Serial.print( F( "cloudCityListFiles " ) );
    Serial.println( cloudCityListFiles );
    return "";
  }

  Serial.print( F( "Bytes received " ) );
  Serial.print( bytesreceived );
  Serial.print( F( " in " ) );
  Serial.print( ( millis() - startTime ) / 1000 );
  Serial.print( F( " seconds " ) );
  if ( ( ( millis() - startTime ) / 1000 ) > 0 )
  {
    Serial.print( bytesreceived / ( ( millis() - startTime ) / 1000 ) );
    Serial.print( F( " bytes/second" ) );
  }
  Serial.println( " " );

  Serial.print( "Response = " );
  Serial.println( response );
  
  return response;
}
