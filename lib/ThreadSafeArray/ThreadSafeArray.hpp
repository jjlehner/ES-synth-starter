//
// Created by Neel on 09/03/2022.
//

#ifndef ES_SYNTH_STARTER_THREADSAFEARRAY_HPP
#define ES_SYNTH_STARTER_THREADSAFEARRAY_HPP

#include "STM32FreeRTOS.h"
#include <bitset>
#include <array>

enum class SwitchStateChange {
    PRESSED, RELEASED, NO_CHANGE
};

class ThreadSafeArray {
public:
    static constexpr size_t NUMBER_OF_INPUTS = 28;
private:
    std::bitset<NUMBER_OF_INPUTS> buff;
    SemaphoreHandle_t keyArrayMutex{};
public:
    ThreadSafeArray();

    // This cannot be initialised in the constructor as xSemaphoreCreateMutex cannot be called in the global scope
    void initMutex();

    void write(const std::bitset<NUMBER_OF_INPUTS> &keyArray);

    std::array<SwitchStateChange, 12> findKeyStateChanges(const std::bitset<NUMBER_OF_INPUTS> &newKeyArray);

    uint16_t read();
};


#endif //ES_SYNTH_STARTER_THREADSAFEARRAY_HPP
