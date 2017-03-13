#include "datageneratorservolaser.h"

DataGeneratorServoLaser::DataGeneratorServoLaser() : DataGenerator()
{
    state_size = sizeof(ServoLaserState);
    state_count = SERVOLASER_STATE_BUFFER_SIZE;
}

void DataGeneratorServoLaser::fillNextState(uint32_t full_index, QByteArray *buffer)
{
    ServoLaserState state = getNextState(full_index);
    buffer->replace(0, buffer->size(), (char*)(void*)&state, sizeof(state));
}

void DataGeneratorServoLaser::fillEmptyState(uint32_t full_index, QByteArray *buffer)
{
    ServoLaserState state = getEmptyState(full_index);
    buffer->replace(0, buffer->size(), (char*)(void*)&state, sizeof(state));
}

ServoLaserState DataGeneratorServoLaser::getNextState(uint32_t full_index)
{
    ServoLaserState state;

    float findex = abs(sin(full_index / 30.0));

    state.state_index = full_index;

    state.x = abs(sin(full_index / 30.0)*180);
    state.y = abs(sin(full_index / 11.0)*180);

    state.laser = findex>0.92 ? 255 : 0;

    state.timeout = base_timeout;
    state.command = CMD_PLAY;
    state.hash = 0;
    state.hash = getHash((void*) &state);

    return state;
}

ServoLaserState DataGeneratorServoLaser::getEmptyState(uint32_t full_index)
{
    ServoLaserState state;

    state.x = state.y = state.laser = 0;

    state.timeout = base_timeout;
    state.state_index = full_index;
    state.command = CMD_PLAY;
    state.hash = 0;
    state.hash = getHash((void*) &state);

    return state;
}
