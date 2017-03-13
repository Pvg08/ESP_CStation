#include "datageneratorledring.h"

DataGeneratorLEDRing::DataGeneratorLEDRing() : DataGenerator()
{
    state_size = sizeof(LEDRingState);
    state_count = LEDRING_PIXELS_STATE_BUFFER_SIZE;
}

void DataGeneratorLEDRing::fillNextState(uint32_t full_index, QByteArray *buffer)
{
    LEDRingState state = getNextState(full_index);
    buffer->replace(0, buffer->size(), (char*)(void*)&state, sizeof(state));
}

void DataGeneratorLEDRing::fillEmptyState(uint32_t full_index, QByteArray *buffer)
{
    LEDRingState state = getEmptyState(full_index);
    buffer->replace(0, buffer->size(), (char*)(void*)&state, sizeof(state));
}

LEDRingState DataGeneratorLEDRing::getNextState(uint32_t full_index)
{
    LEDRingState state;

    memset(state.state, 0, sizeof(state.state));
    state.state_index = full_index;


    for(int i=0; i< LEDRING_PIXELS; i++) {
        state.state[i] = Color(rand() % 50, rand() % 50, rand() % 50);
    }

    state.state[(full_index/3) % LEDRING_PIXELS] = Color(0, 0, 0);
    //state.state[(full_index+12) % LEDRING_PIXELS] = Color(0, 0, 0);
    //state.state[(full_index+18) % LEDRING_PIXELS] = Color(0, 0, 0);
    //state.state[(full_index+6) % LEDRING_PIXELS] = Color(0, 0, 0);

    state.timeout = base_timeout;
    state.command = CMD_PLAY;
    state.hash = 0;
    state.hash = getHash((void*) &state);

    return state;
}

LEDRingState DataGeneratorLEDRing::getEmptyState(uint32_t full_index)
{
    LEDRingState state;

    memset(state.state, 0, sizeof(state.state));

    state.timeout = base_timeout;
    state.state_index = full_index;
    state.command = CMD_PLAY;
    state.hash = 0;
    state.hash = getHash((void*) &state);

    return state;
}

RGBPixel DataGeneratorLEDRing::Color(uint8_t r, uint8_t g, uint8_t b)
{
    RGBPixel color = {r, g, b};
    return color;
}
