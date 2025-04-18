/*
Reflections, distributed entertainment device

Repository is at https://github.com/frankcohen/ReflectionsOS
Includes board wiring directions, server side components, examples, support

Licensed under GPL v3 Open Source Software
(c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
Read the license in the license.txt file that comes with this code.

This file is for story functions, including decoding TAR data to storage,
set-up story elements like gestures, location triggers

*/

#include "Story.h"

#include <SPI.h>
#include <SD.h>

Story::Story(){}
