#include "Arduino.h"
#include "scb.h"

int scb::payment(String qrData, String amount)
{
    String authToken = "";
    int responseCode  = 0;
    // start get AccessToken from
    Serial.println("payment() : Data       = " + qrData);
    Serial.println("payment() : txn Amount = " + amount);

    if ( (responseCode = getAccessToken(&authToken) < 0))
    {
        return -1;
    }else
    {
        // http post payment 
        Serial.println("payment() : get code = " + responseCode);
        Serial.println("payment() : get accessToken  = " + authToken);
    }
    
    return 0;
    //return -1;
}


int scb::getAccessToken(String *accessToken)
{
    // Generate a new UUID
    ESPTrueRandom.uuid(uuidNumber);
    String uuidStr = ESPTrueRandom.uuidToString(uuidNumber);

    Serial.println("getAccessToken() : uuid      = " + uuidStr );
    Serial.println("getAccessToken() : apiKey    = " + String(SCB_API_KEY));
    Serial.println("getAccessToken() : apiSecret = " + String(SCB_API_SERECT));

    if (WiFi.status() == WL_CONNECTED)
    { //Check WiFi connection status

        HTTPClient http;
        String postMessage;
        const size_t capacity       = JSON_OBJECT_SIZE(2);
        DynamicJsonDocument doc(capacity);
        doc["applicationKey"]       = SCB_API_KEY; //SCB_API_KEY;
        doc["applicationSecret"]    = SCB_API_SERECT;
        serializeJson(doc, Serial);
        serializeJson(doc, postMessage);

        http.begin("https://api-sandbox.partners.scb/partners/sandbox/v1/oauth/token"); //Specify destination for HTTP request

        http.addHeader("Content-Type"   ,   "application/json");
        http.addHeader("accept-language",   "EN");
        http.addHeader("requestUId"     ,   uuidStr);
        http.addHeader("resourceOwnerId",   SCB_API_SERECT);

        int httpCode = http.POST(postMessage);

        if (httpCode > 0)
        {
            Serial.println();
            Serial.println(httpCode);
            if (httpCode == 200)
            {
                Serial.println("Hooray!");
/*
                const size_t capacity = 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 140;
                DynamicJsonDocument doc(capacity);

                const char *json = "{\"status\":{\"code\":1000,\"description\":\"Success\"},\"data\":{\"accessToken\":\"0e63894c-dcf5-48eb-b8f5-962a8933642d\",\"tokenType\":\"Bearer\",\"expiresIn\":21600,\"expiresAt\":1580158867}}";

                deserializeJson(doc, json);

                int status_code = doc["status"]["code"];                       // 1000
                const char *status_description = doc["status"]["description"]; // "Success"

                JsonObject data = doc["data"];
                const char *data_accessToken = data["accessToken"]; // "0e63894c-dcf5-48eb-b8f5-962a8933642d"
                const char *data_tokenType = data["tokenType"];     // "Bearer"
                int data_expiresIn = data["expiresIn"];             // 21600
                long data_expiresAt = data["expiresAt"];            // 1580158867
*/
            }
        }

        http.end(); //Free resources
        return httpCode;
    }
    else
    {
        Serial.println("Error in WiFi connection");
        return -1;
    }
}