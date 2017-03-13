#include "datagenerator.h"

DataGenerator::DataGenerator()
{
    state_size = sizeof(StateStruct);
    state_count = 2;
    base_timeout = 0;
}

uint16_t DataGenerator::getDataSize()
{
    return state_size;
}

uint16_t DataGenerator::getBufferSize()
{
    return state_count;
}

void DataGenerator::fillNextState(uint32_t full_index, QByteArray *buffer)
{
    return fillEmptyState(full_index, buffer);
}

void DataGenerator::fillEmptyState(uint32_t full_index, QByteArray *buffer)
{
    StateStruct state = {0};
    state.state_index = full_index;
    state.timeout = base_timeout;
    buffer->replace(0, buffer->size(), (char*)(void*)&state, sizeof(state));
}

uint32_t DataGenerator::getBaseTimeout() const
{
    return base_timeout;
}

void DataGenerator::setBaseTimeout(const uint32_t &value)
{
    base_timeout = value;
}

uint16_t DataGenerator::getHash(void *state)
{
    return crc.XModemCrc((uint8_t*) state, 0, state_size);
}
