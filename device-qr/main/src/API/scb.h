#ifndef SCB_H
#define SCB_H



#include "Arduino.h"
#include "ArduinoJson.h"
// https://arduinojson.org/v6/assistant/

#ifdef ESP8266
    #include <ESP8266WiFi.h>
#else
    #include <WiFi.h>
#endif

#include <HTTPClient.h>
#include "../utils/ESPTrueRandom.h"
#include "./scbConfig.h"

class scb
{
    public:
        int payment(String qrData, String amount);

    private:
        byte uuidNumber[16];
        int getAccessToken(String *accessToken);
};

#endif
