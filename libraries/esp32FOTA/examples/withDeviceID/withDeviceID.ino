/**
   esp32 firmware OTA

   Purpose: Perform an OTA update from a bin located on a webserver (HTTP Only)

   Setup:
   Step 1 : Set your WiFi (ssid & password)
   Step 2 : set esp32fota()

   Upload:
   Step 1 : Menu > Sketch > Export Compiled Library. The bin file will be saved in the sketch folder (Menu > Sketch > Show Sketch folder)
   Step 2 : Upload it to your webserver
   Step 3 : Update your firmware JSON file ( see firwmareupdate )

*/

#include <esp32fota.h>
#include <WiFi.h>


// esp32fota esp32fota("<Type of Firme for this device>", <this version>, <validate signature>);
esp32FOTA esp32FOTA("esp32-fota-http", 1, false);
const char* manifest_url = "http://server/fota/fota.json";

void setup_wifi()
{
  delay(10);
  Serial.print("Connecting to WiFi");

  WiFi.begin(); // no WiFi creds in this demo :-)

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println(WiFi.localIP());
}


void setup()
{
  Serial.begin(115200);
  esp32FOTA.setManifestURL( manifest_url );
  esp32FOTA.useDeviceId( true );
  esp32FOTA.printConfig();
  setup_wifi();
}


void loop()
{
  bool updatedNeeded = esp32FOTA.execHTTPcheck();
  if (updatedNeeded)
  {
    esp32FOTA.execOTA();
  }

  delay(2000);
}
