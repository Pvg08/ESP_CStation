#ifndef DATAGENERATORLEDRGBW_H
#define DATAGENERATORLEDRGBW_H

#include "./datagenerator.h"

#define RGBW_STATE_BUFFER_SIZE 6

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(push, 1)
#endif

struct LEDRGBWState : StateStruct {
    uint8_t r __PACKED;
    uint8_t g __PACKED;
    uint8_t b __PACKED;
    uint8_t w __PACKED;
} __PACKED;

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(pop)
#endif

class DataGeneratorLEDRGBW : public DataGenerator
{
public:
    DataGeneratorLEDRGBW();

    virtual void fillNextState(uint32_t full_index, QByteArray* buffer);
    virtual void fillEmptyState(uint32_t full_index, QByteArray* buffer);
private:
    LEDRGBWState getNextState(uint32_t full_index);
    LEDRGBWState getEmptyState(uint32_t full_index);
};

#endif // DATAGENERATORLEDRGBW_H
