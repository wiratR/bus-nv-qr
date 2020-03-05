#include "Arduino.h"
#include <SoftwareSerial.h>

#if defined(ESP8266) && !defined(D5)
#define D0 (16)
#define D1 (5)  // I2C Bus SCL (clock)
#define D2 (4)  // I2C Bus SDA (data)
#define D3 (0)
#define D4 (2)  // Same as "LED_BUILTIN", but inverted logic
#define D5 (14) // SPI Bus SCK (clock)
#define D6 (12) // SPI Bus MISO
#define D7 (13) // SPI Bus MOSI
#define D8 (15) // SPI Bus SS (CS)
#define D9 (3)  // RX0 (Serial console)
#define D10 (1) // TX0 (Serial console)
#define relay_out D0 // output
#else
#define D0 (0)  // GPIO
#define relay_out D0
#endif
//====================================================================================
//                                  Libraries
//====================================================================================
// Call up the SPIFFS FLASH filing system this is part of the ESP Core
#define FS_NO_GLOBALS
#include <FS.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <WiFiUdp.h>
#include <time.h>
// unwiredlabs Hostname & Geolocation Endpoint url
// refer :: https : //circuitdigest.com/microcontroller-projects/how-to-track-location-with-nodemcu-using-google-map-api
#include <WifiLocation.h>
#include "src/config/projectsKey.h"       // <add your key on this file>
#include "src/utils/utils.h"
#include "src/utils/data_types.h"
#include "src/API/scb.h"
#include "src/database/nvapi.h"

nvapi       db;
utils       utilsHelper;
scb         scbAPI;

SoftwareSerial swSer;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
WifiLocation location(googleApiKey);
// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
#define GMT_7_OFFSET 25200
// GMT +1 = 3600
// GMT +7 = 25200
// GMT +8 = 28800
// GMT -1 = -3600
// GMT 0 = 0

#define DV_STATUS       "/dv_status"        // Firebase Realtime Database node to store 'dv_status'
#define MAP_LOACTION    "/map_location"     // Firebase Realtime Database node to store 'map_location'
#define TX_USAGE        "/tx_usage"         // Firebase Realtime Database node to store 'tx_usage'
#define PAYMENT         "/payment_request"

#define WIFI_CONNTION_TIMEOUT 2000
#define BAUD_RATE 9600
// ====================================================================================
// ArdunioJson should be use version 5.x.x
 TFT_eSPI tft = TFT_eSPI();

boolean showsDebug = true; // set true for more debug output

FirebaseData firebaseData1;
String device_ip;
String sw_version = "2.0.0";
int device_number = 1;

location_t dv_location;
String path_dv_staus;
String path_tx_usage;
String path_pay_request;
int display_mode = 0;
int prev_display;


void setup() {
  // put your setup code here, to run once:
  // =============================================================
  // Hardware initail
  // =============================================================
  HardwareInitial();
  // =============================================================
  // WiFi initialised
  // =============================================================
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  //Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //////////////////////////////////////////////////////////////////////////////
  device_ip = utilsHelper.IpAddress2String(WiFi.localIP());
  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  timeClient.setTimeOffset(GMT_7_OFFSET);

  dv_location = location.getGeoFromWiFi();
  Serial.println("Location request data");
  // ======================================================
  // Firebase initialised
  // ======================================================
  db.initial_db(FIREBASE_HOST, FIREBASE_AUTH);
  // =====================================================
  // Sent Device status
  // =====================================================
  char deviceBuf[4];
  sprintf(deviceBuf, "%04X", device_number); // convert to Hex 4 digits Eg. 1 --> 0001 , 10 ---> 000A
  path_dv_staus = DV_STATUS + String("/evt_bus_") + \
                  String(deviceBuf[0]) + String(deviceBuf[1]) + \
                  String(deviceBuf[2]) + String(deviceBuf[3]);

  // =====================================================
  // TFT initialised
  // =====================================================
  if (!SPIFFS.begin())
  {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nSPIFFS initialised.");

  tft.begin();
  tft.setRotation(1); // set lanscape
  tft.fillScreen(TFT_WHITE);
  showScreen(display_mode); // reboting pages

  refreshDateTime();
  refreshLocations();

  if ( db.sentResponseDeviceStatus ( firebaseData1, path_dv_staus, 1, dv_location, device_ip, formattedDate, showsDebug) < 0 )
  {
    // write logs error
    Serial.println("sentResponseDeviceStatus failed");
    display_mode = 5; // out of Service
  }
  else
  {
    display_mode = 1; // welcome screen
  }
  delay(500);
  showScreen(display_mode);
  prev_display = display_mode;
  delay(300);
  
}

void HardwareInitial()
{
  /* set up PIN */
  pinMode(relay_out, OUTPUT);
  digitalWrite(relay_out, LOW);
  Serial.begin(115200);

  swSer.begin(BAUD_RATE, SWSERIAL_8N1, D1, D2, false, 95, 11);

  Serial.println("\nSoftware serial test started ======= ");
  for (char ch = ' '; ch <= 'z'; ch++)
  {
    swSer.write(ch);
  }
  swSer.println("");
}
// =============================================================

String qrData = "";
String paymentType;
String txn_status;
String paymentCodeId;


//String txtMsg = "";                              // a string for incoming text
unsigned int lastStringLength = qrData.length(); // previous length of the String
String ETag = "";
String deviceRecheckRequest;
int result = -1;

void loop()
{
  Serial.println("Test scb payment API");
  result = scbAPI.payment(qrData, "20.00");
  Serial.println("get a result : " + result);

  while (swSer.available()) 
  {
    char recieved = swSer.read();
    qrData += recieved;

    if (recieved == '\n')
    {
      Serial.print("Received: ");
      qrData.trim();
      Serial.println(qrData);
      Serial.print("<--- end of trimmed string. Length: ");
      Serial.println(qrData.length());
      display_mode = 2;  /// shows in processing mode
      showScreen(display_mode);
      refreshDateTime();
      refreshLocations();
      // Add your code to parse the received line here ............................
      if ((qrData.length() > 14) && (qrData.length() < 20))
      {
        paymentType = "trueMoney";
        Serial.println("setPaymentType is " + paymentType);
        // doing setPay API call trueMoney
        display_mode = 4;
      }
      else if (qrData.length() > 20)
      {
        // test sent https client get and POST
        // get a accesToken
        if ( scbAPI.payment(qrData, "20.00") < 0 )
        {
          Serial.println("scbAPI.payment is failed");
        }
        //
        //
        /* WR test disable Firebase
        txn_status = "request";
        paymentType = "scbAPI";
        Serial.println("setPaymentType is " + paymentType);
        // doing setPay API call scb
        path_pay_request = utilsHelper.getTxName(PAYMENT, device_number, dayStamp, timeStamp);
        if (db.sentRequestPay(firebaseData1,
                              path_pay_request,
                              paymentType,
                              qrData,
                              2000,
                              dv_location,
                              showsDebug) < 0)
        {
          Serial.println("payment is failed");
          display_mode = 4;
        }
        else
        {
          Serial.println("payment is sucess");
          display_mode = 3;
        }
        */
        /*
          // doing setPay API call scb
          path_tx_usage = utilsHelper.getTxName(TX_USAGE, device_number, dayStamp, timeStamp);
          if ( db.sentPayAPI (
                    firebaseData1,
                    path_tx_usage,
                    String(qrData),
                    "20.00",
                    dayStamp,
                    timeStamp,
                    dv_location,
                    device_ip,
                    showsDebug
                ) < 0 )
          {
          Serial.println("payment is failed");
          display_mode = 3;
          }
          else
          {
          Serial.println("payment is success");
          display_mode = 2;
          }
        */
      }
      else
      {
        Serial.println("read qrData failed.");
        display_mode = 4;
      }
      // Clear receive buffer so we're ready to receive the next line
      qrData = "";
      paymentType = "";
      showScreen(display_mode);
      delay(3000);
      display_mode = 1;
      showScreen(display_mode);
      digitalWrite(relay_out, HIGH);
      delay(250);
      digitalWrite(relay_out, LOW);
    }
  }


}

// Hardware Config
// TFT_screen <---> ESP8266 board
// We must use hardware SPI, a minimum of 3 GPIO pins is needed.
// Typical setup for ESP8266 NodeMCU ESP-12 is :
//
// Display SDO/MISO  to NodeMCU pin D6 (or leave disconnected if not reading TFT)
// Display LED       to NodeMCU pin VIN (or 5V, see below)
// Display SCK       to NodeMCU pin D5
// Display SDI/MOSI  to NodeMCU pin D7
// Display DC (RS/AO)to NodeMCU pin D3
// Display RESET     to NodeMCU pin D4 (or RST, see below)
// Display CS        to NodeMCU pin D8 (or GND, see below)
// Display GND       to NodeMCU pin GND (0V)
// Display VCC       to NodeMCU 5V or 3.3V


unsigned long showScreen(uint8_t pages)
{
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
      device_ip = utilsHelper.IpAddress2String(WiFi.localIP());

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
      //tft.drawXBitmap(0, 0, logo, logoWidth, logoHeight, TFT_BLACK);
      utilsHelper.drawBmp("/logo.bmp", 0, 0, tft);

      // Outlines are not included in timing results
      tft.drawRect(0, 150, 320, 30, TFT_BLACK);
      for (uint8_t i = 0; i <= 100; i++) {
        //fillRectangle(poX+i*size, poY+f*size, size, size, fgcolor);
        tft.fillRect(0, 150, i * 3.2, 30, TFT_BLACK);
        delay(150);
      }
      break;
    case 1:
      // welcome screen
      utilsHelper.drawBmp("/welcome.bmp", 0, 0, tft);
      break;
    case 2:
      // in processing
      utilsHelper.drawBmp("/inprocessing.bmp", 0, 0, tft);
      break;
    case 3:
      // Paymnet Success
      utilsHelper.drawBmp("/success.bmp", 0, 0, tft);
      break;
    case 4:
      // Paymnet Failed
      utilsHelper.drawBmp("/failed.bmp", 0, 0, tft);
      break;
    case 5:
      // Out Of Service
      utilsHelper.drawBmp("/outofservice.bmp", 0, 0, tft);
      break;
    default:
      // if nothing else matches, do the default
      // default is optional
      utilsHelper.drawBmp("/outofservice.bmp", 0, 0, tft);
      break;
  }
  return micros() - start;
}

// ============================================================
// this functions to cll refresh datetime
// ============================================================
void refreshDateTime()
{
  // get datetime is now
  while (!timeClient.update())
  {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  // Extract date
  int splitT    = formattedDate.indexOf("T");
  dayStamp      = formattedDate.substring(0, splitT);          // yyyy-mm-dd
  // Extract time
  timeStamp     = formattedDate.substring(splitT + 1, formattedDate.length() - 1);

  if (showsDebug)
  {
    Serial.println("refreshDateTime() : ");
    Serial.println("formattedDate : " + formattedDate);
    Serial.println("dayStamp      : " + dayStamp);
    Serial.println("timeStamp     : " + timeStamp);
  }
}

// ===========================================================
//
// ============================================================
void refreshLocations()
{
  location.getSurroundingWiFiJson();

  if (showsDebug)
  {
    Serial.println("Latitude: " + String(dv_location.lat, 7));
    Serial.println("Longitude: " + String(dv_location.lon, 7));
    Serial.println("Accuracy: " + String(dv_location.accuracy));
  }

}
