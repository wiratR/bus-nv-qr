#include "xbm.h"        // Sketch tab header for xbm images
#include <SPI.h>
//====================================================================================
//                                  Libraries
//====================================================================================
// Call up the SPIFFS FLASH filing system this is part of the ESP Core
#define FS_NO_GLOBALS
#include <FS.h>
#ifdef ESP32
#include "SPIFFS.h"   // For ESP32 only
#endif

// Call up the TFT library
#include <TFT_eSPI.h>   // Hardware-specific library for ESP8266

// Invoke TFT library
TFT_eSPI tft = TFT_eSPI();

// Include Firebase ESP8266 library (this library)
#include "FirebaseESP8266.h"
// Include ESP8266WiFi.h and must be included after FirebaseESP8266.h
#include <ESP8266WiFi.h>

// Config connect WiFi
#define WIFI_SSID "Bonny_2G_e00"
#define WIFI_PASSWORD "1234BAll"

// Config Firebase
#define FIREBASE_HOST  "qr-evt-db.firebaseio.com"
#define FIREBASE_AUTH  "LCX6Yyh4A9wuURogU0fhN03MbsfvWiRF2Z9iSl3z"
// ArdunioJson should be use version 5.x.x

// Declare the Firebase Data object in global scope
FirebaseData firebaseData;

void printResult(FirebaseData &data);

String device_ip = "";
String sw_version = "1.0.0";
int device_number = 1;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nSPIFFS initialised.");

  // Setup Firebase credential in setup()
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  // Optional, set AP reconnection in setup()
  Firebase.reconnectWiFi(true);

  testFirebase();

  // Now initialise the TFT
  tft.begin();
  tft.setRotation(1); // set lanscape
  tft.fillScreen(TFT_WHITE);
  showScreen(0);      // reboting pages
/*
  if(deviceStatus(firebaseData) < 0 )
  {
      Serial.println("\r\nsent device status failed.");
  }
  */
}

void loop() {
  // put your main code here, to run repeatedly:
  for (uint8_t pages = 1; pages < 5; pages++) {
    //drawBmp("/1.bmp", 0, 0);
    showScreen(pages);
    delay(3000);
  }
}


String ip2Str(IPAddress ip){
  String s;
  for (int i=0; i<4; i++) {
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  }
  return s;
}

unsigned long showScreen(uint8_t pages) {
  unsigned long start = micros();
  switch (pages) {
    case 0:
      // screen resolution 240 X 320
      // shows IP
      // shows deviceId
      // show software version
      // (on right side)
      // shows bar status loadiing   
      // get deivce IP
      device_ip = ip2Str(WiFi.localIP());
       
      tft.setCursor(160, 10);
      tft.setTextColor(TFT_BLACK);   tft.setTextSize(1);
      // print IP address
      tft.print  ("       IP  : ");  tft.println(device_ip);
      tft.setCursor(160, 25);
      // print device id
      tft.print  ("device id  : ");  tft.println(device_number);
      tft.setCursor(160, 40);
      tft.print  ("sw version : ");  tft.println(sw_version);
      // Draw bitmap with top left corner at x,y with foreground only color
      // Bits set to 1 plot as the defined color, bits set to 0 are not plotted
      //              x  y  xbm   xbm width  xbm height  color
      tft.drawXBitmap(0, 0, logo, logoWidth, logoHeight, TFT_BLACK);

      // Outlines are not included in timing results
      tft.drawRect(0, 150, 320, 30, TFT_BLACK);
      for (uint8_t i = 0; i <= 100; i++) {
        //fillRectangle(poX+i*size, poY+f*size, size, size, fgcolor);
        tft.fillRect(0, 150, i * 3.2, 30, TFT_BLACK);
        delay(150);
      }
      break;
    case 1:
      // shows welcome screen
      drawBmp("/1.bmp", 0, 0);
      break;
    case 2:
      // shows Paymnet Sucess
      drawBmp("/2.bmp", 0, 0);
      break;
    case 3:
      // shows Paymnet Failed
      drawBmp("/3.bmp", 0, 0);
      break;
    case 4:
      // show Out Of Service
      drawBmp("/4.bmp", 0, 0);
      break;
    default:
      // if nothing else matches, do the default
      // default is optional
      break;
  }
  return micros() - start;
}
