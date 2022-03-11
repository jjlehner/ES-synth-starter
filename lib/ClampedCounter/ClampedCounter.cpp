//
// Created by Neel on 11/03/2022.
//

#include "ClampedCounter.hpp"

ClampedCounter::ClampedCounter(size_t _lower, size_t _upper) : internalCounter((_upper-_lower)/2), lower(_lower), upper(_upper) {}

void ClampedCounter::increment() volatile {
    if (internalCounter == upper)
        return;
    internalCounter++;
}
void ClampedCounter::decrement() volatile {
    if (internalCounter == 0)
        return;
    internalCounter--;
}

size_t ClampedCounter::getInternalCounter() volatile {
    return internalCounter;
}
