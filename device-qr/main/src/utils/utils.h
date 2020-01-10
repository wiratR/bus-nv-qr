#ifndef UTILS_H
#define UTILS_H

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <TFT_eSPI.h>

class utils
{
    public:
        String IpAddress2String(IPAddress ipAddress);        
        String getTxName(String refPath, int deviceNumber, String datePattern, String timePattern);
        void drawBmp(const char *filename, int16_t x, int16_t y, TFT_eSPI tft);

    private :
        String convertStrToFileName(String strIn, int type);
        uint16_t read16(fs::File &f);
        uint32_t read32(fs::File &f);
};

#endif