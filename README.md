# Arduino MAX-7219 Led Matrix library for Mongoose OS

Example:

in ./src/arduino.cpp:

```
#include <Arduino.h>
#include <Max7219.h>

// Native ESP32 SPI Pins
// - VSPI MISO = 19
// - VSPI MOSI = 23
// - VSPI SCK = 18
// - VSPI SS = 5

// - HSPI MISO = 12
// - HSPI MOSI = 13
// - HSPI SCK = 14
// - HSPI SS = 15

#define NUMBER_OF_DEVICES 1 //number of led matrix connect in series
#define DIN_PIN 23 // => MOSI
#define CLK_PIN 18 // => SCK
#define CS_PIN 5 // => SS
#define SPI_BUS 3 // SP32 VSPI=3 HSPI=2
#define SPI_DEBUG true // SP32 VSPI=3 HSPI=2

// uses default SPI config in mos.yml
Max7219Impl ledMatrix = Max7219Impl();

// Max7219Impl ledMatrix = Max7219Impl(NUMBER_OF_DEVICES, DIN_PIN, CS_PIN, CLK_PIN, SPI_BUS, SPI_DEBUG);

void setup(void) {
  delay(100);
  printf("Hello, world!\r\n");
  ledMatrix.setText("Hello World!");

}

void loop() {
  printf(".");

  ledMatrix.clear();
  ledMatrix.scrollTextLeft();
  ledMatrix.drawText();
  ledMatrix.commit();

  delay(500);  
}
```