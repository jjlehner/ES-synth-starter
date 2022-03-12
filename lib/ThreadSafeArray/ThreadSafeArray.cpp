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
    uint16_t comb = (buff >> 12).to_ulong();
    xSemaphoreGive(keyArrayMutex);
    return comb;
}

void ThreadSafeArray::write(const std::bitset<24> &keyArray) {
    xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
    buff = keyArray;
    xSemaphoreGive(keyArrayMutex);
}

