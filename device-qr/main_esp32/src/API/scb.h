#ifndef SCB_H
#define SCB_H

#include <Arduino.h>
#include "ArduinoJson.h"
// https://arduinojson.org/v6/assistant/

#if defined ESP8266
    #include <ESP8266WiFi.h>
    #include <ESP8266HTTPClient.h>
#elif defined ESP32
    #include <WiFi.h>
    #include <HTTPClient.h>
#endif

#include "../utils/ESPTrueRandom.h"
#include "./scbConfig.h"

class scb
{
    public:
        bool payment(String qrData, String amount);

    private:
        byte uuidNumber[16];
        bool getAccessToken(String *accessToken);
};

#endif
