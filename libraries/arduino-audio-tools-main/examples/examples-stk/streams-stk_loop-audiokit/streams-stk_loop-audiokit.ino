/**
 * @file streams-stk_loop-audiokit.ino
 * @author Phil Schatzmann
 * @brief We can use a MemoryLoop as input
 * @version 0.1
 * @date 2022-09-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "AudioTools.h"
#include "AudioLibs/AudioKit.h"
#include "AudioLibs/AudioSTK.h"

MemoryLoop mloop("crashcym.raw", crashcym_raw, crashcym_raw_len);
STKStream<MemoryLoop> in(mloop);
AudioKitStream out;
StreamCopy copier(out, in);

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial,AudioLogger::Warning);

  // setup input
  auto icfg = in.defaultConfig();
  in.begin(icfg);

  // setup output
  auto ocfg = out.defaultConfig(TX_MODE);
  ocfg.copyFrom(icfg);
  ocfg.sd_active = false;
  out.begin(ocfg);

}

void loop() {
  copier.copy();
}