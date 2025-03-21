/**
 * @file streams-vs1053-serial.ino
 * @author Phil Schatzmann
 * @brief Reads audio data from the VS1053 microphone
 * 
 * @author Phil Schatzmann
 * @copyright GPLv3
 */


#include "AudioTools.h"
#include "AudioLibs/VS1053Stream.h"

int channels = 1;
VS1053Stream in; // Access VS1053/VS1003 as stream
CsvOutput<int16_t> csvStream(Serial, channels);
StreamCopy copier(csvStream, in); // copy in to csvStream

// Arduino Setup
void setup(void) {
    Serial.begin(115200);
    AudioLogger::instance().begin(Serial, AudioLogger::Warning);
    
    auto cfg = in.defaultConfig(RX_MODE);
    cfg.sample_rate = 16000;
    cfg.channels = channels;
    cfg.input_device = VS1053_MIC; // or VS1053_AUX
    in.begin(cfg);

    // make sure that we have the correct channels set up
    csvStream.begin();

}

// Arduino loop - copy data
void loop() {
    copier.copy();
}
