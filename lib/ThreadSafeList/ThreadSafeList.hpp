//
// Created by Neel on 16/03/2022.
//

#ifndef ES_SYNTH_STARTER_THREADSAFELIST_HPP
#define ES_SYNTH_STARTER_THREADSAFELIST_HPP

#include <list>
#include "STM32FreeRTOS.h"

template <typename T> class ThreadSafeList {
private:
    std::list<T> list;
    SemaphoreHandle_t listMutex{};
public:
    ThreadSafeList()=default;

    void initMutex(){
        listMutex = xSemaphoreCreateMutex();
    }

    void push_back(T elem){
        xSemaphoreTake(listMutex, portMAX_DELAY);
        list.push_back(elem);
        xSemaphoreGive(listMutex);
    }
    void remove(T val){
        xSemaphoreTake(listMutex, portMAX_DELAY);
        list.remove(val);
        xSemaphoreGive(listMutex);
    }
    std::list<T> read(){
        xSemaphoreTake(listMutex, portMAX_DELAY);
        std::list<T> copy = list;
        xSemaphoreGive(listMutex);
        return copy;
    }
};

#endif //ES_SYNTH_STARTER_THREADSAFELIST_HPP
