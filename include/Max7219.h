/*
 * Copyright (c) 2014-2017 Cesanta Software Limited
 * All rights reserved
 */

#include "mgos_features.h"
#include <stdint.h>
#include "mgos_timers.h"
#include "Arduino.h"
#include <WString.h>
#include "mgos_spi.h"

// max7219 registers
#define MAX7219_REG_NOOP         0x0
#define MAX7219_REG_DIGIT0       0x1
#define MAX7219_REG_DIGIT1       0x2
#define MAX7219_REG_DIGIT2       0x3
#define MAX7219_REG_DIGIT3       0x4
#define MAX7219_REG_DIGIT4       0x5
#define MAX7219_REG_DIGIT5       0x6
#define MAX7219_REG_DIGIT6       0x7
#define MAX7219_REG_DIGIT7       0x8
#define MAX7219_REG_DECODEMODE   0x9
#define MAX7219_REG_INTENSITY    0xA
#define MAX7219_REG_SCANLIMIT    0xB
#define MAX7219_REG_SHUTDOWN     0xC
#define MAX7219_REG_DISPLAYTEST  0xF

#define TEXT_ALIGN_LEFT          0 // Text is aligned to left side of the display
#define TEXT_ALIGN_LEFT_END      1 // Beginning of text is just outside the right end of the display
#define TEXT_ALIGN_RIGHT         2 // End of text is aligned to the right of the display
#define TEXT_ALIGN_RIGHT_END     3 // End of text is just outside the left side of the display


class Max7219Impl {
 public:
  Max7219Impl();
  Max7219Impl(byte _maxDevicesInArray, int8_t DIN, int8_t CS, int8_t CLK);
  Max7219Impl(byte _maxDevicesInArray, int8_t DIN, int8_t CS, int8_t CLK, int8_t _ESP32_SPI_BUS, bool ESP32_SPI_DEBUG);
  ~Max7219Impl();

  void init();
  void spiBegin();
  uint8_t spiTransfer(uint8_t data);
  void sendByte (const byte reg, const byte data);
  void sendByte (const byte device, const byte reg, const byte data);
  void setIntensity(byte intensity);
  void setCharWidth(byte charWidth);
  void setTextAlignment(byte textAlignment);
  void clear();
  void setPixel(byte x, byte y);
  void drawText();
  void setText(String text);
  void setNextText(String nextText);
  void setColumn(int column, byte value);
  void commit();
  void scrollTextRight();
  void scrollTextLeft();
  void oscillateText();

private:
    int inited = 0;
    byte* cols;
    byte spiregister[8];
    byte spidata[8];
    String myText;
    String myNextText;
    int myTextOffset = 1;
    int myTextAlignmentOffset = 0;
    int increment = -1;
    byte myCharWidth = 7;
    byte myTextAlignment = 1;
    byte maxDevicesInArray = 1;
    int8_t ESP32_SPI_BUS = 3;
    bool ESP32_SPI_DEBUG = true;
    int8_t _DIN = -1;
    int8_t _CS = -1;
    int8_t _CLK = -1;
    void calculateTextAlignmentOffset();  
};

extern Max7219Impl Max7219;

/* no_extern_c_check */
