
#include <Max7219.h>
#include "mongoose/mongoose.h"
#include "cp437font.h"

Max7219Impl Max7219;

struct mgos_spi *spi;
struct mgos_spi_txn txn;

Max7219Impl::Max7219Impl() {
  cols = new byte[maxDevicesInArray * 8];
}

Max7219Impl::Max7219Impl(byte _maxDevicesInArray, int8_t DIN, int8_t CS, int8_t CLK) {
  maxDevicesInArray = maxDevicesInArray;
  _DIN = DIN;
  _CS = CS;
  _CLK = CLK;
  cols = new byte[maxDevicesInArray * 8];
}

Max7219Impl::Max7219Impl(byte _maxDevicesInArray, int8_t DIN, int8_t CS, int8_t CLK, int8_t _ESP32_SPI_BUS, bool _ESP32_SPI_DEBUG) {
  maxDevicesInArray = maxDevicesInArray;
  _DIN = DIN;
  _CS = CS;
  _CLK = CLK;
  ESP32_SPI_BUS = _ESP32_SPI_BUS;
  ESP32_SPI_DEBUG = _ESP32_SPI_DEBUG;
  cols = new byte[maxDevicesInArray * 8];
}

Max7219Impl::~Max7219Impl() {
}

void Max7219Impl::init() {
  LOG(LL_INFO, ("Initing MAX7219..."));
  inited = 1;

  // for(byte device = 0; device < maxDevicesInArray; device++) {
  //   sendByte (device, MAX7219_REG_DISPLAYTEST, 1);
  // }
  // delay(1000);

  for(byte device = 0; device < maxDevicesInArray; device++) {
    sendByte (device, MAX7219_REG_DISPLAYTEST, 0);
    sendByte (device, MAX7219_REG_SCANLIMIT, 7);   // show all 8 digits
    sendByte (device, MAX7219_REG_DECODEMODE, 0);  // using an led matrix (not digits)
    sendByte (device, MAX7219_REG_INTENSITY, 0);   // character intensity: range: 0 to 15
    sendByte (device, MAX7219_REG_SHUTDOWN, 1);    // not in shutdown mode (ie. start it up)
  }

}

void Max7219Impl::spiBegin() {
  if (spi == nullptr) {
    if (_DIN == -1) {
      spi = mgos_spi_get_global();
    } else {
      struct mgos_config_spi bus_cfg = {};
      bus_cfg.unit_no = ESP32_SPI_BUS;
      bus_cfg.miso_gpio = 19;
      bus_cfg.mosi_gpio = _DIN;
      bus_cfg.sclk_gpio = _CLK;
      bus_cfg.cs0_gpio = _CS;
      bus_cfg.debug = true;
      spi = mgos_spi_create(&bus_cfg);
    }
  }

  if (spi == NULL) {
    LOG(LL_ERROR, ("Mongoose SPI is not configured, make sure spi.enable is true in mos.yml"));
  }

  txn = {
    .cs = 0, /* Use CS0 line as configured by cs0_gpio */
    .mode = 0,
    .freq = 10000000,
  };  
  // mgos_msleep(100);
}

uint8_t Max7219Impl::spiTransfer(uint8_t data) {  
  if (spi == nullptr) spiBegin();
  if (inited == 0) init();

  txn.fd.tx_data = txn.fd.rx_data = &data;
  txn.fd.len = sizeof(data);

  if (!mgos_spi_run_txn(spi, true /* full_duplex */, &txn)) {
    LOG(LL_ERROR, ("SPI transaction failed"));
    return 0;
  }
  return data;
}

void Max7219Impl::sendByte (const byte reg, const byte data) {
    for(byte device = 0; device < maxDevicesInArray; device++) {
        sendByte(device, reg, data);
    }
}

void Max7219Impl::sendByte (const byte device, const byte reg, const byte data) {

    int offset=device;
    int maxbytes=maxDevicesInArray;
    
    for(int i=0;i<maxbytes;i++) {
        spidata[i] = (byte)0;
        spiregister[i] = (byte)0;
    }
    // put our device data into the array
    spiregister[offset] = reg;
    spidata[offset] = data;

    // now shift out the data
    for(int i=0;i<maxDevicesInArray;i++) {
        // LOG(LL_INFO, ("SPI: Sending [%02x, %02x, %02x] bytes...", device, spiregister[i], spidata[i]));
        spiTransfer(spiregister[i]);
        spiTransfer(spidata[i]);      
    }
}

void Max7219Impl::setIntensity(const byte intensity) {
    sendByte(MAX7219_REG_INTENSITY, intensity);
}

void Max7219Impl::setCharWidth(byte charWidth) {
    myCharWidth = charWidth;
}

void Max7219Impl::setTextAlignment(byte textAlignment) {
    myTextAlignment = textAlignment;
    calculateTextAlignmentOffset();
}

void Max7219Impl::calculateTextAlignmentOffset() {
    switch(myTextAlignment) {
        case TEXT_ALIGN_LEFT:
            myTextAlignmentOffset = 0;
            break;
        case TEXT_ALIGN_LEFT_END:
            myTextAlignmentOffset = maxDevicesInArray * 8;
            break;
        case TEXT_ALIGN_RIGHT:
            myTextAlignmentOffset = myText.length() * myCharWidth - maxDevicesInArray * 8;
            break;
        case TEXT_ALIGN_RIGHT_END:
            myTextAlignmentOffset = - myText.length() * myCharWidth;
            break;
    }
    
}

void Max7219Impl::clear() {
    for (byte col = 0; col < maxDevicesInArray * 8; col++) {
        cols[col] = 0;
    }    
}

void Max7219Impl::commit() {
    // LOG(LL_INFO, ("Printing Screeen:"));
    for (byte col = 0; col < maxDevicesInArray * 8; col++) {
      //  LOG(LL_INFO, (" col [%02x] = %02x", col, cols[col]));

        sendByte(col / 8, col % 8 + 1, cols[col]);
    }
}

void Max7219Impl::setText(String text) {
    myText = text;
    myTextOffset = 0;
    calculateTextAlignmentOffset();
}

void Max7219Impl::setNextText(String nextText) {
    myNextText = nextText;
}

void Max7219Impl::scrollTextRight() {
    myTextOffset = (myTextOffset + 1) % ((int)myText.length() * myCharWidth - 5);
}

void Max7219Impl::scrollTextLeft() {
    myTextOffset = (myTextOffset - 1) % ((int)myText.length() * myCharWidth + maxDevicesInArray * 8);
    if (myTextOffset == 0 && myNextText.length() > 0) {
        myText = myNextText;
        myNextText = "";
        calculateTextAlignmentOffset();
    }
}

void Max7219Impl::oscillateText() {
    int maxColumns = (int)myText.length() * myCharWidth;
    int maxDisplayColumns = maxDevicesInArray * 8;
    if (maxDisplayColumns > maxColumns) {
        return;
    }
    if (myTextOffset - maxDisplayColumns == -maxColumns) {
        increment = 1;
    }
    if (myTextOffset == 0) {
        increment = -1;
    }
    myTextOffset += increment;
}

void Max7219Impl::drawText() {
    unsigned char letter;
    int position = 0;
    for (int i = 0; i < myText.length(); i++) {
        letter = myText.charAt(i);
        for (byte col = 0; col < 8; col++) {
            position = i * myCharWidth + col + myTextOffset + myTextAlignmentOffset;
            if (position >= 0 && position < maxDevicesInArray * 8) {
                setColumn(position, pgm_read_byte (&cp437_font [letter] [col]));
            }
        }
    }
}

void Max7219Impl::setColumn(int column, byte value) {
    if (column < 0 || column >= maxDevicesInArray * 8) {
        return;
    }
    cols[column] = value;
}

void Max7219Impl::setPixel(byte x, byte y) {
    bitWrite(cols[x], y, true);
}
