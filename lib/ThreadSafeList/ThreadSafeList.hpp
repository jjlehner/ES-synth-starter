//
// Created by Neel on 16/03/2022.
//

#ifndef ES_SYNTH_STARTER_THREADSAFELIST_HPP
#define ES_SYNTH_STARTER_THREADSAFELIST_HPP

#include <list>
#include "STM32FreeRTOS.h"

template<typename T>
class ThreadSafeList {
private:
    std::list<T> list;
    SemaphoreHandle_t listMutex{};
    constexpr static size_t MAX_ISR_READ = 8;
    std::array<T, MAX_ISR_READ> readISR;
public:
    ThreadSafeList() = default;

    void initMutex() {
        listMutex = xSemaphoreCreateMutex();
    }

    void push_back(T elem) {
        // xSemaphoreTake(listMutex, portMAX_DELAY);
        taskENTER_CRITICAL();
        
        list.push_back(elem);
        
        taskEXIT_CRITICAL();
        // xSemaphoreGive(listMutex);
    }

    void remove(T val) {
        // xSemaphoreTake(listMutex, portMAX_DELAY);
        taskENTER_CRITICAL();
        
        list.remove(val);
        
        taskEXIT_CRITICAL();
        // xSemaphoreGive(listMutex);
    }

    std::pair<std::array<T, 8>, size_t> read() {
        UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
        // xSemaphoreTakeFromISR(listMutex, portMAX_DELAY);
        size_t i = 0;
        for(T t : list){
            readISR[i++] = t;
        }
        // static signed BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        // xHigherPriorityTaskWoken = pdFALSE;
        // xSemaphoreGiveFromISR(listMutex, &xHigherPriorityTaskWoken);
        taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
        return std::pair<std::array<T, MAX_ISR_READ>, size_t>(readISR, i);
    }
};

#endif //ES_SYNTH_STARTER_THREADSAFELIST_HPP
