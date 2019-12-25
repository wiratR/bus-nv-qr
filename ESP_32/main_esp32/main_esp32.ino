//====================================================================================
//                                  Libraries
//====================================================================================
// Call up the TFT library
#include <TFT_eSPI.h> // Hardware-specific library for ESP8266
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FirebaseESP32.h>
#include <time.h>

#define FIREBASE_HOST   "qr-evt-db.firebaseio.com"
#define FIREBASE_AUTH   "LCX6Yyh4A9wuURogU0fhN03MbsfvWiRF2Z9iSl3z"
//#define WIFI_SSID       "Vix_Guest"
//#define WIFI_PASSWORD   "Committed-Honest-Passionate"
//#define WIFI_SSID       "Bon iPhone"
//#define WIFI_PASSWORD   "1234BAll"
#define WIFI_SSID       "Bonny_2G_e00"
#define WIFI_PASSWORD   "1234BAll"

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// unwiredlabs Hostname & Geolocation Endpoint url
// refer :: https : //circuitdigest.com/microcontroller-projects/how-to-track-location-with-nodemcu-using-google-map-api
#include <WifiLocation.h>
const char *googleApiKey = "AIzaSyDTuLll4kYce8bLICQ6-dL81QIGPsk3kRQ"; //" your api key ";
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

#define WIFI_CONNTION_TIMEOUT   20

int more_text = 1; // set to 1 for more debug output

// =================================================================
// Declare the Firebase Data object in global scope
//Define FirebaseESP32 data objects
FirebaseData firebaseData1;         // FirebaseData for "/dv_status"
FirebaseData firebaseData2;         // FirebaseDate for "/tx_usage"

String device_ip;
String sw_version = "1.0.0";
int device_number = 1;
location_t dv_location;

String path_dv_staus;

// SDCard should be add new PIN for SD_CS
int display_mode = 0;

String status_message = "";
#define STATUS_REQUEST  "200"
#define STAUTS_TIMEOUT  "300"
#define STATUS_FAILED   "400"

// =================================================================

void printResult(FirebaseData &data);
//sent response device status to firebase
int sentResponseDeviceStatus(int status_mode);
void refreshDateTime();
void refreshLocations();

void setup()
{
    int count = 0;
    // put your setup code here, to run once:
    // =============================================================
    // Hardware initail
    // =============================================================
    Serial.begin(115200);
    // =============================================================
    // WiFi initialised
    // =============================================================
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
        //if ( count += WIFI_CONNTION_TIMEOUT )
        //    break;
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    device_ip = IpAddress2String(WiFi.localIP());
    // Initialize a NTPClient to get time
    timeClient.begin();
    // Set offset time in seconds to adjust for your timezone, for example:
    timeClient.setTimeOffset(GMT_7_OFFSET);

    dv_location = location.getGeoFromWiFi();
    Serial.println("Location request data");
    // ======================================================
    // Firebase initialised
    // ======================================================
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
    // =====================================================
    // Sent Device status
    // =====================================================
    char deviceBuf[4];
    sprintf(deviceBuf, "%04X", device_number); // convert to Hex 4 digits Eg. 1 --> 0001 , 10 ---> 000A
    path_dv_staus = DV_STATUS + String("/evt_bus_") + \
    String(deviceBuf[0]) + String(deviceBuf[1]) + \
    String(deviceBuf[2]) + String(deviceBuf[3]);

    refreshDateTime(); 
    refreshLocations();

    if (sentResponseDeviceStatus( 1 ) < 0)
    {
        // write logs error
        Serial.println("sentResponseDeviceStatus failed");
        display_mode = 4; // out of Service
    }
    else
    {
        display_mode = 1; // welcome screen
        //refreshDateTime();
        //refreshLocations();
        // test sent Payment after reset
        sentPay("123456789012",2000,1);
    }
    delay(300);
    
}


// ======================================================== 
// ============== Main Loop ===============================
// ========================================================
void loop()
{    
    // standy by mode read 'recheck' status on 'dv_status/evt_bus_ddd/recheck is (1)
    // will resent sentResponseDeviceStatus
    // if found active form QR-device will reade RXTX and sent requests to Firebase 
    // on node 'txn_usage/TP_yyyymmdd_hhmmss_dddd.dat/
    // call sentPay(String PaymentCode, int PaymentValue)
    while( 1 )
    {
        Serial.println("loop() : check status");
        if ( getRecheckStaus() == 1 )
        {
            Serial.println("loop() : get recheck from RDS");
            refreshDateTime();
            refreshLocations();
            delay(100);
            if (sentResponseDeviceStatus(1) < 0)
            {
                // write logs error
                Serial.println("sentResponseDeviceStatus failed");
                display_mode = 4; // out of Service
            }
            else
            {
                display_mode = 1; // welcome screen
            }
        }
        delay(1000);
    }
    // ============ shows outOf Service ======================= //
    // ============ need reboots ============================== //
    // ======================================================== //


}

// =========================================================
// this function convert IPAddress to String IpAddress2String
// =========================================================
String IpAddress2String(IPAddress ipAddress) 
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
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
    dayStamp = formattedDate.substring(0, splitT);          // yyyy-mm-dd
    // Extract time
    timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1); // 
//    Serial.println(timeStamp);

    if (more_text)
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
    Serial.println(location.getSurroundingWiFiJson());
    if(more_text)
    {
        Serial.println("Latitude: " + String(dv_location.lat, 7));
        Serial.println("Longitude: " + String(dv_location.lon, 7));
        Serial.println("Accuracy: " + String(dv_location.accuracy));
    }
}
// ============================================================
// this function for find location code form Firebase 
// =============================================================
int getLocationCode(double location_x,double location_y)
{
    // not yet implement
    return -1;
}

String convertStrToFileName( String strIn, int type )
{
    String strOut = "";
    strOut = strIn;

    if (type == 1)
        // convert 'yyyyy-mm-dd' to 'yyyymmdd'
        strOut.replace("-", "");
    if (type == 2)
        // convert 'hh:mm:ss' to 'hhmmss'
        strOut.replace(":","");

    return strOut;
}


