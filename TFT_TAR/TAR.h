#include <rom/rtc.h>

bool isManualReset()
{
  bool ret = false;
  switch ( rtc_get_reset_reason(0) ){
    case 1:  log_d("Reset reason:%s\n", "POWERON_RESET");           ret = true;  break;/**<1, Vbat power on reset*/
    case 3:  log_d("Reset reason:%s\n", "SW_RESET");                ret = false; break;/**<3, Software reset digital core*/
    case 4:  log_d("Reset reason:%s\n", "OWDT_RESET");              ret = false; break;/**<4, Legacy watch dog reset digital core*/
    case 5:  log_d("Reset reason:%s\n", "DEEPSLEEP_RESET");         ret = false; break;/**<5, Deep Sleep reset digital core*/
    case 6:  log_d("Reset reason:%s\n", "SDIO_RESET");              ret = false; break;/**<6, Reset by SLC module, reset digital core*/
    case 7:  log_d("Reset reason:%s\n", "TG0WDT_SYS_RESET");        ret = false; break;/**<7, Timer Group0 Watch dog reset digital core*/
    case 8:  log_d("Reset reason:%s\n", "TG1WDT_SYS_RESET");        ret = false; break;/**<8, Timer Group1 Watch dog reset digital core*/
    case 9:  log_d("Reset reason:%s\n", "RTCWDT_SYS_RESET");        ret = false; break;/**<9, RTC Watch dog Reset digital core*/
    case 10: log_d("Reset reason:%s\n", "INTRUSION_RESET");         ret = false; break;/**<10, Instrusion tested to reset CPU*/
    case 11: log_d("Reset reason:%s\n", "TGWDT_CPU_RESET");         ret = false; break;/**<11, Time Group reset CPU*/
    case 12: log_d("Reset reason:%s\n", "SW_CPU_RESET");            ret = false; break;/**<12, Software reset CPU*/
    case 13: log_d("Reset reason:%s\n", "RTCWDT_CPU_RESET");        ret = false; break;/**<13, RTC Watch dog Reset CPU*/
    case 14: log_d("Reset reason:%s\n", "EXT_CPU_RESET");           ret = false; break;/**<14, for APP CPU, reseted by PRO CPU*/
    case 15: log_d("Reset reason:%s\n", "RTCWDT_BROWN_OUT_RESET");  ret = false; break;/**<15, Reset when the vdd voltage is not stable*/
    case 16: log_d("Reset reason:%s\n", "RTCWDT_RTC_RESET");        ret = false; break;/**<16, RTC Watch dog reset digital core and rtc module*/
    default: log_d("Reset reason:%s\n", "NO_MEAN");
  }

  return ret;
}

struct fileMeta
{
  size_t size;
  const char* md5sum;
  const char* path;
};

struct packageMeta
{
  const char* folder;
  size_t files_count;
  fileMeta *files;
};

fileMeta myFiles[1] = 
{
  { 279200      , "2297aacd9380d9b438490d6002bc83be", "firmware_example_esp32" }
};

packageMeta myPackage = 
{
  nullptr, 15, myFiles
};

char tmp_path[255] = {0};

void myTarMessageCallback(const char* format, ...)
{

  if( myPackage.folder == nullptr ) return;

  char *md5sum;

  va_list args;
  va_start(args, format);
  vsnprintf(tmp_path, 255, format, args);
  va_end(args);

  String filePath;
  int found = -1;
  for( size_t i=0;i<myPackage.files_count;i++ ) {
    if( strcmp( myPackage.files[i].path, tmp_path ) == 0 ) {// exact path check
      found = i;
      break;
    } else {// extended path check
      if( String( myPackage.folder ).endsWith("/") ) {
        filePath = String( myPackage.folder ) + String( myPackage.files[i].path );
      } else {
        filePath = String( myPackage.folder ) + "/" + String( myPackage.files[i].path );
      }
      if( strcmp( filePath.c_str(), tmp_path ) == 0 ) {
        found = i;
        break;
      }
    }
  }
  if( found > -1 ) {
    //delay(100);
    filePath = String( myPackage.folder ) + "/" + String( myPackage.files[found].path );
    if( !tarGzFS.exists( filePath ) ) {
      log_w("[TAR] %-16s MD5 FAIL! File can't be opened\n", tmp_path );
      return;
    }
    fs::File tarFile = tarGzFS.open( filePath, "r" );
    if( !tarGzFS.exists( filePath ) ) {
      log_w("[TAR] %-16s MD5 FAIL! File can't be reached\n", tmp_path );
      return;
    }
    size_t tarFileSize = tarFile.size();
    if( tarFileSize == 0 ) {
      log_w("[TAR] %-16s MD5 FAIL! File is empty\n", tmp_path );
      return;
    }
    md5sum = MD5Sum::fromFile( tarFile );
    tarFile.close();

    if( strcmp( md5sum, myPackage.files[found].md5sum ) == 0 ) {
      Serial.printf("[TAR] %-16s MD5 check SUCCESS!\n", tmp_path );
    } else {
      Serial.printf("[TAR] %-16s MD5 check FAIL! Expected vs Real: [ %s:%d ] / [ %s:%d ]\n", tmp_path, myPackage.files[found].md5sum, myPackage.files[found].size, md5sum, tarFileSize );
      BaseUnpacker::setGeneralError( ESP32_TARGZ_INTEGRITY_FAIL );
    }

  } else {
    Serial.printf("[TAR] %-16s can't be checked\n", tmp_path );
  }

}