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

#ifdef ESP8266
  #include <NTPClient.h>
  #include <ESP8266WiFi.h>
  #include "FirebaseESP8266.h"
#else
  #include <WiFi.h>
  #include <NTPClient.h>
  #include <FirebaseESP32.h>
#endif

#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <WiFiUdp.h>
#include <time.h>
// unwiredlabs Hostname & Geolocation Endpoint url
// refer :: https : //circuitdigest.com/microcontroller-projects/how-to-track-location-with-nodemcu-using-google-map-api
#include <WifiLocation.h>
#include "src/config/projectsKey.h"       // <add your key on this file>
// ====================================================================================

WebServer server(80);

// =============== OTA DEFINE WEB UPLOAD_FILE =========================================== //
/* Style */
String style =
    "<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
    "input{background:#f1f1f1;border:0;padding:0 15px}body{background:#3498db;font-family:sans-serif;font-size:14px;color:#777}"
    "#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
    "#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
    "form{background:#fff;max-width:258px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
    ".btn{background:#3498db;color:#fff;cursor:pointer}</style>";

/* Login page */
String loginIndex =
    "<form name=loginForm>"
    "<h1>ESP32 Login</h1>"
    "<input name=userid placeholder='User ID'> "
    "<input name=pwd placeholder=Password type=Password> "
    "<input type=submit onclick=check(this.form) class=btn value=Login></form>"
    "<script>"
    "function check(form) {"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{window.open('/serverIndex')}"
    "else"
    "{alert('Error Password or Username')}"
    "}"
    "</script>" +
    style;

/* Server Index Page */
String serverIndex =
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
    "<label id='file-input' for='file'>   Choose file...</label>"
    "<input type='submit' class=btn value='Update'>"
    "<br><br>"
    "<div id='prg'></div>"
    "<br><div id='prgbar'><div id='bar'></div></div><br></form>"
    "<script>"
    "function sub(obj){"
    "var fileName = obj.value.split('\\\\');"
    "document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
    "};"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    "$.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "$('#bar').css('width',Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!') "
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>" +
    style;
// =========================================================================== //
// ArdunioJson should be use version 5.x.x
 TFT_eSPI tft = TFT_eSPI();

<<<<<<< HEAD
#include "src/utils/xbm.h"

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
WifiLocation location(googleApiKey);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
// GMT +1 = 3600
// GMT +7 = 25200
// GMT +8 = 28800
// GMT -1 = -3600
// GMT 0 = 0
#define GMT_7_OFFSET 25200

#define DV_STATUS     "/dv_status"        // Firebase Realtime Database node to store 'dv_status'
#define MAP_LOACTION  "/map_location"     // Firebase Realtime Database node to store 'map_location'
#define TX_USAGE      "/tx_usage"         // Firebase Realtime Database node to store 'tx_usage'
#define PAYMENT       "/payment_request"

#define WIFI_CONNTION_TIMEOUT 2000
=======
// Declare the Firebase Data object in global scope
FirebaseData firebaseData;
>>>>>>> parent of 3e27b21... add firebase

boolean showsDebug = true; // set true for more debug output

// =================================================================
// Declare the Firebase Data object in global scope
//Define FirebaseESP32 data objects
FirebaseData firebaseData1;

String device_ip;
String sw_version = "1.0.0";
int device_number = 1;

<<<<<<< HEAD
location_t dv_location;
String path_dv_staus;
String path_tx_usage;
String path_pay_request;
int display_mode = 0;
int prev_display;

#define BAUD_RATE 9600

// Reminder: the buffer size optimizations here, in particular the isrBufSize that only accommodates
// a single 8N1 word, are on the basis that any char written to the loopback SoftwareSerial adapter gets read
// before another write is performed. Block writes with a size greater than 1 would usually fail.
#ifdef ESP8266
SoftwareSerial swSer;
#else
SoftwareSerial swSer(14, 12, false, 256);
#endif

#include "src/utils/utils.h"
#include "src/database/nvapi.h"
#include "src/API/scb.h"

utils   utilsHelper;
nvapi   db;
scb     scbAPI;

unsigned long showScreen(uint8_t pages);
void refreshDateTime();
void refreshLocations();
int count = 0;

void setup()
{
=======
void setup() {
>>>>>>> parent of 3e27b21... add firebase
  // put your setup code here, to run once:
  // =============================================================
  // Hardware initail
  // =============================================================
  // HardwareInitial();
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

<<<<<<< HEAD
  // ===================== use mdns for host name resolution ========================== //
  if (!MDNS.begin(HOST_NAME))
  { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart(); }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    } });
  server.begin();
  //////////////////////////////////////////////////////////////////////////////
/*
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
=======
  if (!SPIFFS.begin()) {
>>>>>>> parent of 3e27b21... add firebase
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nSPIFFS initialised.");
<<<<<<< HEAD
*/
#ifdef SHOWLCD
  tft.begin();
  tft.setRotation(1); // set lanscape
  tft.fillScreen(TFT_WHITE);
  showScreen(display_mode); // reboting pages
#endif
/*
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
#ifdef SHOWLCD
  showScreen(display_mode);
#endif
  prev_display = display_mode;
  delay(300);
  */
}

void HardwareInitial()
{
  /* set up PIN */
  pinMode(relay_out, OUTPUT);
  digitalWrite(relay_out, LOW);
  Serial.begin(115200);

#ifdef ESP8266
  swSer.begin(BAUD_RATE, SWSERIAL_8N1, D1, D2, false, 95, 11);
#else
  swSer.begin(BAUD_RATE);
#endif
  Serial.println("\nSoftware serial test started ======= ");
  for (char ch = ' '; ch <= 'z'; ch++)
  {
    swSer.write(ch);
  }
  swSer.println("");
=======

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
>>>>>>> parent of 3e27b21... add firebase
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
  server.handleClient();      // add ---- Server.handle

  Serial.println("Test scb payment API");
  result = scbAPI.payment(qrData, "20.00");
  Serial.println("get a result : " + result);


#ifdef SHOWLCD
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


#endif


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
<<<<<<< HEAD
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
=======
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
>>>>>>> parent of 3e27b21... add firebase
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
