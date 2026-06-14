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

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(F("Wifi not connected, skipping replication"));
    return false;
  }

  Serial.println(F("Replicating Server Files"));

  bool ok = getServerFiles();

  if (ok) Serial.println(F("Replicate done"));
  else Serial.println(F("Replicate FAILED"));

  return ok;
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

static bool streamExpectedBodyToFile(WiFiClient& c, size_t expectedLen, File& f, size_t overallStart, size_t overallTotal, uint32_t timeoutMs = 120000)
{
  uint8_t buff[256];

  size_t bytesReceived = 0;
  int prevBucket = -1;

  uint32_t t0 = millis();
  uint32_t lastFlush = millis();

  while (bytesReceived < expectedLen)
  {
    size_t avail = c.available();

    if (!avail)
    {
      if (!c.connected())
      {
        Serial.println(F("streamExpectedBodyToFile: connection closed before complete."));
        Serial.printf("Range received %u of %u bytes\n", (unsigned)bytesReceived, (unsigned)expectedLen);
        return false;
      }

      if (millis() - t0 > timeoutMs)
      {
        Serial.println(F("streamExpectedBodyToFile: timeout waiting for body bytes."));
        Serial.printf("Range received %u of %u bytes\n", (unsigned)bytesReceived, (unsigned)expectedLen);
        return false;
      }

      delay(5);
      yield();
      continue;
    }

    int toRead = (avail > sizeof(buff)) ? (int)sizeof(buff) : (int)avail;
    size_t remaining = expectedLen - bytesReceived;
    if ((size_t)toRead > remaining) toRead = (int)remaining;

    int r = c.readBytes(buff, toRead);

    if (r <= 0)
    {
      delay(5);
      yield();
      continue;
    }

    size_t written = f.write(buff, r);

    if (written != (size_t)r)
    {
      Serial.printf("File write failed: wanted %d wrote %d\n", r, (int)written);
      return false;
    }

    bytesReceived += r;
    t0 = millis();

    if (millis() - lastFlush > 10000)
    {
      f.flush();
      lastFlush = millis();
      Serial.printf("Flush at overall byte %u\n", (unsigned)(overallStart + bytesReceived));
    }

    if (overallTotal > 0)
    {
      int pct = (int)(((overallStart + bytesReceived) * 100ULL) / overallTotal);
      int bucket = pct / 5;

      if (bucket != prevBucket)
      {
        prevBucket = bucket;
        Serial.printf("Download %d%%, %u/%u bytes\n", pct, (unsigned)(overallStart + bytesReceived), (unsigned)overallTotal);
      }
    }

    delay(1);
    yield();
  }

  f.flush();
  return true;
}

static int parseContentRangeTotal(const String& contentRange)
{
  // Expected form: bytes 0-0/12605440
  int slash = contentRange.lastIndexOf('/');
  if (slash < 0) return -1;

  String total = contentRange.substring(slash + 1);
  total.trim();

  if (total == "*") return -1;
  return total.toInt();
}

static int readHttpHeadersForRange(WiFiClient& c, bool& chunked, String& contentRange)
{
  chunked = false;
  contentRange = "";
  int contentLength = -1;

  while (c.connected())
  {
    String h = c.readStringUntil('\n');
    if (h == "\r" || h.length() == 0) break;

    h.trim();

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
    else if (h.startsWith("Content-Range:") || h.startsWith("content-range:"))
    {
      contentRange = h.substring(strlen("Content-Range:"));
      contentRange.trim();
    }
  }

  return contentLength;
}

static int getRemoteFileSizeByHead(const char* thefile)
{
  WiFiClientSecure client;
  client.setCACert(CLOUDCITY_INTERMEDIATE_CA);
  client.setTimeout(120000);
  client.setNoDelay(true);

  if (!client.connect(CLOUD_HOST, CLOUD_PORT))
  {
    Serial.println(F("TLS connect failed (HEAD)."));
    return -1;
  }

  char req[256];
  int n = snprintf(req, sizeof(req),
                   "HEAD /api/download?name=%s HTTP/1.1\r\n"
                   "Host: %s\r\n"
                   "User-Agent: ESP32\r\n"
                   "Accept: */*\r\n"
                   "Connection: close\r\n"
                   "\r\n",
                   thefile, CLOUD_HOST);

  int wn = client.write((const uint8_t*)req, n);
  client.flush();
  Serial.printf("HEAD wrote %d/%d bytes\n", wn, n);

  if (!waitForFirstByte(client, 120000))
  {
    Serial.println(F("No response bytes from server after HEAD request."));
    client.stop();
    return -1;
  }

  int code = readHttpStatusLine(client, 120000);
  Serial.printf("HEAD status=%d\n", code);

  bool chunked = false;
  bool closeConn = false;
  String loc;
  int len = readHttpHeaders(client, chunked, closeConn, loc);

  while (client.available()) client.read();
  client.stop();

  if (code != 200 || chunked || len <= 0)
  {
    Serial.printf("HEAD unsupported: status=%d chunked=%d contentLength=%d\n", code, (int)chunked, len);
    return -1;
  }

  return len;
}

static int getRemoteFileSizeByRangeProbe(const char* thefile)
{
  WiFiClientSecure client;
  client.setCACert(CLOUDCITY_INTERMEDIATE_CA);
  client.setTimeout(120000);
  client.setNoDelay(true);

  if (!client.connect(CLOUD_HOST, CLOUD_PORT))
  {
    Serial.println(F("TLS connect failed (range probe)."));
    return -1;
  }

  char req[320];
  int n = snprintf(req, sizeof(req),
                   "GET /api/download?name=%s HTTP/1.1\r\n"
                   "Host: %s\r\n"
                   "User-Agent: ESP32\r\n"
                   "Accept: */*\r\n"
                   "Range: bytes=0-0\r\n"
                   "Connection: close\r\n"
                   "\r\n",
                   thefile, CLOUD_HOST);

  int wn = client.write((const uint8_t*)req, n);
  client.flush();
  Serial.printf("Range probe wrote %d/%d bytes\n", wn, n);

  if (!waitForFirstByte(client, 120000))
  {
    Serial.println(F("No response bytes from server after range probe."));
    client.stop();
    return -1;
  }

  int code = readHttpStatusLine(client, 120000);
  Serial.printf("Range probe status=%d\n", code);

  bool chunked = false;
  String contentRange;
  int len = readHttpHeadersForRange(client, chunked, contentRange);

  while (client.available()) client.read();
  client.stop();

  if (code != 206 || chunked)
  {
    Serial.printf("Range probe unsupported: status=%d chunked=%d contentLength=%d contentRange=%s\n", code, (int)chunked, len, contentRange.c_str());
    return -1;
  }

  int total = parseContentRangeTotal(contentRange);
  Serial.printf("Range probe total size=%d\n", total);
  return total;
}

static bool downloadRangeToFile(const char* thefile, const String& path, size_t startByte, size_t endByte, size_t totalBytes)
{
  WiFiClientSecure client;
  client.setCACert(CLOUDCITY_INTERMEDIATE_CA);
  client.setTimeout(120000);
  client.setNoDelay(true);

  if (!client.connect(CLOUD_HOST, CLOUD_PORT))
  {
    Serial.println(F("TLS connect failed (range download)."));
    return false;
  }

  char req[360];
  int n = snprintf(req, sizeof(req),
                   "GET /api/download?name=%s HTTP/1.1\r\n"
                   "Host: %s\r\n"
                   "User-Agent: ESP32\r\n"
                   "Accept: */*\r\n"
                   "Range: bytes=%u-%u\r\n"
                   "Connection: close\r\n"
                   "\r\n",
                   thefile, CLOUD_HOST, (unsigned)startByte, (unsigned)endByte);

  Serial.printf("Range request bytes=%u-%u\n", (unsigned)startByte, (unsigned)endByte);

  int wn = client.write((const uint8_t*)req, n);
  client.flush();
  Serial.printf("Range wrote %d/%d bytes\n", wn, n);

  if (!waitForFirstByte(client, 120000))
  {
    Serial.println(F("No response bytes from server after range request."));
    client.stop();
    return false;
  }

  int code = readHttpStatusLine(client, 120000);
  Serial.printf("range status=%d\n", code);

  bool chunked = false;
  String contentRange;
  int contentLength = readHttpHeadersForRange(client, chunked, contentRange);

  if (code != 206 || chunked || contentLength <= 0)
  {
    Serial.printf("range unsupported headers: status=%d chunked=%d contentLength=%d contentRange=%s\n", code, (int)chunked, contentLength, contentRange.c_str());
    client.stop();
    return false;
  }

  size_t expectedLen = endByte - startByte + 1;
  if ((size_t)contentLength != expectedLen)
  {
    Serial.printf("range length mismatch: expected %u got %d contentRange=%s\n", (unsigned)expectedLen, contentLength, contentRange.c_str());
    client.stop();
    return false;
  }

  File f = SD.open(path, startByte == 0 ? FILE_WRITE : FILE_APPEND);
  if (!f)
  {
    Serial.print(F("SD.open failed for "));
    Serial.println(path);
    client.stop();
    return false;
  }

  bool ok = streamExpectedBodyToFile(client, expectedLen, f, startByte, totalBytes, 120000);

  f.close();
  while (client.available()) client.read();
  client.stop();

  return ok;
}

// -------------------------
// getServerFiles(): ranged download
// -------------------------

bool Storage::getServerFiles()
{
  Serial.println(F("getServerFiles(): ranged TLS download starting..."));
  Serial.printf("Heap free=%u largest=%u\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

  const char* thefile = "cat-file-package.tar";
  const size_t chunkSize = 262144;  // 256 KB range requests are safer than one 12+ MB TLS stream.

  int totalLen = getRemoteFileSizeByHead(thefile);
  if (totalLen <= 0)
  {
    Serial.println(F("HEAD did not provide file size; trying range probe."));
    totalLen = getRemoteFileSizeByRangeProbe(thefile);
  }

  if (totalLen <= 0)
  {
    Serial.println(F("Unable to determine remote file size."));
    return false;
  }

  String path = "/";
  path += thefile;

  if (SD.exists(path))
  {
    Serial.print(F("Removing existing "));
    Serial.println(path);
    SD.remove(path);
  }

  Serial.printf("Downloading %d bytes to %s in %u byte ranges\n", totalLen, path.c_str(), (unsigned)chunkSize);

  size_t startByte = 0;
  while (startByte < (size_t)totalLen)
  {
    size_t endByte = startByte + chunkSize - 1;
    if (endByte >= (size_t)totalLen) endByte = (size_t)totalLen - 1;

    if (!downloadRangeToFile(thefile, path, startByte, endByte, (size_t)totalLen))
    {
      Serial.printf("getServerFiles(): FAILED range %u-%u\n", (unsigned)startByte, (unsigned)endByte);
      return false;
    }

    startByte = endByte + 1;
    delay(100);
    yield();
  }

  File downloaded = SD.open(path, FILE_READ);
  if (!downloaded)
  {
    Serial.println(F("Downloaded file missing after range download."));
    return false;
  }

  size_t actualSize = downloaded.size();
  downloaded.close();

  if (actualSize != (size_t)totalLen)
  {
    Serial.printf("Downloaded size mismatch: expected %d got %u\n", totalLen, (unsigned)actualSize);
    return false;
  }

  Serial.printf("Downloaded %u bytes total\n", (unsigned)actualSize);

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

  Serial.println(F("getServerFiles(): OK"));
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