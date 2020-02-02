#include "Arduino.h"
#include "TFTLcd.h"

unsigned long TFTLcd::testFillScreen() 
{
  unsigned long start = micros();
  tft.fillScreen(ILI9341_BLACK);
  tft.fillScreen(ILI9341_RED);
  tft.fillScreen(ILI9341_GREEN);
  tft.fillScreen(ILI9341_BLUE);
  tft.fillScreen(ILI9341_BLACK);
  return micros() - start;
}

void  TFTLcd::initializeLcd()
{

    Serial.println("ILI9341 Test!");   
    
    tft.begin();          // Initialize screen
 
    // read diagnostics (optional but can help debug problems)
    uint8_t x = tft.readcommand8(ILI9341_RDMODE);
    Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDMADCTL);
    Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDPIXFMT);
    Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDIMGFMT);
    Serial.print("Image Format: 0x"); Serial.println(x, HEX);
    x = tft.readcommand8(ILI9341_RDSELFDIAG);
    Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
    
    Serial.println(F("Benchmark                Time (microseconds)"));
  
    Serial.print(F("Screen fill              "));
    Serial.println(testFillScreen());
    delay(500);

/*
  // The Adafruit_ImageReader constructor call (above, before setup())
  // accepts an uninitialized SdFat or FatFileSystem object. This MUST
  // BE INITIALIZED before using any of the image reader functions!
  Serial.print(F("Initializing filesystem..."));
#if defined(USE_SD_CARD)
  // SD card is pretty straightforward, a single call...
  if(!SD.begin(SD_CS, SD_SCK_MHZ(25))) { // ESP32 requires 25 MHz limit
    Serial.println(F("SD begin() failed"));
    for(;;); // Fatal error, do not continue
  }
#else
  // SPI or QSPI flash requires two steps, one to access the bare flash
  // memory itself, then the second to access the filesystem within...
  if(!flash.begin()) {
    Serial.println(F("flash begin() failed"));
    for(;;);
  }
  if(!filesys.begin(&flash)) {
    Serial.println(F("filesys begin() failed"));
    for(;;);
  }
#endif
  Serial.println(F("OK!"));
  // Fill screen blue. Not a required step, this just shows that we're
  // successfully communicating with the screen.
  tft.fillScreen(ILI9341_BLUE);

  // Load full-screen BMP file 'purple.bmp' at position (0,0) (top left).
  // Notice the 'reader' object performs this, with 'tft' as an argument.
  Serial.print(F("Loading purple.bmp to screen..."));
  stat = reader.drawBMP("/purple.bmp", tft, 0, 0);
  // (Absolute path isn't necessary on most devices, but something
  // with the ESP32 SD library seems to require it.)
  reader.printStatus(stat);   // How'd we do?

  // Query the dimensions of image 'parrot.bmp' WITHOUT loading to screen:
  Serial.print(F("Querying parrot.bmp image size..."));
  stat = reader.bmpDimensions("/parrot.bmp", &width, &height);
  reader.printStatus(stat);   // How'd we do?
  if(stat == IMAGE_SUCCESS) { // If it worked, print image size...
    Serial.print(F("Image dimensions: "));
    Serial.print(width);
    Serial.write('x');
    Serial.println(height);
  }

  // Load small BMP 'wales.bmp' into a GFX canvas in RAM. This should
  // fail gracefully on AVR and other small devices, meaning the image
  // will not load, but this won't make the program stop or crash, it
  // just continues on without it.
  Serial.print(F("Loading wales.bmp to canvas..."));
  stat = reader.loadBMP("/wales.bmp", img);
  reader.printStatus(stat); // How'd we do?

  delay(2000); // Pause 2 seconds before moving on to loop()
*/
}
