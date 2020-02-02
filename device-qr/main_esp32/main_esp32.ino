#include <Arduino.h>

// Board especific libraries
#if defined ESP8266 || defined ESP32
// Use mDNS ? (comment this do disable it)
#define USE_MDNS true
// Arduino OTA (uncomment this to enable)
#define USE_ARDUINO_OTA false

#define USE_ESP8266_WEBUPDATE true

#endif // ESP

//////// Libraries
#if defined ESP8266
// Includes of ESP8266
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266Ping.h>
#include <ESP8266HTTPUpdateServer.h>
#include <SoftwareSerial.h>
// List HardWare Board
#define D0 (16)
#define D1 (5)    // I2C Bus SCL (clock)
#define D2 (4)    // I2C Bus SDA (data)
#define D3 (0)
#define D4 (2)    // Same as "LED_BUILTIN", but inverted logic
#define D5 (14)   // SPI Bus SCK (clock)
#define D6 (12)   // SPI Bus MISO
#define D7 (13)   // SPI Bus MOSI
#define D8 (15)   // SPI Bus SS (CS)
#define D9 (3)    // RX0 (Serial console)
#define D10 (1)   // TX0 (Serial console)
#define relay_out D0 // output
#ifdef USE_MDNS
#include <DNSServer.h>
#include <ESP8266mDNS.h>

#define UPDATE_SIZE_UNKNOWN 0xFFFFFFFF

#endif

#elif defined ESP32
// Includes of ESP32
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiClient.h>
#include <ESP32Ping.h>
#include <FirebaseESP32.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Update.h>
#ifdef USE_MDNS
#include <DNSServer.h>
#include <ESPmDNS.h>
#endif

#endif // ESP
#include <WiFiUdp.h>
#include <time.h>

// Remote debug over WiFi - not recommended for production, only for development
#include "RemoteDebug.h"                //https://github.com/JoaoLopesF/RemoteDebug
RemoteDebug Debug;

#include "src/config/projectsKey.h"
#include "src/utils/utils.h"
#include "src/utils/data_types.h"
#include "src/utils/TFTLcd.h"
#include "src/API/scb.h"
#include "src/database/nvapi.h"

nvapi       db;
utils       utilsHelper;
scb         scbAPI;
TFTLcd      screen;


//#include "src/utils/xbm.h"

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

boolean showsDebug = true; // set true for more debug output

// =================================================================
// Declare the Firebase Data object in global scope
//Define FirebaseESP32 data objects
FirebaseData firebaseData1;

String device_ip;
String sw_version = "1.0.0";
int device_number = 1;
location_t dv_location;
String path_dv_staus;
String path_tx_usage;
String path_pay_request;
int display_mode = 0;
int prev_display;

// ==================== OTA web upload ================================= //
#ifdef USE_ARDUINO_OTA

#ifdef ESP8266
ESP8266WebServer server(80);
#elif ESP32
WebServer server(80);
#endif
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
  "<h1>EVT Login</h1>"
  "<input name=userid placeholder='User ID'> "
  "<input name=pwd placeholder=Password type=Password> "
  "<input type=submit onclick=check(this.form) class=btn value=Login></form>"
  "<script>"
  "function check(form) {"
  "if(form.userid.value=='admin' && form.pwd.value=='P@ssw0rd')"
  "{window.open('/serverIndex')}"
  "else"
  "{alert('Error Password or Username')}"
  "}"
  "</script>" + style;

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
  "</script>" + style;
#endif


void refreshDateTime();
void refreshLocations();

uint32_t mLastTime = 0;
uint32_t mTimeSeconds = 0;

int wificounter;

/*
  IPAddress local_IP(172, 20, 10, 5);
  IPAddress gateway(172, 20, 10, 1);
  IPAddress subnet(255, 255, 255, 240);
  IPAddress primaryDNS(172, 20, 10, 1); //optional
  IPAddress secondaryDNS(8, 8, 8, 8); //optional

  IPAddress local_IP(192, 168, 1, 34);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress primaryDNS(192, 168, 1, 1); //optional
  IPAddress secondaryDNS(8, 8, 8, 8);   //optional
*/

void setup_wifi()
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  if (WiFi.status() == WL_CONNECTED)
  {
    WiFi.disconnect();
  }

  //WiFi.config(local_IP);
  if (WiFi.status() != WL_CONNECTED)
  {
    //if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    //{
    //  Serial.println("STA Failed to configure");
    //}
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    //delay(1000);
  }
  wificounter = 0;
  while (WiFi.status() != WL_CONNECTED && wificounter < 50)
  {
    for (int i = 0; i < 500; i++)
    {
      delay(1);
    }
    Serial.print(".");
    wificounter++;
  }

  if (wificounter >= 50) {
    Serial.println("Restarting ...");
    ESP.restart();
  }
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println("");
  Serial.println("WiFi connected.");
  printCurrentNet();
  printWiFiData();
}

int pingResult;

int state = 0;
int counter = 0;
int someNumLoops = 1000;

#ifndef LED_BUILTIN
#define LED_BUILTIN 13 // pin number is specific to your esp32 board
#endif

/* setup function */
void setup(void) {

  Serial.begin(115200);
  //Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Test sample RESI API");

  // We start by connecting to a WiFi network
  setup_wifi();

  // test ping google
  bool success = Ping.ping("www.google.com", 3);
  if (!success)
  {
    Serial.println("Ping failed");
    return;
  }
  Serial.println("Ping succesful.");

#ifdef USE_ARDUINO_OTA
  //use mdns for host name resolution//
  if (!MDNS.begin(HOST_NAME))
    //if (!MDNS.begin("http:/192.168.1.199.local"))
  { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
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
    ESP.restart();
  }, []() {
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
    }
  });
  server.begin();
#endif

  // ========= Initialize RemoteDebug
  Debug.begin(HOST_NAME);         // Initialize the WiFi server
  Debug.setResetCmdEnabled(true); // Enable the reset command
  Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
  Debug.showColors(true); // Colors
  // =========================================
  // Initial code here
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
  path_dv_staus = DV_STATUS + String("/evt_bus_") +
                  String(deviceBuf[0]) + String(deviceBuf[1]) +
                  String(deviceBuf[2]) + String(deviceBuf[3]);

  refreshDateTime();
  refreshLocations();

  if (db.sentResponseDeviceStatus(firebaseData1, path_dv_staus, 1, dv_location, device_ip, formattedDate, showsDebug) < 0)
  {
    // write logs error
    Serial.println("sentResponseDeviceStatus failed");
  }

  Serial.println("Start Initial LCD ");
  screen.initializeLcd();
  Serial.println("End Initial LCD ");
  /*
    if ( scbAPI.payment("", "20.00") )
    {
    Serial.println("get a payment sucess");
    }
    else
    {
    Serial.println("get a payment failed");
    }
  */

}

void loop(void) {
#ifdef USE_ARDUINO_OTA
  server.handleClient();
#endif
  // RemoteDebug handle
  Debug.handle();

  if (Debug.isActive(Debug.VERBOSE))
  {
    Debug.printf("set a device IP : %s\n", device_ip.c_str());
  }

  toggle();
  delay(500);
  yield();
}

void toggle()
{
  int reCheckStatus = 0;
  debugD("this is a debug - Led read %d", digitalRead(LED_BUILTIN));
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  Serial.println("Strta toggle get a recheck status");
  reCheckStatus = db.getRecheckStaus(firebaseData1, path_dv_staus, showsDebug);
  debugD("toggle get a recheck status %d", reCheckStatus);

  if (reCheckStatus == 1 )
  {
    refreshDateTime();
    refreshLocations();

    if (db.sentResponseDeviceStatus(firebaseData1, path_dv_staus, 1, dv_location, device_ip, formattedDate, showsDebug) < 0)
    {
      // write logs error
      Serial.println("sentResponseDeviceStatus failed");
    }
  }
  else {
    Serial.println("get a check status = " + String(reCheckStatus));


    debugD("done get a recheck status %d", reCheckStatus);
  }


}

// Function example to show a new auto function name of debug* macros

void foo() {

  uint8_t var = 1;

  debugV("this is a debug - var %u", var);
  debugV("This is a println");
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
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT); // yyyy-mm-dd
  // Extract time
  timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);

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

void printWiFiData()
{
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP address : ");
  Serial.println(ip);

  Serial.print("Subnet mask: ");
  Serial.println((IPAddress)WiFi.subnetMask());

  Serial.print("Gateway IP : ");
  Serial.println((IPAddress)WiFi.gatewayIP());

  Serial.print("DNS: ");
  Serial.println((IPAddress)WiFi.dnsIP());

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.println(rssi);
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}
