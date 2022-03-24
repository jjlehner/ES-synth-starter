#include "ThreadSafeArray.hpp"

//
// Created by Neel on 11/03/2022.
//

ThreadSafeArray::ThreadSafeArray() = default;

void ThreadSafeArray::initMutex() {
    keyArrayMutex = xSemaphoreCreateMutex();
}

uint16_t ThreadSafeArray::read() {
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    uint16_t comb = (buff >> 12).to_ulong();
    xSemaphoreGive(keyArrayMutex);
    return comb;
}

void ThreadSafeArray::write(const std::bitset<NUMBER_OF_INPUTS> &keyArray) {
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    buff = keyArray;
    xSemaphoreGive(keyArrayMutex);
}

std::array<SwitchStateChange, 12>
ThreadSafeArray::findKeyStateChanges(const std::bitset<NUMBER_OF_INPUTS> &newKeyArray) {
    std::array<SwitchStateChange, 12> keyStateChanges;
    keyStateChanges.fill(SwitchStateChange::NO_CHANGE);
    auto releasedKeys = (newKeyArray & ~buff) >> (NUMBER_OF_INPUTS-12);
    auto pressedKeys = (~newKeyArray & buff) >> (NUMBER_OF_INPUTS -12);
    for (int i = 0; i < 12; i++) {
        keyStateChanges[i] = releasedKeys[i] ? SwitchStateChange::RELEASED : keyStateChanges[i];
        keyStateChanges[i] = pressedKeys[i] ? SwitchStateChange::PRESSED : keyStateChanges[i];
    }
    return keyStateChanges;
}

