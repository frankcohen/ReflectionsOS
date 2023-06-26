/*
Reflections, distributed entertainment device

What started as a project to wear videos of my children as they grew up on my
arm as a wristwatch, grew to be a platform for making entertaining experiences.
This is the software component. It runs on an ESP32-based platform with OLED display,
audio player, flash memory, GPS, gesture sensor, and accelerometer/compass

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for storage tasks, including getting media/data from Cloud City server, over Wifi and Bluetooth
get TAR over Wifi, Bluetooth, mostly SD utils like print to, eventually send message to other devices

*/

#ifndef _storage_
#define _storage_

#include "Arduino.h"
#include "HTTPClient.h"
#include "config.h"
#include "ArduinoJson.h"
#include "secrets.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

class Storage
{
  public:
    Storage();

    void begin();
    void loop();

    // Wifi services
    bool fileAvailableForDownload();
    bool findOneFile();
    bool getFileSaveToSD( String thedoc );
    String getFileListString();
    void replicateServerFiles();

    // Utils
    void smartDelay(unsigned long ms);

    // SD file utilities
    boolean listDir(fs::FS &fs, const char * dirname, uint8_t levels, bool monitor);
    boolean createDir(fs::FS &fs, const char * path);
    boolean removeDir(fs::FS &fs, const char * path);
    boolean readFile(fs::FS &fs, const char * path);
    boolean writeFile(fs::FS &fs, const char * path, const char * message);
    boolean appendFile(fs::FS &fs, const char * path, const char * message);
    boolean renameFile(fs::FS &fs, const char * path1, const char * path2);
    boolean deleteFile(fs::FS &fs, const char * path);
    boolean removeFiles(fs::FS &fs, const char * dirname, uint8_t levels);
    void availSpace();
    boolean testFileIO(fs::FS &fs, const char * path);
    boolean testNandStorage();

    void extract_files( String tarfilename );
    void rm( File dir, String tempPath );

    void sizeNAND();    

  private:
    String _fileName;
    boolean SDMounted = false;

    int lvlcnt = 0;
    int DeletedCount = 0;
    int FolderDeleteCount = 0;
    int FailCount = 0;
    String rootpath = "/";

/*
I used this tutorial to use Chrome to get the root certificate
https://randomnerdtutorials.com/esp32-https-requests/

nginx ssl installation instruction at:
https://help.zerossl.com/hc/en-us/articles/360058295894-Installing-SSL-Certificate-on-NGINX

NOTE: You must combine the certificate.crt and ca_bundle.crt files

/etc/ssl/cloudcity/certificate.crt;
/etc/ssl/cloudcity/private.key;

Then do this from your Mac laptop:
openssl s_client -showcerts -servername cloudcity.starlingwatch.com -connect cloudcity.starlingwatch.com:443 </dev/null

Then copy the second certificate:
-----END CERTIFICATE-----
 1 s:/C=AT/O=ZeroSSL/CN=ZeroSSL RSA Domain Secure Site CA
   i:/C=US/ST=New Jersey/L=Jersey City/O=The USERTRUST Network/CN=USERTrust RSA Certification Authority
-----BEGIN CERTIFICATE-----
MIIG1TCCBL2gAwIBAgIQbFWr29AHksedBwzYEZ7WvzANBgkqhkiG9w0BAQwFADCB
iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl

*/
    
const char* root_ca= \
"-----BEGIN CERTIFICATE-----\n" \
"MIIG1TCCBL2gAwIBAgIQbFWr29AHksedBwzYEZ7WvzANBgkqhkiG9w0BAQwFADCB\n" \
"iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n" \
"cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n" \
"BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMjAw\n" \
"MTMwMDAwMDAwWhcNMzAwMTI5MjM1OTU5WjBLMQswCQYDVQQGEwJBVDEQMA4GA1UE\n" \
"ChMHWmVyb1NTTDEqMCgGA1UEAxMhWmVyb1NTTCBSU0EgRG9tYWluIFNlY3VyZSBT\n" \
"aXRlIENBMIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAhmlzfqO1Mdgj\n" \
"4W3dpBPTVBX1AuvcAyG1fl0dUnw/MeueCWzRWTheZ35LVo91kLI3DDVaZKW+TBAs\n" \
"JBjEbYmMwcWSTWYCg5334SF0+ctDAsFxsX+rTDh9kSrG/4mp6OShubLaEIUJiZo4\n" \
"t873TuSd0Wj5DWt3DtpAG8T35l/v+xrN8ub8PSSoX5Vkgw+jWf4KQtNvUFLDq8mF\n" \
"WhUnPL6jHAADXpvs4lTNYwOtx9yQtbpxwSt7QJY1+ICrmRJB6BuKRt/jfDJF9Jsc\n" \
"RQVlHIxQdKAJl7oaVnXgDkqtk2qddd3kCDXd74gv813G91z7CjsGyJ93oJIlNS3U\n" \
"gFbD6V54JMgZ3rSmotYbz98oZxX7MKbtCm1aJ/q+hTv2YK1yMxrnfcieKmOYBbFD\n" \
"hnW5O6RMA703dBK92j6XRN2EttLkQuujZgy+jXRKtaWMIlkNkWJmOiHmErQngHvt\n" \
"iNkIcjJumq1ddFX4iaTI40a6zgvIBtxFeDs2RfcaH73er7ctNUUqgQT5rFgJhMmF\n" \
"x76rQgB5OZUkodb5k2ex7P+Gu4J86bS15094UuYcV09hVeknmTh5Ex9CBKipLS2W\n" \
"2wKBakf+aVYnNCU6S0nASqt2xrZpGC1v7v6DhuepyyJtn3qSV2PoBiU5Sql+aARp\n" \
"wUibQMGm44gjyNDqDlVp+ShLQlUH9x8CAwEAAaOCAXUwggFxMB8GA1UdIwQYMBaA\n" \
"FFN5v1qqK0rPVIDh2JvAnfKyA2bLMB0GA1UdDgQWBBTI2XhootkZaNU9ct5fCj7c\n" \
"tYaGpjAOBgNVHQ8BAf8EBAMCAYYwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNVHSUE\n" \
"FjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwIgYDVR0gBBswGTANBgsrBgEEAbIxAQIC\n" \
"TjAIBgZngQwBAgEwUAYDVR0fBEkwRzBFoEOgQYY/aHR0cDovL2NybC51c2VydHJ1\n" \
"c3QuY29tL1VTRVJUcnVzdFJTQUNlcnRpZmljYXRpb25BdXRob3JpdHkuY3JsMHYG\n" \
"CCsGAQUFBwEBBGowaDA/BggrBgEFBQcwAoYzaHR0cDovL2NydC51c2VydHJ1c3Qu\n" \
"Y29tL1VTRVJUcnVzdFJTQUFkZFRydXN0Q0EuY3J0MCUGCCsGAQUFBzABhhlodHRw\n" \
"Oi8vb2NzcC51c2VydHJ1c3QuY29tMA0GCSqGSIb3DQEBDAUAA4ICAQAVDwoIzQDV\n" \
"ercT0eYqZjBNJ8VNWwVFlQOtZERqn5iWnEVaLZZdzxlbvz2Fx0ExUNuUEgYkIVM4\n" \
"YocKkCQ7hO5noicoq/DrEYH5IuNcuW1I8JJZ9DLuB1fYvIHlZ2JG46iNbVKA3ygA\n" \
"Ez86RvDQlt2C494qqPVItRjrz9YlJEGT0DrttyApq0YLFDzf+Z1pkMhh7c+7fXeJ\n" \
"qmIhfJpduKc8HEQkYQQShen426S3H0JrIAbKcBCiyYFuOhfyvuwVCFDfFvrjADjd\n" \
"4jX1uQXd161IyFRbm89s2Oj5oU1wDYz5sx+hoCuh6lSs+/uPuWomIq3y1GDFNafW\n" \
"+LsHBU16lQo5Q2yh25laQsKRgyPmMpHJ98edm6y2sHUabASmRHxvGiuwwE25aDU0\n" \
"2SAeepyImJ2CzB80YG7WxlynHqNhpE7xfC7PzQlLgmfEHdU+tHFeQazRQnrFkW2W\n" \
"kqRGIq7cKRnyypvjPMkjeiV9lRdAM9fSJvsB3svUuu1coIG1xxI1yegoGM4r5QP4\n" \
"RGIVvYaiI76C0djoSbQ/dkIUUXQuB8AL5jyH34g3BZaaXyvpmnV4ilppMXVAnAYG\n" \
"ON51WhJ6W0xNdNJwzYASZYH+tmCWI+N60Gv2NNMGHwMZ7e9bXgzUCZH5FaBFDGR5\n" \
"S9VWqHB73Q+OyIVvIbKYcSc2w/aSuFKGSA==\n" \
"-----END CERTIFICATE-----\n";
   
};

#endif // _storage_
