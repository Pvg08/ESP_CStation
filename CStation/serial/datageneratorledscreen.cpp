#include "datageneratorledscreen.h"

DataGeneratorLEDScreen::DataGeneratorLEDScreen() : DataGenerator()
{
    state_size = sizeof(LEDScreenState);
    state_count = MATRIX_STATE_BUFFER_SIZE;
}

void DataGeneratorLEDScreen::fillNextState(uint32_t full_index, QByteArray *buffer)
{
    LEDScreenState state = getNextState(full_index);
    buffer->replace(0, buffer->size(), (char*)(void*)&state, sizeof(state));
}

void DataGeneratorLEDScreen::fillEmptyState(uint32_t full_index, QByteArray *buffer)
{
    LEDScreenState state = getEmptyState(full_index);
    buffer->replace(0, buffer->size(), (char*)(void*)&state, sizeof(state));
}

LEDScreenState DataGeneratorLEDScreen::getNextState(uint32_t full_index)
{
    LEDScreenState state;

    unsigned bindex = full_index % 1000;

    if (bindex == 0) {
        state = getBrightnessState(full_index, 1);
        return state;
    }
    if (bindex == 100) {
        state = getBrightnessState(full_index, 5);
        return state;
    }
    if (bindex == 300) {
        state = getBrightnessState(full_index, 15);
        return state;
    }
    if (bindex == 600) {
        state = getBrightnessState(full_index, 1);
        return state;
    }
    if (bindex == 900) {
        state = getBrightnessState(full_index, 0);
        return state;
    }

    memset(state.blocks, 0, sizeof(state.blocks));
    state.state_index = full_index;

    unsigned col = full_index % 8;
    full_index /= 8;
    unsigned row = full_index % 8;
    full_index /= 8;
    unsigned page = full_index % 5;

    unsigned i = round(fmax(0, sin((float)state.state_index/10000.0)*320));
    for(; i>0; i--) {
        state.blocks[rand() % 5][rand() % 8] |= (1 << (rand() % 8));
    }

    state.blocks[page][row] |= 1 << col;

    state.timeout = base_timeout;
    state.command = CMD_PLAY;
    state.hash = 0;
    state.hash = getHash((void*) &state);

    return state;
}

LEDScreenState DataGeneratorLEDScreen::getEmptyState(uint32_t full_index)
{
    LEDScreenState state;

    memset(state.blocks, 0, sizeof(state.blocks));

    state.timeout = base_timeout;
    state.state_index = full_index;
    state.command = CMD_PLAY;
    state.hash = 0;
    state.hash = getHash((void*) &state);

    return state;
}

LEDScreenState DataGeneratorLEDScreen::getBrightnessState(uint32_t full_index, uint8_t brightness)
{
    LEDScreenState state;

    memset(state.blocks, 0, sizeof(state.blocks));

    if (brightness>MATRIX_MAX_BRIGHTNESS_LEVEL) brightness = MATRIX_MAX_BRIGHTNESS_LEVEL;

    for(unsigned i=0; i<MATRIX_COUNT; i++) {
        state.blocks[i][0] = brightness;
    }

    state.timeout = base_timeout;
    state.state_index = full_index;
    state.command = CMD_BRIGHTNESS;
    state.hash = 0;
    state.hash = getHash((void*) &state);

    return state;
}
