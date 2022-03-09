//
// Created by Neel on 09/03/2022.
//

#ifndef ES_SYNTH_STARTER_THREADSAFEARRAY_HPP
#define ES_SYNTH_STARTER_THREADSAFEARRAY_HPP


class ThreadSafeArray {
private:
    std::array<uint8_t,7> buff{};
    SemaphoreHandle_t keyArrayMutex{};
public:
    ThreadSafeArray()=default;
    // This cannot be initialised in the constructor as xSemaphoreCreateMutex cannot be called in the global scope
    void initMutex() {
        keyArrayMutex = xSemaphoreCreateMutex();
    }

    void write(const std::array<uint8_t,7> &keyArray) {
        xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
        buff = keyArray;
        xSemaphoreGive(keyArrayMutex);
    }

    uint16_t read() {
        xSemaphoreTake(keyArrayMutex, portMAX_DELAY);
        uint16_t comb = (buff[0]<<4*2) + (buff[1]<<4*1) + buff[2];
        xSemaphoreGive(keyArrayMutex);
        return comb;
    }
};


#endif //ES_SYNTH_STARTER_THREADSAFEARRAY_HPP
