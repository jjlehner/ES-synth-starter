//
// Created by Neel on 16/03/2022.
//

#ifndef ES_SYNTH_STARTER_THREADSAFELIST_HPP
#define ES_SYNTH_STARTER_THREADSAFELIST_HPP

#include <list>
#include <algorithm>
#include "STM32FreeRTOS.h"

template<typename T>
class ThreadSafeList {
private:
    std::list<T> list;
    constexpr static size_t MAX_ISR_READ = 8;
    std::array<T, MAX_ISR_READ> readISR;
public:
    ThreadSafeList() = default;
    void push_back(T elem) {
        taskENTER_CRITICAL();
        
        list.push_back(elem);
        
        taskEXIT_CRITICAL();
    }

    void remove(T val) {
        taskENTER_CRITICAL();
        
        list.remove(val);
        
        taskEXIT_CRITICAL();
    }

    std::pair<T, bool> find(const T& val){
        taskENTER_CRITICAL();

        auto it = std::find(list.begin(), list.end(), val);
        auto returnVal = it != list.end() ? std::pair<T, bool>(*it, true) : std::pair<T, bool>(*list.begin(), false);
        taskEXIT_CRITICAL();
        return returnVal;
    }

    std::pair<std::array<T, 8>, size_t> read() {
        UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
        size_t i = 0;
        for(T t : list){
            readISR[i++] = t;
        }
        taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
        return std::pair<std::array<T, MAX_ISR_READ>, size_t>(readISR, i);
    }
};

#endif //ES_SYNTH_STARTER_THREADSAFELIST_HPP
