//
// Created by Neel on 11/03/2022.
//

#ifndef ES_SYNTH_STARTER_VOLUMECOUNTER_HPP
#define ES_SYNTH_STARTER_VOLUMECOUNTER_HPP

#include <cstddef>

class ClampedCounter {
private:
    size_t internalCounter;
public:
    ClampedCounter();
    void increment();
    void decrement();
    size_t getInternalCounter();
};


#endif //ES_SYNTH_STARTER_VOLUMECOUNTER_HPP
