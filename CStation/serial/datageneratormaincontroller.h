#ifndef DATAGENERATORMAINCONTROLLER_H
#define DATAGENERATORMAINCONTROLLER_H

#include "./datagenerator.h"
#include <QStack>

#define MAINCONTROLLER_STATE_BUFFER_SIZE 2

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

    virtual bool isGenerationNeeded(uint32_t current_index);
    virtual void fillNextState(uint32_t full_index, QByteArray* buffer);
    virtual void fillEmptyState(uint32_t full_index, QByteArray* buffer);

    void appendNextCommand(uint8_t cmd, uint32_t param0, uint8_t param1, uint8_t param2, uint8_t param3);
private:
    QStack<MainControllerState> next_states;
    MainControllerState getNextState(uint32_t full_index);
    MainControllerState getEmptyState(uint32_t full_index);
};

#endif // DATAGENERATORMAINCONTROLLER_H
