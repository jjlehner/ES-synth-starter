//
// Created by Neel on 16/03/2022.
//

#ifndef ES_SYNTH_STARTER_THREADSAFELIST_HPP
#define ES_SYNTH_STARTER_THREADSAFELIST_HPP

#include <list>
#include "STM32FreeRTOS.h"

template <class T> class ThreadSafeList {
private:
    std::list<T> list;
    SemaphoreHandle_t listMutex{};
public:
    ThreadSafeList()=default;

    void initMutex();

    void push_back(T elem);

    std::list<T> read();
};

#endif //ES_SYNTH_STARTER_THREADSAFELIST_HPP
