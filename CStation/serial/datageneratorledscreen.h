#ifndef DATAGENERATORLEDSCREEN_H
#define DATAGENERATORLEDSCREEN_H

#include "./datagenerator.h"

#define CMD_BRIGHTNESS 'B'

#define MATRIX_COUNT 5
#define MATRIX_ROWS_COUNT 8
#define MATRIX_STATE_BUFFER_SIZE 6

#define MATRIX_MAX_BRIGHTNESS_LEVEL 15

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(push, 1)
#endif

typedef uint8_t LEDMatrixState[MATRIX_ROWS_COUNT] __PACKED;
struct LEDScreenState : StateStruct {
    LEDMatrixState blocks[MATRIX_COUNT] __PACKED;
} __PACKED;

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(pop)
#endif

class DataGeneratorLEDScreen : public DataGenerator
{
public:
    DataGeneratorLEDScreen();

    virtual void fillNextState(uint32_t full_index, QByteArray* buffer);
    virtual void fillEmptyState(uint32_t full_index, QByteArray* buffer);
private:
    LEDScreenState getNextState(uint32_t full_index);
    LEDScreenState getEmptyState(uint32_t full_index);
    LEDScreenState getBrightnessState(uint32_t full_index, uint8_t brightness);
};

#endif // DATAGENERATORLEDSCREEN_H
