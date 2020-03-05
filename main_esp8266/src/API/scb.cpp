#include "Arduino.h"
#include "scb.h"

bool scb::payment(String qrData, String amount)
{
    String authToken = "";
    bool isValid = false;

    // Generate a new UUID
    ESPTrueRandom.uuid(uuidNumber);
    String uuidStr = ESPTrueRandom.uuidToString(uuidNumber);

    // start get AccessToken from
    Serial.println("payment() : Data       = " + qrData);
    Serial.println("payment() : txn Amount = " + amount);

    if (!getAccessToken(&authToken) )
    {
        Serial.println("payment() : getAccessToken failed");
        return isValid;
    }

    Serial.println("payment() : get a accessToken  = " + authToken);

    if (WiFi.status() == WL_CONNECTED)
    { //Check WiFi connection status
        HTTPClient http;
        String postMessage;

        const size_t capacity = JSON_OBJECT_SIZE(9);
        DynamicJsonDocument doc(capacity);
        doc["qrData"]               = qrData;
        doc["payeeTerminalNo"]      = SCB_TERMINAL_ID;
        doc["payeeBillerId"]        = SCB_MERCHARD_ID;
        doc["transactionAmount"]    = "20.00";
        doc["transactionCurrency"]  = "THB";
        doc["reference1"]           = "12345671234";
        doc["reference2"]           = "2";
        doc["reference3"]           = "SCB01234566789";
        doc["partnerTransactionId"] = "165134740692203LJPXEBK8TNUIJVPHH9PM";
        //serializeJson(doc, Serial);
        serializeJson(doc, postMessage);

        http.begin("https://api-sandbox.partners.scb/partners/sandbox/v1/payment/merchant/rtp/confirm"); //Specify destination for HTTP request
        http.addHeader("Content-Type",      "application/json");
        http.addHeader("accept-language",   "EN");
        http.addHeader("authorization",     "Bearer " +authToken);
        http.addHeader("requestUId",        uuidStr);
        http.addHeader("resourceOwnerId",   SCB_API_SERECT);

        int httpCode = http.POST(postMessage);

        if (httpCode > 0)
        {
            String JsonResponse = http.getString();
            Serial.println("");
            Serial.println("payment() : HTTP code = " + String(httpCode));
            Serial.println("payment() : response  = " + JsonResponse);
            if (httpCode == 200)
            {
                const size_t capacity = 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(16) + 630;
                DynamicJsonDocument doc(capacity);
                //const char *json = "{\"status\":{\"code\":1000,\"description\":\"Success\"},\"data\":{\"originalTransaction\":{\"messageIdentification\":\"\",\"messageNameIdentification\":\"\",\"paymentInformationIdentification\":\"\"},\"partnerTransactionId\":\"102376737772201H819JM9Z1JI22I90918F\",\"transactionId\":\"67bb8b8b-e205-440d-8a3a-f3a1b1668552\",\"transactionDateTime\":\"2019-06-01T09:00:00.000+07:00\",\"exchangeRate\":\"\",\"transactionAmount\":\"1500.00\",\"transactionCurrency\":\"THB\",\"equivalenceTransactionAmount\":\"\",\"equivalenceTransactionCurrency\":\"\",\"payerBankCode\":\"014\",\"payerTepaCode\":\"140\",\"billerId\":\"123456789012345\",\"reference1\":\"12345678901234567890\",\"reference2\":\"\",\"reference3\":\"SCB01061900001\",\"slipId\":\"\"}}";
                deserializeJson(doc, JsonResponse);
                int status_code                 = doc["status"]["code"];                       // 1000
                const char *status_description  = doc["status"]["description"]; // "Success"

                if ( status_code == 1000) // sucess
                {
                    isValid = true;
                    JsonObject data = doc["data"];
                    JsonObject data_originalTransaction = data["originalTransaction"];
                    const char *data_originalTransaction_messageIdentification = data_originalTransaction["messageIdentification"];                       // ""
                    const char *data_originalTransaction_messageNameIdentification = data_originalTransaction["messageNameIdentification"];               // ""
                    const char *data_originalTransaction_paymentInformationIdentification = data_originalTransaction["paymentInformationIdentification"]; // ""
                    const char *data_partnerTransactionId = data["partnerTransactionId"];                     // "102376737772201H819JM9Z1JI22I90918F"
                    const char *data_transactionId = data["transactionId"];                                   // "67bb8b8b-e205-440d-8a3a-f3a1b1668552"
                    const char *data_transactionDateTime = data["transactionDateTime"];                       // "2019-06-01T09:00:00.000+07:00"
                    const char *data_exchangeRate = data["exchangeRate"];                                     // ""
                    const char *data_transactionAmount = data["transactionAmount"];                           // "1500.00"
                    const char *data_transactionCurrency = data["transactionCurrency"];                       // "THB"
                    const char *data_equivalenceTransactionAmount = data["equivalenceTransactionAmount"];     // ""
                    const char *data_equivalenceTransactionCurrency = data["equivalenceTransactionCurrency"]; // ""
                    const char *data_payerBankCode = data["payerBankCode"];                                   // "014"
                    const char *data_payerTepaCode = data["payerTepaCode"];                                   // "140"
                    const char *data_billerId = data["billerId"];                                             // "123456789012345"
                    const char *data_reference1 = data["reference1"];                                         // "12345678901234567890"
                    const char *data_reference2 = data["reference2"];                                         // ""
                    const char *data_reference3 = data["reference3"];                                         // "SCB01061900001"
                    const char *data_slipId = data["slipId"];                                                 // ""
                }
                else
                {
                    Serial.println("payment() : error code " + String(status_code));
                    Serial.println("payment() : details "    + String(status_description));
                }
            }
        }
    }
    else
    {
        Serial.println("Error in WiFi connection");
    }

    return isValid;
}

bool scb::getAccessToken(String *accessToken)
{
    bool isValid = false;
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
        //serializeJson(doc, Serial);
        serializeJson(doc, postMessage);

        http.begin("https://api-sandbox.partners.scb/partners/sandbox/v1/oauth/token"); //Specify destination for HTTP request
        http.addHeader("Content-Type"   ,   "application/json");
        http.addHeader("accept-language",   "EN");
        http.addHeader("requestUId"     ,   uuidStr);
        http.addHeader("resourceOwnerId",   SCB_API_SERECT);

        int httpCode = http.POST(postMessage);

        if (httpCode > 0)
        {
            String JsonResponse = http.getString();
            Serial.println("");
            Serial.println("getAccessToken() : HTTP code = " + String(httpCode));
            Serial.println("getAccessToken() : response  = " + JsonResponse);
            if (httpCode == 200)
            {
                const size_t capacity = 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 140;
                DynamicJsonDocument doc(capacity);
                //const char *json = "{\"status\":{\"code\":1000,\"description\":\"Success\"},\"data\":{\"accessToken\":\"0e63894c-dcf5-48eb-b8f5-962a8933642d\",\"tokenType\":\"Bearer\",\"expiresIn\":21600,\"expiresAt\":1580158867}}";
                deserializeJson(doc, JsonResponse);
                int status_code                = doc["status"]["code"];         // 1000
                const char *status_description = doc["status"]["description"];  // "Success"
                
                if( status_code == 1000)
                {
                    JsonObject data             = doc["data"];
                    String data_accessToken     = data["accessToken"];  // "0e63894c-dcf5-48eb-b8f5-962a8933642d"
                    const char *data_tokenType  = data["tokenType"];    // "Bearer"
                    int data_expiresIn          = data["expiresIn"];    // 21600
                    long data_expiresAt         = data["expiresAt"];    // 1580158867
                    *accessToken = data_accessToken;
                    isValid = true;
                } 
                else
                {
                    Serial.println("getAccessToken() : error code " + String(status_code));
                    Serial.println("getAccessToken() : details " + String(status_description));
                }
                
            }
            else
            {
                Serial.println("getAccessToken() : http code error = " + httpCode);
            }  
        }

        http.end(); //Free resources
    }
    else
    {
        Serial.println("Error in WiFi connection");
    }

    return isValid;
}