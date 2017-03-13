#include "datageneratormaincontroller.h"

DataGeneratorMainController::DataGeneratorMainController() : DataGenerator()
{
    state_size = sizeof(MainControllerState);
    state_count = MAINCONTROLLER_STATE_BUFFER_SIZE;
}

bool DataGeneratorMainController::isGenerationNeeded(uint32_t current_index)
{
    return !next_states.isEmpty();
}

void DataGeneratorMainController::fillNextState(uint32_t full_index, QByteArray *buffer)
{
    MainControllerState state = getNextState(full_index);
    buffer->replace(0, buffer->size(), (char*)(void*)&state, sizeof(state));
}

void DataGeneratorMainController::fillEmptyState(uint32_t full_index, QByteArray *buffer)
{
    MainControllerState state = getEmptyState(full_index);
    buffer->replace(0, buffer->size(), (char*)(void*)&state, sizeof(state));
}

void DataGeneratorMainController::appendNextCommand(uint8_t cmd, uint32_t param0, uint8_t param1, uint8_t param2, uint8_t param3)
{
    MainControllerState state;

    state.cmd = cmd;
    state.param0 = param0;
    state.param1 = param1;
    state.param2 = param2;
    state.param3 = param3;

    state.timeout = base_timeout;
    state.state_index = 0;
    state.command = CMD_PLAY;
    state.hash = 0;

    next_states.push(state);
}

MainControllerState DataGeneratorMainController::getNextState(uint32_t full_index)
{
    if (!next_states.isEmpty()) {
        MainControllerState state = next_states.pop();
        state.state_index = full_index;
        state.hash = 0;
        state.hash = getHash((void*) &state);
        return state;
    } else {
        return getEmptyState(full_index);
    }
}

MainControllerState DataGeneratorMainController::getEmptyState(uint32_t full_index)
{
    MainControllerState state;

    state.cmd = state.param0 = state.param1 = state.param2 = state.param3 = 0;

    state.timeout = base_timeout;
    state.state_index = full_index;
    state.command = CMD_PLAY;
    state.hash = 0;
    state.hash = getHash((void*) &state);

    return state;
}
