//
// Created by Neel on 16/03/2022.
//

#ifndef ES_SYNTH_STARTER_THREADSAFELIST_HPP
#define ES_SYNTH_STARTER_THREADSAFELIST_HPP

#include <list>
#include <array>
#include <algorithm>
#include "STM32FreeRTOS.h"

template<typename T>
class ThreadSafeList {
private:
    std::list<T> list;
    constexpr static size_t MAX_ISR_READ = 8;
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
        std::array<T, MAX_ISR_READ> readISR;
        size_t i = 0;
        UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
        for(T t : list){
            readISR[i++] = t;
            if(i==8){
                break;
            }
        }
        taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
        return std::pair<std::array<T, MAX_ISR_READ>, size_t>(readISR, i);
    }

    void clear(){
        taskENTER_CRITICAL();
        list.clear();
        taskEXIT_CRITICAL();
    }

};

#endif //ES_SYNTH_STARTER_THREADSAFELIST_HPP
