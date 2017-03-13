#include "datageneratorledringrgb.h"

DataGeneratorLEDRingRGB::DataGeneratorLEDRingRGB() : DataGenerator()
{
    state_size = sizeof(LEDRingRGBState);
    state_count = LEDRING_RGB_PIXELS_STATE_BUFFER_SIZE;
}

void DataGeneratorLEDRingRGB::fillNextState(uint32_t full_index, QByteArray *buffer)
{
    LEDRingRGBState state = getNextState(full_index);
    buffer->replace(0, buffer->size(), (char*)(void*)&state, sizeof(state));
}

void DataGeneratorLEDRingRGB::fillEmptyState(uint32_t full_index, QByteArray *buffer)
{
    LEDRingRGBState state = getEmptyState(full_index);
    buffer->replace(0, buffer->size(), (char*)(void*)&state, sizeof(state));
}

LEDRingRGBState DataGeneratorLEDRingRGB::getNextState(uint32_t full_index)
{
    LEDRingRGBState state;

    memset(state.ring_state, 0, sizeof(state.ring_state));
    state.state_index = full_index;

    for(int i=0; i< LEDRING_RGB_PIXELS; i++) {
        state.ring_state[i] = Color(rand() % 50, rand() % 50, rand() % 50);
    }

    state.ring_state[(full_index/3) % LEDRING_RGB_PIXELS] = Color(0, 0, 0);
    //state.ring_state[(full_index+12) % LEDRING_PIXELS] = Color(0, 0, 0);
    //state.ring_state[(full_index+18) % LEDRING_PIXELS] = Color(0, 0, 0);
    //state.ring_state[(full_index+6) % LEDRING_PIXELS] = Color(0, 0, 0);

    state.led_state = state.ring_state[0];

    state.timeout = base_timeout;
    state.command = CMD_PLAY;
    state.hash = 0;
    state.hash = getHash((void*) &state);

    return state;
}

LEDRingRGBState DataGeneratorLEDRingRGB::getEmptyState(uint32_t full_index)
{
    LEDRingRGBState state;

    memset(state.ring_state, 0, sizeof(state.ring_state));
    state.led_state = state.ring_state[0];

    state.timeout = base_timeout;
    state.state_index = full_index;
    state.command = CMD_PLAY;
    state.hash = 0;
    state.hash = getHash((void*) &state);

    return state;
}

RGBPixel DataGeneratorLEDRingRGB::Color(uint8_t r, uint8_t g, uint8_t b)
{
    RGBPixel color = {r, g, b};
    return color;
}
