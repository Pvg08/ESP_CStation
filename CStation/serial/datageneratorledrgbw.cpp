#include "datageneratorledrgbw.h"

DataGeneratorLEDRGBW::DataGeneratorLEDRGBW() : DataGenerator()
{
    state_size = sizeof(LEDRGBWState);
    state_count = RGBW_STATE_BUFFER_SIZE;
}

void DataGeneratorLEDRGBW::fillNextState(uint32_t full_index, QByteArray *buffer)
{
    LEDRGBWState state = getNextState(full_index);
    buffer->replace(0, buffer->size(), (char*)(void*)&state, sizeof(state));
}

void DataGeneratorLEDRGBW::fillEmptyState(uint32_t full_index, QByteArray *buffer)
{
    LEDRGBWState state = getEmptyState(full_index);
    buffer->replace(0, buffer->size(), (char*)(void*)&state, sizeof(state));
}

LEDRGBWState DataGeneratorLEDRGBW::getNextState(uint32_t full_index)
{
    LEDRGBWState state;

    float findex = fabs(sin(full_index / 100.0));

    state.state_index = full_index;
    state.r = findex * 100 + (1-findex) * 0;
    state.g = findex * 33 + (1-findex) * 76;
    state.b = findex * 0 + (1-findex) * 255;
    state.w = findex * 255 + (1-findex) * 10;

    if (full_index % 5 == 0) {
        state.r = 255;
        state.w = 255;
    }

    state.timeout = base_timeout;
    state.command = CMD_PLAY;
    state.hash = 0;
    state.hash = getHash((void*) &state);

    return state;
}

LEDRGBWState DataGeneratorLEDRGBW::getEmptyState(uint32_t full_index)
{
    LEDRGBWState state;

    state.r = state.g = state.b = state.w = 0;

    state.timeout = base_timeout;
    state.state_index = full_index;
    state.command = CMD_PLAY;
    state.hash = 0;
    state.hash = getHash((void*) &state);

    return state;
}
