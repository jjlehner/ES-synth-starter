//
// Created by Neel on 11/03/2022.
//

#include "ClampedCounter.hpp"

ClampedCounter::ClampedCounter(size_t _lower, size_t _upper) : internalCounter((_upper-_lower)/2), lower(_lower), upper(_upper) {}

void ClampedCounter::increment() {
    if (internalCounter == 16)
        return;
    internalCounter++;
}
void ClampedCounter::decrement() {
    if (internalCounter == 0)
        return;
    internalCounter--;
}

size_t ClampedCounter::getInternalCounter() {
    return internalCounter;
}
