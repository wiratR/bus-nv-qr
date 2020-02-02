#ifndef TFTLCD_H
#define TFTLCD_H

#include <Arduino.h>
//#include <SPI.h>
#include <Adafruit_GFX.h>           // include Adafruit graphics library
#include <Adafruit_ILI9341.h>       // include Adafruit ILI9341 TFT library
/*
#include <SdFat.h>                  // SD card & FAT filesystem library
#include <Adafruit_SPIFlash.h>      // SPI / QSPI flash library
#include <Adafruit_ImageReader.h>   // Image-reading function
*/
// Comment out the next line to load from SPI/QSPI flash instead of SD card:
//#define USE_SD_CARD

#if defined ESP8266
/*
  #define TFT_CS   0
  #define TFT_DC   15
  #define SD_CS    2
*/
    #define D2 (4)    // I2C Bus SDA (data)
    #define D3 (0)
    #define D8 (15)   // SPI Bus SS (CS)
    #define TFT_CS    D2 //D2     // TFT CS  pin is connected to NodeMCU pin D2
    #define TFT_RST   D3 //D3     // TFT RST pin is connected to NodeMCU pin D3
    #define TFT_DC    D8 //D4     // TFT DC  pin is connected to NodeMCU pin D4
    // initialize ILI9341 TFT library with hardware SPI module
    // SCK (CLK) ---> NodeMCU pin D5 (GPIO14)
    // MOSI(DIN) ---> NodeMCU pin D7 (GPIO13)
#elif defined ESP32
  #define TFT_CS   15
  #define TFT_DC   33
  #define SD_CS    14

#endif



class TFTLcd
{
    Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
/*
#if defined(USE_SD_CARD)
  SdFat                SD;         // SD card filesystem
  Adafruit_ImageReader reader(SD); // Image-reader object, pass in SD filesys
#else
  // SPI or QSPI flash filesystem (i.e. CIRCUITPY drive)
  #if defined(__SAMD51__) || defined(NRF52840_XXAA)
    Adafruit_FlashTransport_QSPI flashTransport(PIN_QSPI_SCK, PIN_QSPI_CS,
      PIN_QSPI_IO0, PIN_QSPI_IO1, PIN_QSPI_IO2, PIN_QSPI_IO3);
  #else
    #if (SPI_INTERFACES_COUNT == 1)
      Adafruit_FlashTransport_SPI flashTransport(SS, &SPI);
    #else
      Adafruit_FlashTransport_SPI flashTransport(SS1, &SPI1);
    #endif
  #endif
  Adafruit_SPIFlash    flash(&flashTransport);
  FatFileSystem        filesys;
  Adafruit_ImageReader reader(filesys); // Image-reader, pass in flash filesys
#endif


    Adafruit_ILI9341       tft    = Adafruit_ILI9341(TFT_CS, TFT_DC);
    Adafruit_Image         img;        // An image loaded into RAM
    int32_t                width  = 0, // BMP image dimensions
                           height = 0;
    */
    public :
        void initializeLcd();

    private :
        unsigned long testFillScreen();
};
#endif