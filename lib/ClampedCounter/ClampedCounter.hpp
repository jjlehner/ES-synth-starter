//
// Created by Neel on 11/03/2022.
//

#ifndef ES_SYNTH_STARTER_VOLUMECOUNTER_HPP
#define ES_SYNTH_STARTER_VOLUMECOUNTER_HPP

#include <cstddef>
#include <atomic>

class ClampedCounter {
private:
    std::atomic<size_t> internalCounter;
    size_t lower;
    size_t upper;
public:
    explicit ClampedCounter(size_t _lower = 0, size_t _upper = 16);

    void increment() volatile;

    void decrement() volatile;

    size_t getInternalCounter() volatile;
};


#endif //ES_SYNTH_STARTER_VOLUMECOUNTER_HPP
