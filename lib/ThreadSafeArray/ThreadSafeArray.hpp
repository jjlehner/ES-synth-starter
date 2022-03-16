//
// Created by Neel on 09/03/2022.
//

#ifndef ES_SYNTH_STARTER_THREADSAFEARRAY_HPP
#define ES_SYNTH_STARTER_THREADSAFEARRAY_HPP

#include "STM32FreeRTOS.h"
#include <bitset>
#include <array>

enum class KeyStateChange{
    PRESSED, RELEASED, NO_CHANGE
};
class ThreadSafeArray {
private:
    std::bitset<24> buff;
    SemaphoreHandle_t keyArrayMutex{};
public:
    ThreadSafeArray();

    // This cannot be initialised in the constructor as xSemaphoreCreateMutex cannot be called in the global scope
    void initMutex();

    void write(const std::bitset<24> &keyArray);
    std::array<KeyStateChange, 12> findKeyStateChanges(const std::bitset<24> &newKeyArray);

    uint16_t read();
};


#endif //ES_SYNTH_STARTER_THREADSAFEARRAY_HPP
