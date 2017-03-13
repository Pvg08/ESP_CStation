#ifndef DATAGENERATOR_H
#define DATAGENERATOR_H

#include <math.h>
#include <QObject>
#include <QByteArray>
#include "Crc16.h"

#define CMD_PLAY 0
#define CMD_NONE 1
#define CMD_RESET 'R'

#if defined(WIN32) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
    #define __PACKED                         /* dummy */
#else
    #define __PACKED __attribute__((packed)) /* gcc packed */
#endif

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(push, 1)
#endif

struct StateStruct {
    uint32_t state_index __PACKED;
    uint32_t timeout __PACKED;
    uint16_t command __PACKED;
    uint16_t hash __PACKED;
} __PACKED;

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(pop)
#endif

class DataGenerator
{
public:
    DataGenerator();
    uint16_t getDataSize();
    uint16_t getBufferSize();

    virtual void fillNextState(uint32_t full_index, QByteArray* buffer);
    virtual void fillEmptyState(uint32_t full_index, QByteArray* buffer);
    uint32_t getBaseTimeout() const;
    void setBaseTimeout(const uint32_t &value);

protected:
    uint16_t state_size;
    uint16_t state_count;
    uint32_t base_timeout;
    Crc16 crc;

    uint16_t getHash(void* state);
};

#endif // DATAGENERATOR_H
