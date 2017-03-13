#ifndef DATAGENERATORMAINCONTROLLER_H
#define DATAGENERATORMAINCONTROLLER_H

#include "./datagenerator.h"

#define MAINCONTROLLER_STATE_BUFFER_SIZE 2

/* Data Exchange Params */
#define CMD_CMD_TURNINGON 0x01
#define CMD_CMD_TURNOFFBEGIN 0x03
#define CMD_CMD_TURNOFFREADY 0x04
#define CMD_CMD_TURNOFF 0x05
#define CMD_CMD_SETMODESTATE 0x10
#define CMD_CMD_SETRTCTIME 0x20

#define CMD_MODE_TRACKING 0x11
#define CMD_MODE_INDICATION 0x12
#define CMD_MODE_SILENCE 0x13
#define CMD_MODE_CONTROL 0x14
#define CMD_MODE_SECURITY 0x15
/* /Data Exchange Params */

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(push, 1)
#endif

struct MainControllerState : StateStruct {
    uint8_t cmd __PACKED;
    uint32_t param0 __PACKED;
    uint8_t param1 __PACKED;
    uint8_t param2 __PACKED;
    uint8_t param3 __PACKED;
} __PACKED;

#if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
#pragma pack(pop)
#endif

class DataGeneratorMainController : public DataGenerator
{
public:
    DataGeneratorMainController();

    virtual void fillNextState(uint32_t full_index, QByteArray* buffer);
    virtual void fillEmptyState(uint32_t full_index, QByteArray* buffer);
private:
    MainControllerState getNextState(uint32_t full_index);
    MainControllerState getEmptyState(uint32_t full_index);
};

#endif // DATAGENERATORMAINCONTROLLER_H
