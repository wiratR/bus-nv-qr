#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>               // Hardware-specific library for ESP8266
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FirebaseESP32.h>
#include <time.h>

#include "src/config/projectsKey.h"
#include "src/API/scb.h"
#include "src/utils/utils.h"
#include "src/database/nvapi.h"

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

#define DV_STATUS       "/dv_status"       // Firebase Realtime Database node to store 'dv_status'
#define MAP_LOACTION    "/map_location" // Firebase Realtime Database node to store 'map_location'
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
WebServer server(80);
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

scb   scbAPI;
utils utilsHelper;
nvapi db;

void refreshDateTime();
void refreshLocations();

/* setup function */
void setup(void) {

  Serial.begin(115200);

  //pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Test sample RESI API");
  
  // Connect to WiFi network

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(HOST_NAME))
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

  delay(500);

  if( scbAPI.payment("", "20.00") )
  {
      Serial.println("get a payment sucess");
  }
  else
  {
    Serial.println("get a payment failed");
  }
  delay(500);
}

void loop(void) {
  server.handleClient();
  delay(1);
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