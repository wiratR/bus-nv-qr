#ifndef ESPTrueRandom_h
#define ESPTrueRandom_h

#ifdef ESP32
#define RANDOM_REG32 *((volatile uint32_t *)(0x3FF75144))
#endif

#include <Arduino.h>
#include <inttypes.h>

class ESPTrueRandomClass
{
public:
    ICACHE_FLASH_ATTR ESPTrueRandomClass();
    ICACHE_FLASH_ATTR int rand();
    ICACHE_FLASH_ATTR long random();
    ICACHE_FLASH_ATTR long random(long howBig);
    ICACHE_FLASH_ATTR long random(long howsmall, long how);
    ICACHE_FLASH_ATTR int randomBit(void);
    ICACHE_FLASH_ATTR char randomByte(void);
    ICACHE_FLASH_ATTR void memfill(char *location, int size);
    ICACHE_FLASH_ATTR void mac(uint8_t *macLocation);
    ICACHE_FLASH_ATTR void uuid(uint8_t *uuidLocation);
    ICACHE_FLASH_ATTR String uuidToString(uint8_t *uuidLocation);
    bool useRNG;

private:
    unsigned long lastYield;
    ICACHE_FLASH_ATTR int randomBitRaw(void);
    ICACHE_FLASH_ATTR int randomBitRaw2(void);
};
extern ESPTrueRandomClass ESPTrueRandom;
#endif