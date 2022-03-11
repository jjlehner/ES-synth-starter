//
// Created by Neel on 11/03/2022.
//

#include "ClampedCounter.hpp"

ClampedCounter::ClampedCounter() : internalCounter(8) {}

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
