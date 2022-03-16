//
// Created by Neel on 16/03/2022.
//

#include "ThreadSafeList.hpp"

template<class T>
void ThreadSafeList<T>::initMutex() {
    listMutex = xSemaphoreCreateMutex();
}

template<class T>
void ThreadSafeList<T>::push_back(T elem) {
    xSemaphoreTake(listMutex, portMAX_DELAY);
    list.push_back(elem);
    xSemaphoreGive(listMutex);
}

template<class T>
std::list<T> ThreadSafeList<T>::read() {
    xSemaphoreTake(listMutex, portMAX_DELAY);
    std::list<T> copy = list;
    xSemaphoreGive(listMutex);
    return copy;
}