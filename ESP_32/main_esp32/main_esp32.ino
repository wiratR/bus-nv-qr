//====================================================================================
//                                  Libraries
//====================================================================================
// Call up the TFT library
//#include <TFT_eSPI.h> // Hardware-specific library for ESP8266
#include <time.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

#define FIREBASE_HOST   "qr-evt-db.firebaseio.com"
#define FIREBASE_AUTH   "LCX6Yyh4A9wuURogU0fhN03MbsfvWiRF2Z9iSl3z"
#define WIFI_SSID       "Vix_Guest"
#define WIFI_PASSWORD   "Committed-Honest-Passionate"

#define DV_STATUS     "/dv_status"        // Firebase Realtime Database node to store 'dv_status'
#define MAP_LOACTION  "/map_location"     // Firebase Realtime Database node to store 'map_location'
#define TX_USAGE      "/tx_usage"         // Firebase Realtime Database node to store 'tx_usage'

// =================================================================
// Declare the Firebase Data object in global scope
// Define FirebaseESP32 data object
FirebaseData firebaseData;
FirebaseJson json;

char *device_ip;
String sw_version = "1.0.0";
int device_number = 1;

int timezone = 7 * 3600; //set up BKK - TimeZone (GMT +7)
int dst = 0;             //set date swing time
// SDCard should be add new PIN for SD_CS
int display_mode = 0;
// =================================================================

void printResult(FirebaseData &data);
//sent response device status to firebase
int sentResponseDeviceStatus(int deviceId, char *IPAddr, int status_mode);

void setup()
{
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
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    // =============================================================
    // Firebase initialised
    // =============================================================
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.reconnectWiFi(true);
    //Set database read timeout to 1 minute (max 15 minutes)
    Firebase.setReadTimeout(firebaseData, 1000 * 60);
    //tiny, small, medium, large and unlimited.
    //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
    Firebase.setwriteSizeLimit(firebaseData, "tiny");
    // =====================================================
    // Sent Device status
    // =====================================================
    if (sentResponseDeviceStatus(device_number, device_ip, 1) < 0)
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



// ======================================================== 
// ============== Main Loop ===============================
// ========================================================
void loop()
{
    // 
}

// =========================================================
// this function convert IPAddress to char ip2CharArray
// =========================================================
char *ip2CharArray(IPAddress ip)
{
    static char outChar[16];
    sprintf(outChar, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    return outChar;
}

// ============================================================
// 
// =============================================================
