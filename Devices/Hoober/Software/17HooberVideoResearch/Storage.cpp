#include "Storage.h"

// This is really stupid, yet very important. The following #define
// needs to exist before the #include for the ESP32-targz library
// ALSO, don't move this into the Storage.h or you get a compile error
#define DEST_FS_USES_SD
#include "ESP32-targz.h"

/*
* Helper for TarUnpacker
*/

void CustomTarStatusProgressCallback( const char* name, size_t size, size_t total_unpacked ){
  Serial.printf("[TAR] %-32s %8d bytes - %8d Total bytes\n", name, size, total_unpacked );
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

void Storage::begin()
{  
  //replicateServerFiles();
}

void Storage::availSpace()
{
  Serial.printf("Total storage space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used storage space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

void Storage::loop()
{
}

/* Mirrors files on Cloud City server to local storage */

void Storage::replicateServerFiles()
{
  Serial.println("Replicating Server Files");

  //listDir(SD, "/", 100);

  /* Fixme later: JSON data size limited */
  DynamicJsonDocument doc(500);

  //Serial.println( "getFileListString" );
  //Serial.println( getFileListString() );

  DeserializationError error = deserializeJson(doc, getFileListString() );
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    delay(15*1000);
    smartDelay(500);
    return;
  }

  JsonObject fileseq = doc["Reflections"];
  //serializeJsonPretty(fileseq, Serial);

  for (JsonObject::iterator it = fileseq.begin(); it!=fileseq.end(); ++it)
  {
    String itkey = it->key().c_str();
    String itvalue = it->value().as<char*>();

    //Serial.println( itkey );

    JsonObject group = doc["Reflections"][ itkey ];

    String thefile = "";
    String thesize = "";
    long lngsize = 0;

    for (JsonObject::iterator groupit = group.begin(); groupit!=group.end(); ++groupit)
    {
      String groupkey = groupit->key().c_str();
      String groupvalue = groupit->value().as<char*>();

      /*
      Serial.print( "groupkey " );
      Serial.print( groupkey );
      Serial.print( " groupvalue " );
      Serial.println( groupvalue );
      */

      if ( groupkey.equals( "file" ) ) { thefile = groupvalue; }

      if ( groupkey.equals( "size" ) )
      {
        thesize = groupvalue;
        lngsize = thesize.toInt();
      }
    }

    // Is there a file already downloaded?

    //Serial.print("tests ");
    //Serial.println( thefile );

    if ( SD.exists( "/" + thefile ) )
    {
      //Serial.print("exists ");
      //Serial.println( "/" + thefile );

      // Skip if it is the same file size

      File myf = SD.open( "/" + thefile );
      if ( ! myf )
      {
        Serial.print( "Storage replicateServerFiles: myf did not open");
      }

      /*
      Serial.print("size = ");
      Serial.println( myf.size() );
      Serial.print("lngsize = ");
      Serial.println( lngsize );
      */
      
      if ( (long) myf.size() == (long) lngsize )
      {
        Serial.print( "Skipping existing file " );
        Serial.println( thefile );
      }
      else
      {
        // Download the file

        if ( ! getFileSaveToSD( "/" + thefile ) ) return;

        // If it is a tar, then unpack it too

        if ( thefile.endsWith( TAR_FILENAME ) )
        {
          Serial.print( "Extractomatic " );
          Serial.print( "/" + thefile );
          extract_files( "/" + thefile );
        }
      }
    }
    else
    {
      Serial.print( "getFileSaveToSD ");
      Serial.println( thefile );

      getFileSaveToSD( thefile );

      // If it is a tar, then unpack it

      if ( thefile.endsWith( TAR_FILENAME ) )
      {
        Serial.print( "Extractomatic2 " );
        Serial.print( "/" + thefile );

        extract_files( "/" + thefile );
      }

    }
  }

  Serial.println("Replicate done");
  
  //listDir(SD, "/", 100);
  //Serial.println("+++");
}

/*
* Extracts TAR files to local storage
*/

void Storage::extract_files( String tarfilename )
{
  Serial.println("Extracting TAR");

  char tarFolder[100];
  String mydir = "/" + tarfilename.substring( 0, tarfilename.length()-4 );
  mydir.toCharArray( tarFolder, tarfilename.length() );

  Serial.print( "extract dir = ");
  Serial.print( tarFolder );

  //tarfilename.toCharArray( tarFolder, tarfilename.length() - 3 );

  char fnbuff[100];
  tarfilename.toCharArray( fnbuff, tarfilename.length() + 1 );

  TarUnpacker *TARUnpacker = new TarUnpacker();

  TARUnpacker->setupFSCallbacks( targzTotalBytesFn, targzFreeBytesFn ); // prevent the partition from exploding, recommended
  TARUnpacker->setTarProgressCallback( BaseUnpacker::defaultProgressCallback ); // prints the untarring progress for each individual file
  TARUnpacker->setTarStatusProgressCallback( BaseUnpacker::defaultTarStatusProgressCallback ); // print the filenames as they're expanded
  TARUnpacker->setTarMessageCallback( myTarMessageCallback /*BaseUnpacker::targzPrintLoggerCallback*/ ); // tar log verbosity

  if(  !TARUnpacker->tarExpander(SD, fnbuff, SD, tarFolder ) )
  {
     Serial.print("tarExpander failed with return code #%d");
     Serial.println( TARUnpacker->tarGzGetError() );
  }

  Serial.println("Extracting Complete");
}

/*
Lists contents of the SD card to the Serial monitor
*/

void Storage::listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.path());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        }
        else 
        {
            Serial.print("  FILE: ");
            Serial.print( file.path() );
            Serial.print("  SIZE: ");
            Serial.println( file.size() );
        }
        file = root.openNextFile();
    }
}

void Storage::createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void Storage::removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void Storage::readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void Storage::writeFile(fs::FS &fs, const char * path, const char * message)
{
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void Storage::appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void Storage::renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void Storage::deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

// Removes files in directory, including any files in sub directories

void Storage::removeFiles(fs::FS &fs, const char * dirname, uint8_t levels)
{
    Serial.printf("Removing files from directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file)
    {
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.print(file.path());
            Serial.print(", levels ");
            Serial.println( levels );

            if(levels){
              removeFiles(SD, file.path(), levels - 1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.path());
            Serial.print("  SIZE: ");
            Serial.println(file.size());

            deleteFile(SD, file.path() );
        }
        file = root.openNextFile();
    }
}

void Storage::removeDirectories(fs::FS &fs, const char * dirname, uint8_t levels)
{
    Serial.printf("Removing directories from directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file)
    {
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.print(file.name());
            Serial.print(", levels ");
            Serial.println( levels );

            if(levels){
              removeDirectories(SD, file.name(), levels - 1);
              removeDir( SD, file.path() );
            }
        }
        file = root.openNextFile();
    }
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


bool Storage::findOneFile()
{
        Serial.println( "findOneFile" );

        HTTPClient http;

        http.begin(cloudCityURL, cloudCityPort, "/onefilename");

        int httpCode = http.GET();
        if( httpCode != HTTP_CODE_OK )
        {
                //Serial.print( "Server response code: " );
                //Serial.println( httpCode );
                return false;
        }

        // get length of document (is -1 when Server sends no Content-Length header)
        int len = http.getSize();
        // Serial.print( "Size: " );
        // Serial.println( len );

        _fileName = http.getString();
        if ( !_fileName.equals( "nofiles" ) )
        {
                Serial.print( "fileName: " );
                Serial.println( _fileName );
        }

        http.end();
        return true;
}

boolean Storage::getFileSaveToSD( String thedoc )
{
        HTTPClient http;

        http.begin( cloudCityURL, cloudCityPort, "/files/" + thedoc );

        int httpCode = http.GET();
        if( httpCode != HTTP_CODE_OK )
        {
                Serial.print( "Server response code: " );
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

        File myFile = SD.open( "/" + thedoc, "wb" );
        if ( myFile )
        {
                Serial.print( "SD file opened for write: " );
                Serial.println( thedoc );
        }
        else
        {
                Serial.print( "Error opening new file for writing: " );
                Serial.println( thedoc );
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

        Serial.print( "Bytes received " );
        Serial.print( bytesReceived );
        Serial.print( " in " );
        Serial.print( ( millis() - startTime ) / 1000 );
        Serial.print( " seconds " );
        if ( ( ( millis() - startTime ) / 1000 ) > 0 )
        {
                Serial.print( bytesReceived / ( ( millis() - startTime ) / 1000 ) );
                Serial.print( " bytes/second" );
        }
        Serial.println( " " );

        return true;
}

/*
  Client to CloudCity.py service on server
  http://35.163.96.119:8088/listfiles
  Responds with JSON encoded list of files and sizes
*/

String Storage::getFileListString()
{
        HTTPClient http;
        String flresponse = "";

        int counter = 0;
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
          counter++;
          if(counter>=60){
            Serial.println(" time out" );
            break;
          }
        }

        int beginval = 0;
        beginval = http.begin( cloudCityURL, cloudCityPort, "/listfiles" );
        if ( !beginval )
        {
          Serial.print( "getFileListString, Server failed to begin. " );
          Serial.println( beginval );
          Serial.print( "cloudCityURL " );
          Serial.println( cloudCityURL );
          Serial.print( "cloudCityPort " );
          Serial.println( cloudCityPort );
          return "";
        }

        delay(2000);

        int httpCode = http.GET();
        if( httpCode != HTTP_CODE_OK )
        {
          Serial.print( "Server response code: " );
          Serial.println( httpCode );
          return "";
        }

        // get length of document (is -1 when Server sends no Content-Length header)
        int len = http.getSize();
        //Serial.print( "Size: " );
        //Serial.println( len );

        /* Fixme later: Maximum download size is set to 2000 */

        // create buffer for read
        char buff[2000] = { 0 };

        // get tcp stream
        WiFiClient * stream = http.getStreamPtr();

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
              buff[ bytesReceived ] = 0;

              String rc( buff );
              flresponse = flresponse + rc;

              if(len > 0)
              {
                len -= c;
              }
          }
          smartDelay(1);
        }

        http.end();

        Serial.print( "Bytes received " );
        Serial.print( bytesReceived );
        Serial.print( " in " );
        Serial.print( ( millis() - startTime ) / 1000 );
        Serial.print( " seconds " );
        if ( ( ( millis() - startTime ) / 1000 ) > 0 )
        {
                Serial.print( bytesReceived / ( ( millis() - startTime ) / 1000 ) );
                Serial.print( " bytes/second" );
        }
        Serial.println( " " );

        return flresponse;
}
