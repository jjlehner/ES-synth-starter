#include "ThreadSafeArray.hpp"
#include "STM32FreeRTOS.h"

//
// Created by Neel on 11/03/2022.
//

ThreadSafeArray::ThreadSafeArray()=default;

void ThreadSafeArray::initMutex() {
    keyArrayMutex = xSemaphoreCreateMutex();
}

uint16_t ThreadSafeArray::read() {
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    uint16_t comb = (buff[0] << 8) + (buff[1] << 4) + buff[2];
    xSemaphoreGive(keyArrayMutex);
    return comb;
}

void ThreadSafeArray::write(const std::array<uint8_t, 7> &keyArray) {
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    buff = keyArray;
    xSemaphoreGive(keyArrayMutex);
}

