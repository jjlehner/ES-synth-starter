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
    SemaphoreHandle_t listMutex{};
    constexpr static size_t MAX_ISR_READ = 8;
    std::array<T, MAX_ISR_READ> readISR;
public:
    ThreadSafeList() = default;

    void initMutex() {
        listMutex = xSemaphoreCreateMutex();
    }
    static constexpr size_t BUFFER_SIZE = 2;
    std::array<T, BUFFER_SIZE> isrSaveBuffer;
    size_t firstEmptyIndex = 0;
    void push_back(T elem) {
        // xSemaphoreTake(listMutex, portMAX_DELAY);
        taskENTER_CRITICAL();
        
        list.push_back(elem);
        
        taskEXIT_CRITICAL();
        // xSemaphoreGive(listMutex);
    }
    void push_back_ISR(T elem) {
        // xSemaphoreTake(listMutex, portMAX_DELAY);
        UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();

//        isrSaveBuffer[firstEmptyIndex] = elem;
        firstEmptyIndex++;
        firstEmptyIndex = firstEmptyIndex == BUFFER_SIZE ? 0 : firstEmptyIndex;
        taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
        // xSemaphoreGive(listMutex);
    }
    void emptySaveBuffer(){
        taskENTER_CRITICAL();

        while(firstEmptyIndex){
            list.push_back(isrSaveBuffer[firstEmptyIndex--]);
        }
        taskEXIT_CRITICAL();

    }
    void remove(T val) {
        // xSemaphoreTake(listMutex, portMAX_DELAY);
        taskENTER_CRITICAL();
        
        list.remove(val);
        
        taskEXIT_CRITICAL();
        // xSemaphoreGive(listMutex);
    }

    std::pair<T, bool> find(const T& val){
        // xSemaphoreTake(listMutex, portMAX_DELAY);
        taskENTER_CRITICAL();

        auto it = std::find(list.begin(), list.end(), val);
        auto returnVal = it != list.end() ? std::pair<T, bool>(*it, true) : std::pair<T, bool>(*list.begin(), false);
        taskEXIT_CRITICAL();
        // xSemaphoreGive(listMutex);
        return returnVal;
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

    void clear(){
        taskENTER_CRITICAL();
        list.clear();
        taskEXIT_CRITICAL();
    }

    void pop_front(){
        UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
        if(list.size() > 1){
            list.pop_front();
        }
        taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
    }
};

#endif //ES_SYNTH_STARTER_THREADSAFELIST_HPP
