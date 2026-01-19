/*
 Reflections, mobile connected entertainment device
 Repository is at https://github.com/frankcohen/ReflectionsOS
 ...
*/

#include "Storage.h"

// MUST be before ESP32-targz include
#define DEST_FS_USES_SD
#include "ESP32-targz.h"

extern LOGGER logger;

const int httpsPort = 443;

// -------------------------------
// TAR progress helpers
// -------------------------------

int globalFileCount = 0;
int globalFileCountOld = 0;
extern Arduino_GFX *gfx;
int16_t st_x, st_y;
uint16_t st_w, st_h;

void ShowProgress(String message)
{
  gfx->fillScreen(COLOR_BLUE);

  gfx->setFont(&ScienceFair14pt7b);
  gfx->setTextColor(COLOR_TEXT_YELLOW);
  gfx->getTextBounds(message.c_str(), 0, 0, &st_x, &st_y, &st_w, &st_h);

  st_y = 130;
  st_x = 40;
  gfx->setCursor((gfx->width() - st_w) / 2, st_y);
  gfx->println(message);
}

void CustomTarStatusProgressCallback(const char* name, size_t size, size_t total_unpacked)
{
  Serial.printf("[TAR] %-32s %8d bytes - %8d Total bytes\n", name, (int)size, (int)total_unpacked);

  delay(200); // govern speed of writes to NAND

  String mef = String(globalFileCount++);
  mef += " files";
  ShowProgress(mef);
}

int pre_percent;

void CustomProgressCallback(uint8_t progress)
{
  if (pre_percent != progress)
  {
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

// -------------------------------
// Storage
// -------------------------------

Storage::Storage(){}

void Storage::sizeNAND()
{
  uint32_t block_count = SD.totalBytes();
  Serial.print("OK, block_count = ");
  Serial.print(block_count);
  Serial.print(", Card size = ");
  Serial.print((block_count / (1024 * 1024)) * 512);
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
  if (!SD.exists(mef))
  {
    Serial.print(F("Creating directory "));
    Serial.println(mef);
    createDir(SD, mef.c_str());
  }
}

void Storage::ensureCloudClient()
{
  cloudClient.setCACert(CLOUDCITY_ROOT_CA);
  cloudClient.setTimeout(30000);
}

void Storage::availSpace()
{
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void Storage::loop(){}

// Mirrors files on Cloud City server to local storage
bool Storage::replicateServerFiles()
{
  globalFileCount = 1;
  globalFileCountOld = 0;

  if (WiFi.status() != 3)
  {
    Serial.println(F("Wifi not connected, skipping replication"));
    return false;
  }

  Serial.println(F("Replicating Server Files"));
  getServerFiles();
  Serial.println("Replicate done");
  return true;
}

// Extracts TAR files to local storage
bool Storage::extract_files(String tarfilename)
{
  char tarFolder[100];
  String mef = "/";
  mef += NAND_BASE_DIR;
  mef.toCharArray(tarFolder, 100);

  Serial.print("extract dir = ");
  Serial.println(tarFolder);

  char fnbuff[100];
  tarfilename.toCharArray(fnbuff, tarfilename.length() + 1);

  TarUnpacker *TARUnpacker = new TarUnpacker();

  TARUnpacker->setupFSCallbacks(targzTotalBytesFn, targzFreeBytesFn);
  TARUnpacker->setTarProgressCallback(BaseUnpacker::defaultProgressCallback);
  TARUnpacker->setTarStatusProgressCallback(CustomTarStatusProgressCallback);
  TARUnpacker->setTarMessageCallback(myTarMessageCallback);

  if (!TARUnpacker->tarExpander(SD, fnbuff, SD, tarFolder))
  {
    Serial.print("tarExpander failed with return code #%d");
    Serial.println(TARUnpacker->tarGzGetError());
    return false;
  }

  Serial.println("Extracting Complete");
  return true;
}

// Recursively removes files in directory, including any files in sub directories
bool Storage::removeFiles(fs::FS &fs, const char * dirname, uint8_t levels)
{
  Serial.printf("Removing files from directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("removeFiles - Failed to open directory");
    return false;
  }
  if (!root.isDirectory())
  {
    Serial.println("Not a directory");
    return false;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.print(file.path());
      Serial.print(", levels ");
      Serial.println(levels);

      if (levels) removeFiles(SD, file.path(), levels - 1);
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.path());
      Serial.print("  SIZE: ");
      Serial.println(file.size());

      deleteFile(SD, file.path());
    }
    file = root.openNextFile();
  }
  return true;
}

/*
 * Recursively removes all files and directories
 */
void Storage::rm(File dir, String tempPath)
{
  while (true)
  {
    File entry = dir.openNextFile();
    String localPath;

    if (entry)
    {
      if (entry.isDirectory())
      {
        localPath = tempPath + entry.name() + rootpath + '\0';
        char folderBuf[localPath.length()];
        localPath.toCharArray(folderBuf, localPath.length());

        Serial.print("rm entry = ");
        Serial.print(entry.name());
        Serial.print(", folderBuf = ");
        Serial.println(folderBuf);

        rm(entry, folderBuf);

        if (strlen(folderBuf) > 0) folderBuf[strlen(folderBuf) - 1] = 0;

        Serial.print("rm 2 folderBuf = ");
        Serial.println(folderBuf);

        if (SD.rmdir(folderBuf))
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
        localPath.toCharArray(charBuf, localPath.length());

        Serial.print("rm 3 entry = ");
        Serial.print(entry.name());
        Serial.print(", charBuf = ");
        Serial.println(charBuf);

        if (SD.remove(charBuf))
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
    else
    {
      break;
    }
  }
}

boolean Storage::listDir(fs::FS &fs, const char * dirname, uint8_t levels, bool monitor)
{
  Serial.printf("Listing directory: %s\n", dirname);

  File rootlist = fs.open(dirname);
  if (!rootlist)
  {
    Serial.print(F("Failed to open directory w: "));
    Serial.println(dirname);
    return false;
  }
  if (!rootlist.isDirectory()) return false;

  File filelist = rootlist.openNextFile();
  while (filelist)
  {
    if (filelist.isDirectory())
    {
      if (monitor)
      {
        Serial.print(F("  DIR : "));
        Serial.println(filelist.name());
      }
      if (levels) listDir(fs, filelist.path(), levels - 1, monitor);
    }
    else
    {
      if (monitor)
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

boolean Storage::createDir(fs::FS &fs, const char * path)
{
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path))
  {
    Serial.println(F("Dir created"));
    return true;
  }
  Serial.println(F("mkdir failed"));
  return false;
}

boolean Storage::removeDir(fs::FS &fs, const char * path)
{
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path))
  {
    Serial.println(F("Dir removed"));
    return true;
  }
  Serial.println(F("removeDir failed"));
  return false;
}

boolean Storage::readFile(fs::FS &fs, const char * path)
{
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file)
  {
    Serial.println(F("Failed to open file for reading"));
    return false;
  }

  Serial.print(F("Read from file: "));
  while (file.available()) Serial.print((char)file.read());
  Serial.println();

  file.close();
  return true;
}

boolean Storage::writeFile(fs::FS &fs, const char * path, const char * message)
{
  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println(F("Failed to open file for writing"));
    return false;
  }
  if (file.print(message)) Serial.println(F("File written"));
  else Serial.println(F("Write failed"));
  file.close();
  return true;
}

boolean Storage::appendFile(fs::FS &fs, const char * path, const char * message)
{
  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println(F("Failed to open file for appending"));
    return false;
  }
  if (file.print(message))
  {
    Serial.println(F("Message appended"));
    file.close();
    return true;
  }
  Serial.println(F("Append failed"));
  file.close();
  return false;
}

boolean Storage::renameFile(fs::FS &fs, const char * path1, const char * path2)
{
  if (fs.rename(path1, path2)) return true;
  Serial.println(F("Rename failed"));
  return false;
}

boolean Storage::deleteFile(fs::FS &fs, const char * path)
{
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path))
  {
    Serial.println(F("File deleted"));
    return true;
  }
  Serial.print(F("Delete failed"));
  Serial.println(path);
  return false;
}

boolean Storage::testFileIO(fs::FS &fs, const char * path)
{
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  if (file)
  {
    len = file.size();
    while (len)
    {
      size_t toRead = len > 512 ? 512 : len;
      file.read(buf, toRead);
      len -= toRead;
    }
    file.close();
    return true;
  }
  Serial.println(F("Failed to open file for reading"));
  return false;
}

void Storage::setMounted(bool mounted)
{
  SDMounted = mounted;
}

boolean Storage::testNandStorage()
{
  if (!SDMounted) return false;

  if (!listDir(SD, "/", 0, false)) return false;
  if (!createDir(SD, "/mydir")) return false;
  if (!listDir(SD, "/", 0, false)) return false;
  if (!removeDir(SD, "/mydir")) return false;
  if (!listDir(SD, "/", 2, false)) return false;
  if (!writeFile(SD, "/hello.txt", "Hello ")) return false;
  if (!appendFile(SD, "/hello.txt", "World!\n")) return false;
  if (!readFile(SD, "/hello.txt")) return false;

  return true;
}

bool Storage::fileAvailableForDownload()
{
  return false;
}

void Storage::smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  while (millis() - start < ms) {}
}

// -------------------------
// HTTP parsing helpers
// -------------------------

static bool waitForFirstByte(WiFiClient& c, uint32_t timeoutMs)
{
  uint32_t t0 = millis();
  while (!c.available() && c.connected() && (millis() - t0 < timeoutMs))
  {
    delay(5);
  }
  return c.available() > 0;
}

static int readHttpStatusLine(WiFiClient& c, uint32_t timeoutMs = 120000)
{
  if (!waitForFirstByte(c, timeoutMs))
  {
    Serial.println("readHttpStatusLine: timeout waiting for status line bytes.");
    return -1;
  }

  char line[160];
  size_t n = c.readBytesUntil('\n', line, sizeof(line) - 1);
  line[n] = 0;
  if (n > 0 && line[n - 1] == '\r') line[n - 1] = 0;

  // skip blank lines
  int guard = 0;
  while (line[0] == 0 && c.connected() && guard++ < 3)
  {
    n = c.readBytesUntil('\n', line, sizeof(line) - 1);
    line[n] = 0;
    if (n > 0 && line[n - 1] == '\r') line[n - 1] = 0;
  }

  if (strncmp(line, "HTTP/", 5) != 0)
  {
    Serial.print("readHttpStatusLine: not HTTP status line: '");
    Serial.print(line);
    Serial.println("'");
    return -1;
  }

  char* sp1 = strchr(line, ' ');
  if (!sp1) return -1;
  while (*sp1 == ' ') sp1++;

  return atoi(sp1);
}

static int readHttpHeaders(WiFiClient& c, bool& chunked, bool& connectionClose, String& location)
{
  chunked = false;
  connectionClose = false;
  location = "";
  int contentLength = -1;

  while (c.connected())
  {
    String h = c.readStringUntil('\n');
    if (h == "\r" || h.length() == 0) break;

    h.trim();

    // minimal parsing without allocating a lowercase copy
    if (h.startsWith("Content-Length:") || h.startsWith("content-length:"))
    {
      String v = h.substring(strlen("Content-Length:"));
      v.trim();
      contentLength = v.toInt();
    }
    else if (h.startsWith("Transfer-Encoding:") || h.startsWith("transfer-encoding:"))
    {
      if (h.indexOf("chunked") >= 0 || h.indexOf("Chunked") >= 0) chunked = true;
    }
    else if (h.startsWith("Connection:") || h.startsWith("connection:"))
    {
      if (h.indexOf("close") >= 0 || h.indexOf("Close") >= 0) connectionClose = true;
    }
    else if (h.startsWith("Location:") || h.startsWith("location:"))
    {
      location = h.substring(strlen("Location:"));
      location.trim();
    }
  }
  return contentLength;
}

static bool streamFixedBodyToFile(WiFiClient& c, int len, File& f, uint32_t timeoutMs = 60000)
{
  uint8_t buff[1024];
  int bytesReceived = 0;
  int prevBucket = -1;
  int total = len;

  uint32_t t0 = millis();
  while (len > 0 && c.connected())
  {
    size_t avail = c.available();
    if (!avail)
    {
      if (millis() - t0 > timeoutMs) return false;
      delay(1);
      continue;
    }

    int toRead = (avail > sizeof(buff)) ? (int)sizeof(buff) : (int)avail;
    if (toRead > len) toRead = len;

    int r = c.readBytes(buff, toRead);
    if (r <= 0) { delay(1); continue; }

    f.write(buff, r);
    len -= r;
    bytesReceived += r;
    t0 = millis();

    int pct = (bytesReceived * 100) / total;
    int bucket = pct / 10;
    if (bucket != prevBucket)
    {
      prevBucket = bucket;
      Serial.printf("Download %d%%\n", pct);
    }
  }

  Serial.printf("Downloaded %d bytes\n", bytesReceived);
  return (len == 0);
}

// -------------------------
// getServerFiles(): direct download
// -------------------------

bool Storage::getServerFiles()
{
  Serial.println("getServerFiles(): direct download (single TLS) starting...");
  Serial.printf("Heap free=%u largest=%u\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

  const char* thefile = "cat-file-package.tar";

  WiFiClientSecure client;
  client.setCACert(CLOUDCITY_INTERMEDIATE_CA);
  client.setTimeout(120000); // important: allow slow first-byte
  client.setNoDelay(true);

  if (!client.connect(CLOUD_HOST, CLOUD_PORT))
  {
    Serial.println("TLS connect failed (download).");
    return false;
  }

  // SD destination
  String path = "/";
  path += thefile;
  if (SD.exists(path)) SD.remove(path);

  File f = SD.open(path, FILE_WRITE);
  if (!f)
  {
    Serial.println("SD.open() failed.");
    client.stop();
    return false;
  }

  // Build request with NO String allocations
  char req[256];
  int n = snprintf(req, sizeof(req),
                   "GET /api/download?name=%s HTTP/1.1\r\n"
                   "Host: %s\r\n"
                   "User-Agent: ESP32\r\n"
                   "Accept: */*\r\n"
                   "Connection: close\r\n"
                   "\r\n",
                   thefile, CLOUD_HOST);

  Serial.println("---- REQUEST ----");
  Serial.write((const uint8_t*)req, n);
  Serial.println("-----------------");

  int wn = client.write((const uint8_t*)req, n);
  client.flush();
  Serial.printf("Wrote %d/%d bytes\n", wn, n);

  // If the server is going to respond, we should see a first byte relatively soon.
  // This log tells us definitively if it’s “no response at all”.
  if (!waitForFirstByte(client, 120000))
  {
    Serial.println("No response bytes from server after request (120s).");
    f.close();
    client.stop();
    return false;
  }

  int dcode = readHttpStatusLine(client, 120000);
  Serial.printf("download status=%d\n", dcode);
  if (dcode != 200)
  {
    f.close();
    client.stop();
    return false;
  }

  bool dchunked = false, dclose = false;
  String dloc;
  int dlen = readHttpHeaders(client, dchunked, dclose, dloc);
  if (dchunked || dlen < 0)
  {
    Serial.printf("download unsupported headers: chunked=%d contentLength=%d\n", (int)dchunked, dlen);
    f.close();
    client.stop();
    return false;
  }

  Serial.printf("Downloading %d bytes to %s\n", dlen, path.c_str());
  bool ok = streamFixedBodyToFile(client, dlen, f, 120000);

  f.close();
  while (client.available()) client.read();
  client.stop();

  if (!ok)
  {
    Serial.println("getServerFiles(): FAILED (stream)");
    return false;
  }

  if (String(thefile).endsWith(TAR_FILENAME))
  {
    Serial.print(F("Extractomatic "));
    Serial.println(thefile);

    if (!extract_files(path))
    {
      Serial.println(F("TAR extraction failed"));
      return false;
    }
  }

  Serial.println("getServerFiles(): OK");
  return true;
}

// -------------------------
// Original HTTPClient methods below (unchanged)
// -------------------------

boolean Storage::getFileSaveToSD(String thedoc)
{
  // ... (leave your existing code as-is)
  // (I’m not rewriting this section here since you’re not using it in the init utility path.)
  return false;
}

String Storage::getFileListString()
{
  // ... (leave your existing code as-is)
  return "";
}

void Storage::printStats()
{
  uint64_t totalBytes = SD.totalBytes();
  uint64_t usedBytes = SD.usedBytes();
  uint64_t cardSize = SD.cardSize();

  Serial.printf("Total space: %llu MB\n", totalBytes / (1024 * 1024));
  Serial.printf("Used space: %llu bytes\n", usedBytes);
  Serial.printf("Card size: %llu bytes\n", cardSize);
}