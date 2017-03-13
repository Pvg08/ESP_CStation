#ifndef DATAGENERATORSERVOLASER_H
#define DATAGENERATORSERVOLASER_H

#include "./datagenerator.h"

#define SERVOLASER_STATE_BUFFER_SIZE 2

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(push, 1)
#endif

struct ServoLaserState : StateStruct {
    uint8_t x __PACKED;
    uint8_t y __PACKED;
    uint8_t laser __PACKED;
} __PACKED;

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(pop)
#endif

class DataGeneratorServoLaser : public DataGenerator
{
public:
    DataGeneratorServoLaser();

    virtual void fillNextState(uint32_t full_index, QByteArray* buffer);
    virtual void fillEmptyState(uint32_t full_index, QByteArray* buffer);
private:
    ServoLaserState getNextState(uint32_t full_index);
    ServoLaserState getEmptyState(uint32_t full_index);
};

#endif // DATAGENERATORSERVOLASER_H
