#ifndef DATAGENERATORLEDRING_H
#define DATAGENERATORLEDRING_H

#include "./datagenerator.h"

#define LEDRING_PIXELS 24
#define LEDRING_PIXELS_STATE_BUFFER_SIZE 4

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(push, 1)
#endif

#ifndef RGBPIXEL_STRUCT
struct RGBPixel {
  uint8_t r __PACKED;
  uint8_t g __PACKED;
  uint8_t b __PACKED;
} __PACKED;
#define RGBPIXEL_STRUCT
#endif

struct LEDRingState : StateStruct {
    RGBPixel state[LEDRING_PIXELS] __PACKED;
} __PACKED;

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(pop)
#endif

class DataGeneratorLEDRing : public DataGenerator
{
public:
    DataGeneratorLEDRing();

    virtual void fillNextState(uint32_t full_index, QByteArray* buffer);
    virtual void fillEmptyState(uint32_t full_index, QByteArray* buffer);
private:
    LEDRingState getNextState(uint32_t full_index);
    LEDRingState getEmptyState(uint32_t full_index);

    RGBPixel Color(uint8_t r, uint8_t g, uint8_t b);
};

#endif // DATAGENERATORLEDRING_H
