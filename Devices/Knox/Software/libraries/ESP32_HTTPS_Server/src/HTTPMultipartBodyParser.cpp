#include "HTTPMultipartBodyParser.hpp"
#include <sstream>

const size_t MAXLINESIZE = 256;

namespace httpsserver {

HTTPMultipartBodyParser::HTTPMultipartBodyParser(HTTPRequest * req):
  HTTPBodyParser(req),
  peekBuffer(NULL),
  peekBufferSize(0),
  boundary(""),
  lastBoundary(""),
  fieldName(""),
  fieldMimeType(""),
  fieldFilename("")
{
  auto contentType = _request->getHeader("Content-Type");
  auto contentLength = _request->getHeader("Content-Length");
// frankolo
// #ifdef DEBUG_MULTIPART_PARSER
  Serial.print("Content type: ");
  Serial.println(contentType.c_str());

  Serial.print( "Content-Length: ");
  Serial.println( contentLength.c_str()  );

// #endif
  auto boundaryIndex = contentType.find("boundary=");
  if(boundaryIndex == std::string::npos) {
    HTTPS_LOGE("Multipart: missing boundary=");
    discardBody();
    return;
  }
  boundary = contentType.substr(boundaryIndex + 9); // "boundary="
  auto commaIndex = boundary.find(';');



  // frankolo
  // why
  // why
  // why
  // why

  boundary = "--" + boundary.substr(0, commaIndex);
  if(boundary.size() > 72) {
    HTTPS_LOGE("Multipart: boundary string too long");
    discardBody();
  }
  lastBoundary = boundary + "--";
}

HTTPMultipartBodyParser::~HTTPMultipartBodyParser() {
  if (peekBuffer) {
    free(peekBuffer);
    peekBuffer = NULL;
  }
}

void HTTPMultipartBodyParser::discardBody() {
  if (peekBuffer) {
    free(peekBuffer);
  }
  peekBuffer = NULL;
  peekBufferSize = 0;
  _request->discardRequestBody();
}

bool HTTPMultipartBodyParser::endOfBody() {

  /*
  Serial.print( "endOfBody " );
  Serial.print( peekBufferSize );
  Serial.print( " " );
  if ( _request->requestComplete() )
  {
    Serial.println( "true" );
  }
  else
  {
    Serial.println( "false" );
  }
  */

  return peekBufferSize == 0  && _request->requestComplete();
}

void HTTPMultipartBodyParser::fillBuffer(size_t maxLen) {
  // Fill the buffer with up to maxLen bytes (total length, including
  // what was already in the buffer), but stop reading ahead once
  // we have a CR in the buffer (because the upper layers will
  // stop consuming there anyway, to forestall overrunning
  // a boundary)

  char *bufPtr;
  if (peekBuffer == NULL) {
    // Nothing in the buffer. Allocate one of the wanted size
    peekBuffer = (char *)malloc(maxLen);
    if (peekBuffer == NULL) {
      HTTPS_LOGE("Multipart: out of memory");
      discardBody();
      return;
    }
    bufPtr = peekBuffer;
    peekBufferSize = 0;

    //Serial.println( "fillBuffer 111");

  } else if (peekBufferSize < maxLen) {
    // Something in the buffer, but not enough
/*
    Serial.print( "add to buffer ");
    Serial.print( maxLen );
    Serial.print( " peekBufferSize ");
    Serial.println( peekBufferSize );
*/

    //Serial.println( "fillBuffer 222");

    char *newPeekBuffer = (char *)realloc(peekBuffer, maxLen);
    if (newPeekBuffer == NULL) {
      HTTPS_LOGE("Multipart: out of memory");
      discardBody();
      return;
    }

    peekBuffer = newPeekBuffer;
    bufPtr = peekBuffer + peekBufferSize;

  } else {

    //Serial.println( "fillBuffer 333");

    // We already have enough data in the buffer.
    return;
  }

  //Serial.println( "fillBuffer 444");

  while(bufPtr < peekBuffer+maxLen) {
    size_t didRead = _request->readChars(bufPtr, peekBuffer+maxLen-bufPtr);
    if (didRead == 0) {
      //Serial.println( "fillBuffer 555");
      break;
    }

    /*
    Serial.println( "=" );
    for ( int z = 0; z<240; z++ )
    {
      Serial.print( bufPtr[z] );
      Serial.print( "=" );
      Serial.print( ( (byte *) bufPtr)[z], HEX );
      Serial.print( ", " );

    };
    Serial.println( "+" );
    */

    bufPtr += didRead;
    // We stop buffering once we have a CR in the buffer
    if (memchr(peekBuffer, '\r', bufPtr-peekBuffer) != NULL) {

      Serial.println( "fillBuffer 666");
      break;
    }
  }
  peekBufferSize = bufPtr - peekBuffer;

  //Serial.print( "fillBuffer 777");
  //Serial.print( peekBufferSize );

  if (peekBufferSize == 0) {
    HTTPS_LOGE("Multipart incomplete");
  }
}

void HTTPMultipartBodyParser::consumedBuffer(size_t consumed) {
/*
Serial.print( "consumedBuffer() " );
Serial.print( consumed );
Serial.print( " peekBufferSize ");
Serial.println( peekBufferSize );
*/
  if (consumed == 0) {
    return;
  }
  if (consumed == peekBufferSize) {
    free(peekBuffer);
    peekBuffer = NULL;
    peekBufferSize = 0;
  } else {
    memmove(peekBuffer, peekBuffer+consumed, peekBufferSize-consumed);
    peekBufferSize -= consumed;
  }
}

bool HTTPMultipartBodyParser::skipCRLF() {
  if (peekBufferSize < 2) {
    fillBuffer(2);
  }
  if (peekBufferSize < 2) {
    return false;
  }
  if (peekBuffer[0] != '\r') {
    return false;
  }
  if (peekBuffer[1] != '\n') {
    HTTPS_LOGE("Multipart incorrect line terminator");
    discardBody();
    return false;
  }
  consumedBuffer(2);
  return true;
}

std::string HTTPMultipartBodyParser::readLine() {
  fillBuffer(MAXLINESIZE);
  if (peekBufferSize == 0) {
    return "";
  }
  char *crPtr = (char *)memchr(peekBuffer, '\r', peekBufferSize);
  if (crPtr == NULL) {
    HTTPS_LOGE("Multipart line too long");
    discardBody();
    return "";
  }
  size_t lineLength = crPtr-peekBuffer;
  std::string rv(peekBuffer, lineLength);
  consumedBuffer(lineLength);
  skipCRLF();
  return rv;
}

// Returns true if the buffer contains a boundary (or possibly lastBoundary)
bool HTTPMultipartBodyParser::peekBoundary() {

  //Serial.print( "Peekboundary ");

  if (peekBuffer == NULL || peekBufferSize < boundary.size()) {

    /*
    if (peekBuffer == NULL)
    {
      Serial.print( "peekBuffer is NULL " );
    }
    Serial.print( peekBufferSize );
    Serial.print( " " );
    Serial.print( boundary.size() );
    Serial.println( "returning false" );
    */

    return false;
  }
  char *ptr = peekBuffer;
  if (*ptr == '\r') {
    ptr++;
  }
  if (*ptr == '\n') {
    ptr++;
  }

/*Serial.print( "boundary = ");
Serial.print( boundary.c_str() );
Serial.print( " " );
Serial.print( boundary.size() );
Serial.print( " " );
Serial.println( "=" );
for ( int z = 0; z<240; z++ )
{
  Serial.print( ptr[z] );
  Serial.print( "=" );
  Serial.print( ( (byte *) ptr)[z], HEX );
  Serial.print( ", " );

};
Serial.println( "+ memcmp=" );
Serial.println( memcmp(ptr, boundary.c_str(), boundary.size()) );
*/

  return memcmp(ptr, boundary.c_str(), boundary.size()) == 0;
}

bool HTTPMultipartBodyParser::nextField() {

  //Serial.println("nextField ***");

  fillBuffer(MAXLINESIZE);

  //Serial.println( "nextfield 111");

  while ( !peekBoundary() )
  {
    //Serial.println( "nextfield 222" );
    std::string dummy = readLine();

    if ( endOfBody() )
    {
      HTTPS_LOGE("Multipart missing last boundary");
      return false;
    }

    //Serial.println( "nextfield 333");
    fillBuffer(MAXLINESIZE);
    //Serial.println( "nextfield 444");
  }
  //Serial.println("nextField *** 1");

  skipCRLF();
  std::string line = readLine();

  if (line == lastBoundary) {
    //Serial.println("nextField *** 2");
    discardBody();
    return false;
  }
  if (line != boundary) {
    HTTPS_LOGE("Multipart incorrect boundary");
    return false;
  }

  //Serial.println("nextField *** 3");
  // Read header lines up to and including blank line
  fieldName = "";
  fieldMimeType = "text/plain";
  fieldFilename = "";
  while (true) {
    line = readLine();
    if (line == "") {
      break;
    }
    if (line.substr(0, 14) == "Content-Type: ") {
      fieldMimeType = line.substr(14);

      //Serial.print("nextField *** 4 ");
      //Serial.println( fieldMimeType.c_str() );
    }
    if (line.substr(0, 31) == "Content-Disposition: form-data;") {
      // Parse name=value; or name="value"; fields.
      std::string field;
      line = line.substr(31);
      while(true) {
        size_t pos = line.find_first_not_of(' ');
        if (pos != std::string::npos) {
          line = line.substr(pos);
        }
        if (line == "") break;
        pos = line.find(';');
        if (pos == std::string::npos) {
          field = line;
          line = "";
        } else {
          field = line.substr(0, pos);
          line = line.substr(pos+1);
        }
        pos = field.find('=');
        if (pos == std::string::npos) {
          HTTPS_LOGE("Multipart ill-formed form-data header");
          return false;
        }
        std::string headerName = field.substr(0, pos);
        std::string headerValue = field.substr(pos+1);
        if (headerValue.substr(0,1) == "\"") {
          headerValue = headerValue.substr(1, headerValue.size()-2);
        }
        if (headerName == "name") {
          fieldName = headerValue;
        }
        if (headerName == "filename") {
          fieldFilename = headerValue;
        }

        /*
        Serial.print("nextField *** fieldName=");
        Serial.println( fieldName.c_str() );
        Serial.print("nextField *** fieldFileName =" );
        Serial.println( fieldFilename.c_str() );
        */

      }
    }
  }
  if (fieldName == "") {
    HTTPS_LOGE("Multipart missing name");
    return false;
  }

  //Serial.println("nextField *** end");

  return true;
}

std::string HTTPMultipartBodyParser::getFieldName() {
  return fieldName;
}

std::string HTTPMultipartBodyParser::getFieldFilename() {
  return fieldFilename;
}

std::string HTTPMultipartBodyParser::getFieldMimeType() {
  return fieldMimeType;
}

bool HTTPMultipartBodyParser::endOfField() {
  return peekBoundary();
}

size_t HTTPMultipartBodyParser::read(byte* buffer, size_t bufferSize) {
  if (peekBoundary()) {
    return 0;
  }
  size_t readSize = std::min(bufferSize, MAXLINESIZE);
  fillBuffer(readSize);
  if (peekBoundary()) {
    return 0;
  }
  // We read at most up to a CR (so we don't miss a boundary that has been partially buffered)
  // but we always read at least one byte so if the first byte in the buffer is a CR we do read it.
  if (peekBufferSize > 1) {
    char *crPtr = (char *)memchr(peekBuffer+1, '\r', peekBufferSize-1);
    if (crPtr != NULL && crPtr - peekBuffer < bufferSize) {
      bufferSize = crPtr - peekBuffer;
    }
  }
  size_t copySize = std::min(bufferSize, peekBufferSize);
  memcpy(buffer, peekBuffer, copySize);
  consumedBuffer(copySize);
  return copySize;
}

} /* namespace httpsserver */
