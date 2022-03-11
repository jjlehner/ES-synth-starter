//
// Created by Neel on 11/03/2022.
//

#ifndef ES_SYNTH_STARTER_VOLUMECOUNTER_HPP
#define ES_SYNTH_STARTER_VOLUMECOUNTER_HPP

#include <cstddef>

class ClampedCounter {
private:
    size_t internalCounter;
    size_t lower;
    size_t upper;
public:
    ClampedCounter(size_t _lower=0, size_t _upper=16);
    void increment();
    void decrement();
    size_t getInternalCounter();
};


#endif //ES_SYNTH_STARTER_VOLUMECOUNTER_HPP
