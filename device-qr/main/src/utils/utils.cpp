#include "Arduino.h"
#include "utils.h"


// =========================================================
// this function convert IPAddress to String IpAddress2String
// =========================================================
String utils::IpAddress2String(IPAddress ipAddress)
{
    return String(ipAddress[0]) + String(".") +
           String(ipAddress[1]) + String(".") +
           String(ipAddress[2]) + String(".") +
           String(ipAddress[3]);
}

String utils::convertStrToFileName(String strIn, int type)
{
    String strOut = "";
    strOut = strIn;

    if (type == 1)
        // convert 'yyyyy-mm-dd' to 'yyyymmdd'
        strOut.replace("-", "");
    if (type == 2)
        // convert 'hh:mm:ss' to 'hhmmss'
        strOut.replace(":", "");

    return strOut;
}

String utils::getTxName(String refPath, int deviceNumber, String datePattern, String timePattern)
{
    char deviceBuf[4];
    sprintf(deviceBuf, "%04X", deviceNumber); // convert to Hex 4 digits Eg. 1 --> 0001 , 10 ---> 000A
    String strDate = convertStrToFileName(datePattern, 1);
    String strTime = convertStrToFileName(timePattern, 2);
    // "/txn_usage/TP_yyyymmdd_hhmmss_dddddd.dat"
    return refPath + String("/TP_") +
                         String(strDate) + "_" + String(strTime) + "_" +
                         String(deviceBuf[0]) + String(deviceBuf[1]) +
                         String(deviceBuf[2]) + String(deviceBuf[3]) + "_dat";
}
// BMP image rendering function
void utils::drawBmp(const char *filename, int16_t x, int16_t y, TFT_eSPI tft)
{

    if ((x >= tft.width()) || (y >= tft.height()))
        return;

    fs::File bmpFS;

    // Open requested file on SD card
    bmpFS = SPIFFS.open(filename, "r");

    if (!bmpFS)
    {
        Serial.println("File not found ");
        return;
    }

    uint32_t seekOffset;
    uint16_t w, h, row, col;
    uint8_t r, g, b;

    uint32_t startTime = millis();

    if (read16(bmpFS) == 0x4D42)
    {
        read32(bmpFS);
        read32(bmpFS);
        seekOffset = read32(bmpFS);
        read32(bmpFS);
        w = read32(bmpFS);
        h = read32(bmpFS);

        if ((read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0))
        {
            y += h - 1;

            bool oldSwapBytes = tft.getSwapBytes();
            tft.setSwapBytes(true);
            bmpFS.seek(seekOffset);

            uint16_t padding = (4 - ((w * 3) & 3)) & 3;
            uint8_t lineBuffer[w * 3 + padding];

            for (row = 0; row < h; row++)
            {

                bmpFS.read(lineBuffer, sizeof(lineBuffer));
                uint8_t *bptr = lineBuffer;
                uint16_t *tptr = (uint16_t *)lineBuffer;
                // Convert 24 to 16 bit colours
                for (uint16_t col = 0; col < w; col++)
                {
                    b = *bptr++;
                    g = *bptr++;
                    r = *bptr++;
                    *tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                }

                // Push the pixel row to screen, pushImage will crop the line if needed
                // y is decremented as the BMP image is drawn bottom up
                tft.pushImage(x, y--, w, 1, (uint16_t *)lineBuffer);
            }
            tft.setSwapBytes(oldSwapBytes);
            Serial.print("Loaded in ");
            Serial.print(millis() - startTime);
            Serial.println(" ms");
        }
        else
            Serial.println("BMP format not recognized.");
    }
    bmpFS.close();
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t utils::read16(fs::File &f)
{
    uint16_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read(); // MSB
    return result;
}

uint32_t utils::read32(fs::File &f)
{
    uint32_t result;
    ((uint8_t *)&result)[0] = f.read(); // LSB
    ((uint8_t *)&result)[1] = f.read();
    ((uint8_t *)&result)[2] = f.read();
    ((uint8_t *)&result)[3] = f.read(); // MSB
    return result;
}
