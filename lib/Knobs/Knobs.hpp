//
// Created by Neel on 11/03/2022.
//

#ifndef ES_SYNTH_STARTER_KNOBS_HPP
#define ES_SYNTH_STARTER_KNOBS_HPP

#include <cstdint>
#include <utility>
#include <array>
#include <Arduino.h>
#include <bitset>
#include "ClampedCounter.hpp"
#include "ThreadSafeArray.hpp"

enum class RotationDist : int8_t {
    Negative = -1,
    Positive = +1,
    NoChange = 0,
    Impossible = 127,
};

class Knobs {
private:
    bool prevA = false;
    bool prevB = false;
    int oldChange = 0;
    ClampedCounter rotation;
    unsigned long timeSinceDirStart;
    //Where row represents previous B,A and column represents current B,A
    static constexpr const RotationDist rotationDist[4][4] = {
            {RotationDist::NoChange,   RotationDist::Positive,   RotationDist::Negative,   RotationDist::Impossible},
            {RotationDist::Negative,   RotationDist::NoChange,   RotationDist::Impossible, RotationDist::Positive},
            {RotationDist::Positive,   RotationDist::Impossible, RotationDist::NoChange,   RotationDist::Negative},
            {RotationDist::Impossible, RotationDist::Negative,   RotationDist::Positive,   RotationDist::NoChange}
    };
    static constexpr const unsigned long epsilon = 20;
public:
    static inline std::pair<bool, bool> getAB(const std::bitset<ThreadSafeArray::NUMBER_OF_INPUTS> &keyArray, size_t knobIdx) {
        bool A = keyArray[9 + 2 * knobIdx];
        bool B = keyArray[8 + 2 * knobIdx];
        return std::make_pair(A, B);
    }

    Knobs(size_t min=0, size_t max=16);

    void updateRotation(bool newA, bool newB) volatile;

    size_t getRotation() volatile;
};

#endif //ES_SYNTH_STARTER_KNOBS_HPP
