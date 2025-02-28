/*
 Reflections, mobile connected entertainment device

 Repository is at https://github.com/frankcohen/ReflectionsOS
 Includes board wiring directions, server side components, examples, support

 Licensed under GPL v3 Open Source Software
 (c) Frank Cohen, All rights reserved. fcohen@starlingwatch.com
 Read the license in the license.txt file that comes with this code.
*/

#ifndef ScienceFair14pt_h
#define ScienceFair14pt_h

#include <Arduino_GFX_Library.h>
#include <stdint.h>
#include <pgmspace.h>

const uint8_t ScienceFair14pt7bBitmaps[] PROGMEM = {
  0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x03, 0xFE, 0xEF, 0xDF, 0xBF,
  0x7E, 0xE0, 0x02, 0x00, 0x01, 0xC7, 0x00, 0xE7, 0x00, 0x63, 0x83, 0xFF,
  0xF9, 0xFF, 0xFC, 0xFF, 0xFC, 0x0C, 0x70, 0x0E, 0x38, 0x3F, 0xFF, 0x1F,
  0xFF, 0x9F, 0xFF, 0xC1, 0xC7, 0x00, 0xE7, 0x00, 0x63, 0x80, 0x00, 0x00,
  0x07, 0x00, 0x38, 0x01, 0xC0, 0x7F, 0xC7, 0xFF, 0x7F, 0xFF, 0x9C, 0xFC,
  0xE7, 0xE7, 0x3F, 0x38, 0x39, 0xC1, 0xCE, 0x0F, 0xFE, 0x3F, 0xF8, 0xFF,
  0xE0, 0xE7, 0x07, 0x38, 0x39, 0xF9, 0xCF, 0xCE, 0x7E, 0x73, 0xFF, 0xFD,
  0xFF, 0xC7, 0xFC, 0x07, 0x00, 0x38, 0x00, 0x00, 0x02, 0x00, 0x03, 0xC0,
  0xE0, 0x03, 0xFC, 0x38, 0x01, 0xFF, 0x9C, 0x00, 0xF0, 0xE7, 0x00, 0x38,
  0x1D, 0xC0, 0x0E, 0x07, 0x70, 0x03, 0x81, 0xF8, 0x00, 0xE0, 0x7E, 0x00,
  0x3C, 0x3B, 0x80, 0x07, 0xFE, 0xE3, 0xC0, 0xFF, 0x33, 0xFC, 0x1F, 0x1D,
  0xFF, 0x80, 0x07, 0x70, 0xE0, 0x01, 0xF8, 0x1C, 0x00, 0x6E, 0x07, 0x00,
  0x3B, 0x81, 0xC0, 0x0E, 0xE0, 0x70, 0x03, 0xBC, 0x38, 0x00, 0xE7, 0xFE,
  0x00, 0x70, 0xFF, 0x00, 0x1C, 0x1F, 0x00, 0x01, 0x00, 0x00, 0x3F, 0x80,
  0xFF, 0x03, 0xFE, 0x07, 0x00, 0x0E, 0x00, 0x1C, 0x00, 0x38, 0x1C, 0x70,
  0x38, 0xE0, 0x71, 0xFF, 0xFD, 0xFF, 0xFF, 0xFF, 0xFE, 0x07, 0x1C, 0x0E,
  0x38, 0x1C, 0x70, 0x38, 0xE0, 0x71, 0xC0, 0xE3, 0xFF, 0xC3, 0xFF, 0x83,
  0xFF, 0x00, 0xFF, 0xFE, 0x08, 0x73, 0xCE, 0x39, 0xC7, 0x1C, 0xE3, 0x8E,
  0x38, 0xE3, 0x8E, 0x38, 0xE3, 0x8E, 0x1C, 0x71, 0xC3, 0x8E, 0x3C, 0x70,
  0x80, 0x43, 0x8F, 0x1C, 0x70, 0xE3, 0x8E, 0x1C, 0x71, 0xC7, 0x1C, 0x71,
  0xC7, 0x1C, 0x71, 0xCE, 0x38, 0xE7, 0x1C, 0xF3, 0x84, 0x00, 0x07, 0x00,
  0x38, 0x19, 0xCD, 0xEE, 0xF7, 0xFF, 0x1F, 0xF0, 0x7F, 0x01, 0xF0, 0x1F,
  0xC1, 0xFF, 0x1F, 0xFD, 0xEE, 0xF6, 0x73, 0x03, 0x80, 0x1C, 0x00, 0x1C,
  0x1C, 0xFF, 0xFF, 0xFF, 0x1C, 0x1C, 0x1C, 0xFF, 0xFE, 0x73, 0x9C, 0xFF,
  0xFF, 0xFF, 0xFC, 0xFF, 0x80, 0x02, 0x01, 0xC1, 0xC0, 0xE0, 0x70, 0x38,
  0x38, 0x1C, 0x0E, 0x07, 0x07, 0x03, 0x81, 0xC0, 0xC0, 0xE0, 0x70, 0x38,
  0x18, 0x1C, 0x0E, 0x07, 0x07, 0x00, 0x80, 0x3F, 0xC7, 0xFE, 0xFF, 0xFE,
  0x07, 0xE0, 0xFE, 0x0F, 0xE1, 0xFE, 0x1F, 0xE3, 0x7E, 0x77, 0xE6, 0x7E,
  0xE7, 0xEC, 0x7F, 0x87, 0xF8, 0x7F, 0x07, 0xF0, 0x7E, 0x07, 0xFF, 0xF7,
  0xFE, 0x3F, 0xC0, 0xFF, 0xFF, 0xC7, 0x1C, 0x71, 0xC7, 0x1C, 0x71, 0xC7,
  0x1C, 0x71, 0xC7, 0x1C, 0x71, 0xC7, 0x1C, 0x3F, 0xC7, 0xFE, 0xFF, 0xFE,
  0x07, 0xE0, 0x7E, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x73,
  0xFF, 0x7F, 0xEF, 0xFC, 0xE0, 0x0E, 0x00, 0xE0, 0x0E, 0x00, 0xFF, 0xFF,
  0xFF, 0xFF, 0xF0, 0xFF, 0xE7, 0xFF, 0xBF, 0xFE, 0x00, 0x70, 0x03, 0x80,
  0x1C, 0x00, 0xE0, 0x07, 0x00, 0x38, 0x01, 0xC0, 0x0F, 0xFF, 0xFF, 0xFF,
  0x7F, 0xFC, 0x00, 0xE0, 0x07, 0x00, 0x38, 0x01, 0xFF, 0xFF, 0xFF, 0xEF,
  0xFE, 0x00, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0,
  0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7F, 0xFF, 0x7F, 0xF3, 0xFF, 0x00,
  0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x3F, 0xF7,
  0xFF, 0xFF, 0xFE, 0x00, 0xE0, 0x0E, 0x00, 0xE0, 0x0E, 0x00, 0xE0, 0x0E,
  0x00, 0xE0, 0x0F, 0xFC, 0xFF, 0xEF, 0xFF, 0x00, 0x70, 0x07, 0x00, 0x70,
  0x07, 0xFF, 0xFF, 0xFE, 0xFF, 0xC0, 0x3F, 0xC7, 0xFC, 0xFF, 0xCE, 0x00,
  0xE0, 0x0E, 0x00, 0xE0, 0x0F, 0xFC, 0xFF, 0xEF, 0xFF, 0xE0, 0x3E, 0x03,
  0xE0, 0x3E, 0x03, 0xE0, 0x3E, 0x03, 0xE0, 0x3E, 0x03, 0xFF, 0xF7, 0xFE,
  0x3F, 0xC0, 0x3F, 0xC7, 0xFE, 0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07, 0x00,
  0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00,
  0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x3F, 0xC7,
  0xFE, 0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7F, 0xFF, 0x7F, 0xEF,
  0xFF, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E,
  0x07, 0xFF, 0xF7, 0xFE, 0x3F, 0xC0, 0x3F, 0xC7, 0xFE, 0xFF, 0xFE, 0x07,
  0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7F, 0xFF,
  0x7F, 0xF3, 0xFF, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x1F, 0xF1, 0xFE,
  0x1F, 0xC0, 0xFF, 0x80, 0x00, 0x1F, 0xF0, 0x39, 0xCE, 0x00, 0x00, 0x00,
  0x07, 0xFF, 0xF3, 0x9C, 0xE0, 0xFF, 0xFE, 0x00, 0xF0, 0x07, 0xC0, 0x7A,
  0x02, 0xD8, 0x36, 0x41, 0x33, 0x19, 0x98, 0xCC, 0x44, 0x63, 0x63, 0x0A,
  0x18, 0x70, 0xC1, 0x06, 0x1C, 0x30, 0xA1, 0x8D, 0x8C, 0x6C, 0x66, 0x33,
  0x31, 0x99, 0x04, 0xD8, 0x36, 0x80, 0xBC, 0x07, 0xC0, 0x1E, 0x00, 0xFF,
  0xFE, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFE, 0x00, 0xF0, 0x07, 0xC0, 0x7A, 0x02, 0xD8, 0x36, 0x41, 0x33, 0x19,
  0x98, 0xCC, 0x44, 0x63, 0x63, 0x0A, 0x18, 0x70, 0xC1, 0x06, 0x1C, 0x30,
  0xA1, 0x8D, 0x8C, 0x6C, 0x66, 0x33, 0x31, 0x99, 0x04, 0xD8, 0x36, 0x80,
  0xBC, 0x07, 0xC0, 0x1E, 0x00, 0xFF, 0xFE, 0x3F, 0xC7, 0xFE, 0xFF, 0xFE,
  0x07, 0xE0, 0x7E, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70,
  0xFF, 0x0F, 0xE0, 0xFC, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00,
  0xE0, 0x0E, 0x00, 0xFF, 0xFE, 0x00, 0xF0, 0x07, 0xC0, 0x7A, 0x02, 0xD8,
  0x36, 0x41, 0x33, 0x19, 0x98, 0xCC, 0x44, 0x63, 0x63, 0x0A, 0x18, 0x70,
  0xC1, 0x06, 0x1C, 0x30, 0xA1, 0x8D, 0x8C, 0x6C, 0x66, 0x33, 0x31, 0x99,
  0x04, 0xD8, 0x36, 0x80, 0xBC, 0x07, 0xC0, 0x1E, 0x00, 0xFF, 0xFE, 0x0F,
  0xF0, 0x7F, 0xE3, 0xFF, 0xCE, 0x07, 0x38, 0x1C, 0xE0, 0x73, 0x81, 0xCE,
  0x07, 0x38, 0x1C, 0xE0, 0x73, 0x81, 0xCE, 0x07, 0x38, 0x1F, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0x38, 0x1C, 0xE0, 0x73, 0x81, 0xCE, 0x07, 0x38, 0x1C,
  0x3F, 0xF0, 0xFF, 0xE3, 0xFF, 0xCE, 0x07, 0x38, 0x1C, 0xE0, 0x73, 0x81,
  0xCE, 0x07, 0x38, 0x1C, 0xE0, 0x73, 0x81, 0xCE, 0x07, 0x38, 0x1F, 0xFF,
  0xFF, 0xFF, 0xBF, 0xFF, 0x38, 0x1C, 0xE0, 0x73, 0xFF, 0xCF, 0xFE, 0x3F,
  0xF0, 0x3F, 0xC7, 0xFE, 0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x0E,
  0x00, 0xE0, 0x0E, 0x00, 0xE0, 0x0E, 0x00, 0xE0, 0x0E, 0x00, 0xE0, 0x0E,
  0x07, 0xE0, 0x7E, 0x07, 0xFF, 0xF7, 0xFE, 0x3F, 0xC0, 0xFF, 0xCF, 0xFE,
  0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07,
  0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07,
  0xFF, 0xFF, 0xFE, 0xFF, 0xC0, 0x7F, 0xDF, 0xF7, 0xFD, 0xC0, 0x70, 0x1C,
  0x07, 0x01, 0xC0, 0x70, 0x1C, 0x07, 0x01, 0xC0, 0x70, 0x3F, 0xFF, 0xFF,
  0xFF, 0x70, 0x1C, 0x07, 0xFD, 0xFF, 0x7F, 0xC0, 0x7F, 0xDF, 0xF7, 0xFD,
  0xC0, 0x70, 0x1C, 0x07, 0x01, 0xC0, 0x70, 0x1C, 0x07, 0x01, 0xC0, 0x70,
  0x3F, 0xFF, 0xFF, 0xFF, 0x70, 0x1C, 0x07, 0x01, 0xC0, 0x70, 0x00, 0x3F,
  0xC7, 0xFE, 0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x0E, 0x00, 0xE0,
  0x0E, 0x00, 0xE0, 0x0E, 0x00, 0xE0, 0x0E, 0x1F, 0xE1, 0xFE, 0x1F, 0xE0,
  0x7E, 0x07, 0xFF, 0xF7, 0xFE, 0x3F, 0xC0, 0x38, 0x1C, 0xE0, 0x73, 0x81,
  0xCE, 0x07, 0x38, 0x1C, 0xE0, 0x73, 0x81, 0xCE, 0x07, 0x38, 0x1C, 0xE0,
  0x73, 0x81, 0xCE, 0x07, 0x38, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x38,
  0x1C, 0xE0, 0x73, 0x81, 0xCE, 0x07, 0x38, 0x1C, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFE, 0x03, 0x81, 0xC0, 0xE0, 0x70, 0x38, 0x1C, 0x0E,
  0x07, 0x03, 0x81, 0xC0, 0xE0, 0x70, 0x38, 0x1C, 0x0E, 0x07, 0x03, 0x81,
  0xFF, 0xFF, 0xEF, 0xE0, 0x38, 0x1C, 0xE0, 0x73, 0x81, 0xCE, 0x07, 0x38,
  0x1C, 0xE0, 0x73, 0x81, 0xCE, 0x07, 0x38, 0x1C, 0xE0, 0x73, 0x81, 0xCE,
  0x07, 0x38, 0x1F, 0xFF, 0xFF, 0xFF, 0xBF, 0xFF, 0x38, 0x1C, 0xE0, 0x73,
  0x81, 0xCE, 0x07, 0x38, 0x1C, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
  0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xFF,
  0xFF, 0xFF, 0x3F, 0xBF, 0x8F, 0xFF, 0xFB, 0xFF, 0xFF, 0xF0, 0x70, 0x7E,
  0x0E, 0x0F, 0xC1, 0xC1, 0xF8, 0x38, 0x3F, 0x07, 0x07, 0xE0, 0xE0, 0xFC,
  0x1C, 0x1F, 0x83, 0x83, 0xF0, 0x70, 0x7E, 0x0E, 0x0F, 0xC1, 0xC1, 0xF8,
  0x38, 0x3F, 0x07, 0x07, 0xE0, 0xE0, 0xFC, 0x1C, 0x1F, 0x83, 0x83, 0xF0,
  0x70, 0x7E, 0x0E, 0x0E, 0x3F, 0xC7, 0xFE, 0xFF, 0xFE, 0x07, 0xE0, 0x7E,
  0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E,
  0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x70,
  0x3F, 0xC7, 0xFE, 0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07,
  0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07,
  0xE0, 0x7E, 0x07, 0xFF, 0xF7, 0xFE, 0x3F, 0xC0, 0x3F, 0xF0, 0xFF, 0xE3,
  0xFF, 0xCE, 0x07, 0x38, 0x1C, 0xE0, 0x73, 0x81, 0xCE, 0x07, 0x38, 0x1C,
  0xE0, 0x73, 0x81, 0xCE, 0x07, 0x38, 0x1F, 0xFF, 0xFF, 0xFF, 0xBF, 0xFC,
  0x38, 0x00, 0xE0, 0x03, 0x80, 0x0E, 0x00, 0x38, 0x00, 0x3F, 0xC7, 0xFE,
  0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07,
  0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07,
  0xFF, 0xF7, 0xFE, 0x3F, 0xC0, 0x78, 0x03, 0x80, 0x10, 0x3F, 0xF0, 0xFF,
  0xE3, 0xFF, 0xCE, 0x07, 0x38, 0x1C, 0xE0, 0x73, 0x81, 0xCE, 0x07, 0x38,
  0x1C, 0xE0, 0x73, 0x81, 0xCE, 0x07, 0x38, 0x1F, 0xFF, 0xFF, 0xFF, 0xBF,
  0xFF, 0x38, 0x1C, 0xE0, 0x73, 0x81, 0xCE, 0x07, 0x38, 0x1C, 0x3F, 0xC7,
  0xFE, 0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x0E, 0x00, 0xE0, 0x0E,
  0x00, 0xE0, 0x0E, 0x00, 0xE0, 0x0F, 0xFC, 0x7F, 0xE3, 0xFF, 0x00, 0x70,
  0x07, 0xFF, 0xFF, 0xFE, 0xFF, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x70,
  0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70,
  0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70,
  0x07, 0x00, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0,
  0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0,
  0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xFF, 0xF7, 0xFE, 0x3F, 0xC0, 0xE0, 0x7E,
  0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E,
  0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E,
  0x07, 0xFF, 0xFF, 0xFE, 0xFF, 0xC0, 0xE0, 0xE0, 0xFC, 0x1C, 0x1F, 0x83,
  0x83, 0xF0, 0x70, 0x7E, 0x0E, 0x0F, 0xC1, 0xC1, 0xF8, 0x38, 0x3F, 0x07,
  0x07, 0xE0, 0xE0, 0xFC, 0x1C, 0x1F, 0x83, 0x83, 0xF0, 0x70, 0x7E, 0x0E,
  0x0F, 0xC1, 0xC1, 0xF8, 0x38, 0x3F, 0x07, 0x07, 0xE0, 0xE0, 0xFC, 0x1C,
  0x1F, 0xFF, 0xFF, 0xBF, 0xFF, 0xE3, 0xFB, 0xF8, 0xE0, 0x7E, 0x07, 0xE0,
  0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0,
  0x7E, 0x07, 0xE0, 0x7F, 0xFF, 0x7F, 0xEF, 0xFF, 0xE0, 0x7E, 0x07, 0xE0,
  0x7E, 0x07, 0xE0, 0x70, 0xE0, 0x3F, 0x01, 0xF8, 0x0F, 0xC0, 0x7E, 0x03,
  0xF0, 0x1F, 0x80, 0xFC, 0x07, 0xE0, 0x3F, 0x01, 0xF8, 0x0F, 0xC0, 0x7E,
  0x03, 0xFF, 0xFD, 0xFF, 0xC7, 0xFC, 0x07, 0x00, 0x38, 0x01, 0xC0, 0x0E,
  0x00, 0x70, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x07, 0x00, 0x70, 0x07,
  0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x73, 0xFF,
  0x7F, 0xEF, 0xFC, 0xE0, 0x0E, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xFF,
  0xFF, 0xF8, 0xE3, 0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xE3,
  0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xE3, 0x8F, 0xFF, 0xFC, 0x20, 0x30, 0x1C,
  0x0E, 0x07, 0x01, 0x80, 0xE0, 0x70, 0x38, 0x0C, 0x07, 0x03, 0x81, 0xC0,
  0x60, 0x38, 0x1C, 0x0E, 0x03, 0x81, 0xC0, 0xE0, 0x30, 0x1C, 0x08, 0xFF,
  0xFF, 0xC7, 0x1C, 0x71, 0xC7, 0x1C, 0x71, 0xC7, 0x1C, 0x71, 0xC7, 0x1C,
  0x71, 0xC7, 0x1C, 0x71, 0xC7, 0x1C, 0x7F, 0xFF, 0xFC, 0xFF, 0xFE, 0x00,
  0xF0, 0x07, 0xC0, 0x7A, 0x02, 0xD8, 0x36, 0x41, 0x33, 0x19, 0x98, 0xCC,
  0x44, 0x63, 0x63, 0x0A, 0x18, 0x70, 0xC1, 0x06, 0x1C, 0x30, 0xA1, 0x8D,
  0x8C, 0x6C, 0x66, 0x33, 0x31, 0x99, 0x04, 0xD8, 0x36, 0x80, 0xBC, 0x07,
  0xC0, 0x1E, 0x00, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x60, 0xF1,
  0xF8, 0xF0, 0x40, 0x3F, 0xC7, 0xFE, 0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07,
  0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF,
  0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x70, 0xFF,
  0xCF, 0xFE, 0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0,
  0x7E, 0x07, 0xE0, 0x7F, 0xFF, 0xFF, 0xEF, 0xFF, 0xE0, 0x7E, 0x07, 0xE0,
  0x7E, 0x07, 0xFF, 0xFF, 0xFE, 0xFF, 0xC0, 0x3F, 0xC7, 0xFE, 0xFF, 0xFE,
  0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x0E, 0x00, 0xE0, 0x0E, 0x00, 0xE0, 0x0E,
  0x00, 0xE0, 0x0E, 0x00, 0xE0, 0x0E, 0x07, 0xE0, 0x7E, 0x07, 0xFF, 0xF7,
  0xFE, 0x3F, 0xC0, 0xFF, 0xCF, 0xFE, 0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07,
  0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07,
  0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xFF, 0xFF, 0xFE, 0xFF, 0xC0, 0xFF,
  0xFF, 0xFF, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xFF, 0xFF,
  0xFF, 0xE0, 0xE0, 0xE0, 0xE0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0,
  0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xFF, 0xFF, 0xFF, 0xE0, 0xE0,
  0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0x3F, 0xC7, 0xFE, 0xFF, 0xFE, 0x07, 0xE0,
  0x7E, 0x07, 0xE0, 0x0E, 0x00, 0xE0, 0x0E, 0x00, 0xE0, 0x0E, 0x1F, 0xE1,
  0xFE, 0x1F, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xFF, 0xF7, 0xFE, 0x3F,
  0xC0, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E,
  0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x7E,
  0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x70, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0x03, 0x81, 0xC0, 0xE0, 0x70, 0x38, 0x1C,
  0x0E, 0x07, 0x03, 0x81, 0xC0, 0xE0, 0x70, 0x38, 0x1C, 0x0E, 0x07, 0x03,
  0x81, 0xFF, 0xFF, 0xEF, 0xE0, 0xE0, 0x77, 0x03, 0xB8, 0x39, 0xC3, 0xCE,
  0x1C, 0x71, 0xE3, 0x8E, 0x1C, 0xE0, 0xE7, 0x07, 0x70, 0x3F, 0x81, 0xFC,
  0x0F, 0xF0, 0x7F, 0x83, 0xCE, 0x1E, 0x70, 0xE1, 0xC7, 0x0F, 0x38, 0x39,
  0xC0, 0xEE, 0x07, 0x00, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
  0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xFF, 0xFF,
  0xFF, 0xE0, 0x03, 0xF8, 0x03, 0xFC, 0x01, 0xFF, 0x01, 0xFF, 0xC1, 0xFF,
  0xE0, 0xFF, 0xB8, 0xEF, 0xDC, 0x77, 0xE7, 0x73, 0xF3, 0xB9, 0xF8, 0xF8,
  0xFC, 0x7C, 0x7E, 0x1C, 0x3F, 0x0E, 0x1F, 0x80, 0x0F, 0xC0, 0x07, 0xE0,
  0x03, 0xF0, 0x01, 0xF8, 0x00, 0xFC, 0x00, 0x7E, 0x00, 0x38, 0xE0, 0x7F,
  0x07, 0xF8, 0x7F, 0xC7, 0xFC, 0x7E, 0xE7, 0xEF, 0x7E, 0x77, 0xE3, 0xFE,
  0x1F, 0xE1, 0xFE, 0x0F, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E,
  0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x70, 0x3F, 0xC7, 0xFE, 0xFF, 0xFE, 0x07,
  0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07,
  0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xFF, 0xF7, 0xFE,
  0x3F, 0xC0, 0xFF, 0xCF, 0xFE, 0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07, 0xE0,
  0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7F, 0xFF, 0xFF,
  0xEF, 0xFC, 0xE0, 0x0E, 0x00, 0xE0, 0x0E, 0x00, 0xE0, 0x00, 0x3F, 0xC7,
  0xFE, 0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E,
  0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E,
  0x07, 0xFF, 0xF7, 0xFE, 0x3F, 0xC0, 0x78, 0x03, 0x80, 0x10, 0xFF, 0xCF,
  0xFE, 0xFF, 0xFE, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E,
  0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7F, 0xFF, 0xFF, 0xEF, 0xFC, 0xE3, 0x8E,
  0x1C, 0xE1, 0xCE, 0x0E, 0xE0, 0xF0, 0x3F, 0xC7, 0xFE, 0xFF, 0xFE, 0x07,
  0xE0, 0x7E, 0x07, 0xE0, 0x0E, 0x00, 0xE0, 0x0F, 0xFC, 0x7F, 0xE3, 0xFF,
  0x00, 0x70, 0x07, 0x00, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xFF, 0xF7, 0xFE,
  0x3F, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x70, 0x07, 0x00, 0x70, 0x07,
  0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07,
  0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0x70, 0x07, 0x00, 0xE0, 0x7E,
  0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E,
  0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E, 0x07, 0xE0, 0x7E,
  0x07, 0xFF, 0xF7, 0xFE, 0x3F, 0xC0, 0x60, 0x07, 0x70, 0x0E, 0x70, 0x0E,
  0x70, 0x0E, 0x38, 0x1C, 0x38, 0x1C, 0x38, 0x1C, 0x18, 0x18, 0x1C, 0x38,
  0x1C, 0x38, 0x1C, 0x38, 0x0E, 0x70, 0x0E, 0x70, 0x0E, 0x70, 0x07, 0xE0,
  0x07, 0xE0, 0x07, 0xE0, 0x03, 0xE0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0,
  0x61, 0xC0, 0x1D, 0xC7, 0x00, 0x77, 0x1C, 0x03, 0x9C, 0x38, 0x0E, 0x38,
  0xE0, 0x38, 0xE3, 0x81, 0xC3, 0x86, 0x07, 0x06, 0x1C, 0x1C, 0x1C, 0x70,
  0xE0, 0x71, 0xC3, 0x81, 0xC3, 0x8E, 0x03, 0x9E, 0x38, 0x0E, 0x79, 0xC0,
  0x39, 0xE7, 0x00, 0x7F, 0xDC, 0x01, 0xFF, 0xE0, 0x07, 0xFF, 0x80, 0x0F,
  0xBE, 0x00, 0x3C, 0xF0, 0x00, 0xF3, 0xC0, 0x03, 0xC7, 0x00, 0x70, 0x0E,
  0x70, 0x0E, 0x38, 0x1C, 0x3C, 0x3C, 0x1C, 0x38, 0x0E, 0x70, 0x0E, 0x70,
  0x07, 0xE0, 0x07, 0xE0, 0x03, 0xC0, 0x03, 0xC0, 0x03, 0xC0, 0x07, 0xE0,
  0x07, 0xE0, 0x0E, 0x70, 0x0E, 0x70, 0x1C, 0x38, 0x3C, 0x3C, 0x38, 0x1C,
  0x70, 0x0E, 0x70, 0x0E, 0xE0, 0x0E, 0xE0, 0x39, 0xC0, 0x73, 0xC1, 0xE3,
  0x83, 0x87, 0x07, 0x07, 0x1C, 0x0E, 0x38, 0x0E, 0xE0, 0x1D, 0xC0, 0x3F,
  0x80, 0x3E, 0x00, 0x7C, 0x00, 0x70, 0x00, 0xE0, 0x01, 0xC0, 0x03, 0x80,
  0x07, 0x00, 0x0E, 0x00, 0x1C, 0x00, 0x38, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
  0xF0, 0x07, 0x00, 0xE0, 0x1E, 0x01, 0xC0, 0x38, 0x03, 0x80, 0x70, 0x0F,
  0x00, 0xE0, 0x1C, 0x01, 0xC0, 0x38, 0x07, 0x80, 0x70, 0x0E, 0x00, 0xFF,
  0xFF, 0xFF, 0xFF, 0xF0, 0x0C, 0x73, 0xCE, 0x38, 0xE3, 0x8E, 0x38, 0xE3,
  0x8E, 0x3B, 0xEF, 0xBE, 0x38, 0xE3, 0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xE3,
  0xC7, 0x0C, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE3,
  0xCF, 0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xF3, 0xCF, 0x38,
  0xE3, 0x8E, 0x38, 0xE3, 0x8E, 0x38, 0xEF, 0xBC, 0xE0, 0xFF, 0xFE, 0x00,
  0xF0, 0x07, 0xC0, 0x7A, 0x02, 0xD8, 0x36, 0x41, 0x33, 0x19, 0x98, 0xCC,
  0x44, 0x63, 0x63, 0x0A, 0x18, 0x70, 0xC1, 0x06, 0x1C, 0x30, 0xA1, 0x8D,
  0x8C, 0x6C, 0x66, 0x33, 0x31, 0x99, 0x04, 0xD8, 0x36, 0x80, 0xBC, 0x07,
  0xC0, 0x1E, 0x00, 0xFF, 0xFE };

const GFXglyph ScienceFair14pt7bGlyphs[] PROGMEM = {
  {     0,   1,   1,   7,    0,    0 },   // 0x20 ' '
  {     1,   3,  21,   5,    1,  -20 },   // 0x21 '!'
  {     9,   7,   5,   9,    1,  -20 },   // 0x22 '"'
  {    14,  17,  16,  19,    1,  -14 },   // 0x23 '#'
  {    48,  13,  26,  15,    1,  -23 },   // 0x24 '$'
  {    91,  26,  23,  28,    1,  -21 },   // 0x25 '%'
  {   166,  15,  21,  17,    1,  -20 },   // 0x26 '&'
  {   206,   3,   5,   5,    1,  -20 },   // 0x27 '''
  {   208,   6,  27,   8,    1,  -23 },   // 0x28 '('
  {   229,   6,  27,   8,    1,  -23 },   // 0x29 ')'
  {   250,  13,  15,  15,    1,  -20 },   // 0x2A '*'
  {   275,   8,   8,  10,    1,  -13 },   // 0x2B '+'
  {   283,   5,   6,   7,    1,   -2 },   // 0x2C ','
  {   287,  10,   3,  12,    1,  -11 },   // 0x2D '-'
  {   291,   3,   3,   5,    1,   -2 },   // 0x2E '.'
  {   293,   9,  23,  11,    1,  -21 },   // 0x2F '/'
  {   319,  12,  21,  14,    1,  -20 },   // 0x30 '0'
  {   351,   6,  21,   8,    1,  -20 },   // 0x31 '1'
  {   367,  12,  21,  14,    1,  -20 },   // 0x32 '2'
  {   399,  13,  21,  15,    1,  -20 },   // 0x33 '3'
  {   434,  12,  21,  14,    1,  -20 },   // 0x34 '4'
  {   466,  12,  21,  14,    1,  -20 },   // 0x35 '5'
  {   498,  12,  21,  14,    1,  -20 },   // 0x36 '6'
  {   530,  12,  21,  14,    1,  -20 },   // 0x37 '7'
  {   562,  12,  21,  14,    1,  -20 },   // 0x38 '8'
  {   594,  12,  21,  14,    1,  -20 },   // 0x39 '9'
  {   626,   3,  12,   5,    1,  -11 },   // 0x3A ':'
  {   631,   5,  15,   7,    1,  -11 },   // 0x3B ';'
  {   641,  13,  27,  15,    1,  -21 },   // 0x3C '<'
  {   685,  10,   8,  12,    1,  -11 },   // 0x3D '='
  {   695,  13,  27,  15,    1,  -21 },   // 0x3E '>'
  {   739,  12,  21,  14,    1,  -20 },   // 0x3F '?'
  {   771,  13,  27,  15,    1,  -21 },   // 0x40 '@'
  {   815,  14,  21,  16,    1,  -20 },   // 0x41 'A'
  {   852,  14,  21,  16,    1,  -20 },   // 0x42 'B'
  {   889,  12,  21,  14,    1,  -20 },   // 0x43 'C'
  {   921,  12,  21,  14,    1,  -20 },   // 0x44 'D'
  {   953,  10,  21,  12,    1,  -20 },   // 0x45 'E'
  {   980,  10,  21,  12,    1,  -20 },   // 0x46 'F'
  {  1007,  12,  21,  14,    1,  -20 },   // 0x47 'G'
  {  1039,  14,  21,  16,    1,  -20 },   // 0x48 'H'
  {  1076,   3,  21,   5,    1,  -20 },   // 0x49 'I'
  {  1084,   9,  21,  11,    1,  -20 },   // 0x4A 'J'
  {  1108,  14,  21,  16,    1,  -20 },   // 0x4B 'K'
  {  1145,   8,  21,  10,    1,  -20 },   // 0x4C 'L'
  {  1166,  19,  21,  21,    1,  -20 },   // 0x4D 'M'
  {  1216,  12,  21,  14,    1,  -20 },   // 0x4E 'N'
  {  1248,  12,  21,  14,    1,  -20 },   // 0x4F 'O'
  {  1280,  14,  21,  16,    1,  -20 },   // 0x50 'P'
  {  1317,  12,  24,  14,    1,  -20 },   // 0x51 'Q'
  {  1353,  14,  21,  16,    1,  -20 },   // 0x52 'R'
  {  1390,  12,  21,  14,    1,  -20 },   // 0x53 'S'
  {  1422,  12,  21,  14,    1,  -20 },   // 0x54 'T'
  {  1454,  12,  21,  14,    1,  -20 },   // 0x55 'U'
  {  1486,  12,  21,  14,    1,  -20 },   // 0x56 'V'
  {  1518,  19,  21,  21,    1,  -20 },   // 0x57 'W'
  {  1568,  12,  21,  14,    1,  -20 },   // 0x58 'X'
  {  1600,  13,  21,  15,    1,  -20 },   // 0x59 'Y'
  {  1635,  12,  21,  14,    1,  -20 },   // 0x5A 'Z'
  {  1667,   6,  29,   8,    1,  -24 },   // 0x5B '['
  {  1689,   9,  23,  11,    1,  -21 },   // 0x5C '\'
  {  1715,   6,  29,   8,    1,  -24 },   // 0x5D ']'
  {  1737,  13,  27,  15,    1,  -21 },   // 0x5E '^'
  {  1781,  11,   3,  13,    1,    1 },   // 0x5F '_'
  {  1786,   7,   5,  10,    1,  -21 },   // 0x60 '`'
  {  1791,  12,  21,  14,    1,  -20 },   // 0x61 'a'
  {  1823,  12,  21,  14,    1,  -20 },   // 0x62 'b'
  {  1855,  12,  21,  14,    1,  -20 },   // 0x63 'c'
  {  1887,  12,  21,  14,    1,  -20 },   // 0x64 'd'
  {  1919,   8,  21,  10,    1,  -20 },   // 0x65 'e'
  {  1940,   8,  21,  10,    1,  -20 },   // 0x66 'f'
  {  1961,  12,  21,  14,    1,  -20 },   // 0x67 'g'
  {  1993,  12,  21,  14,    1,  -20 },   // 0x68 'h'
  {  2025,   3,  21,   5,    1,  -20 },   // 0x69 'i'
  {  2033,   9,  21,  11,    1,  -20 },   // 0x6A 'j'
  {  2057,  13,  21,  15,    1,  -20 },   // 0x6B 'k'
  {  2092,   8,  21,  10,    1,  -20 },   // 0x6C 'l'
  {  2113,  17,  21,  19,    1,  -20 },   // 0x6D 'm'
  {  2158,  12,  21,  14,    1,  -20 },   // 0x6E 'n'
  {  2190,  12,  21,  14,    1,  -20 },   // 0x6F 'o'
  {  2222,  12,  21,  14,    1,  -20 },   // 0x70 'p'
  {  2254,  12,  24,  14,    1,  -20 },   // 0x71 'q'
  {  2290,  12,  21,  14,    1,  -20 },   // 0x72 'r'
  {  2322,  12,  21,  14,    1,  -20 },   // 0x73 's'
  {  2354,  12,  21,  14,    1,  -20 },   // 0x74 't'
  {  2386,  12,  21,  14,    1,  -20 },   // 0x75 'u'
  {  2418,  16,  21,  18,    1,  -20 },   // 0x76 'v'
  {  2460,  22,  21,  24,    1,  -20 },   // 0x77 'w'
  {  2518,  16,  21,  18,    1,  -20 },   // 0x78 'x'
  {  2560,  15,  21,  17,    1,  -20 },   // 0x79 'y'
  {  2600,  12,  21,  14,    1,  -20 },   // 0x7A 'z'
  {  2632,   6,  29,   8,    1,  -24 },   // 0x7B '{'
  {  2654,   3,  24,   5,    1,  -22 },   // 0x7C '|'
  {  2663,   6,  29,   8,    1,  -24 },   // 0x7D '}'
  {  2685,  13,  27,  15,    1,  -21 } }; // 0x7E '~'

const GFXfont ScienceFair14pt7b PROGMEM = {
  (uint8_t  *)ScienceFair14pt7bBitmaps,
  (GFXglyph *)ScienceFair14pt7bGlyphs,
  0x20, 0x7E, 34 };

// Approx. 3401 bytes


#endif
