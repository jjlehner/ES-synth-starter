//
// Created by Neel on 11/03/2022.
//

#ifndef ES_SYNTH_STARTER_KNOBS_HPP
#define ES_SYNTH_STARTER_KNOBS_HPP

#include <cstdint>
#include <utility>
#include <array>

enum class RotationDist : int8_t {
    Negative = -1,
    Positive = +1,
    NoChange = 0,
    Impossible = 127,
};

class Knobs {
private:
    bool prevA;
    bool prevB;
    int oldChange;

    //Where row represents previous B,A and column represents current B,A
    static constexpr const RotationDist rotationDist[4][4] = {
            {RotationDist::NoChange,   RotationDist::Positive,   RotationDist::Negative,   RotationDist::Impossible},
            {RotationDist::Negative,   RotationDist::NoChange,   RotationDist::Impossible, RotationDist::Positive},
            {RotationDist::Positive,   RotationDist::Impossible, RotationDist::NoChange,   RotationDist::Negative},
            {RotationDist::Impossible, RotationDist::Negative,   RotationDist::Positive,   RotationDist::NoChange}
    };
public:
    static inline std::pair<bool, bool> getAB(const std::array<uint8_t, 7> &keyArray, size_t idx) {
        bool A3 = keyArray[idx] >> 3;
        bool B3 = (keyArray[idx] & 0x4) >> 2;
        return std::make_pair(A3, B3);
    }

    int getChange(bool newA, bool newB) {
        int rowIndex = ((uint8_t) prevB << 1) + prevA;
        int colIndex = ((uint8_t) newB << 1) + newA;

        switch (rotationDist[rowIndex][colIndex]) {
            case RotationDist::Positive:
                oldChange = 1;
                break;
            case RotationDist::Negative:
                oldChange = -1;
                break;
            case RotationDist::NoChange:
//                break;
            case RotationDist::Impossible:
                break;
        }

        prevA = newA;
        prevB = newB;
        return oldChange;
    }
};

constexpr const RotationDist Knobs::rotationDist[4][4];
#endif //ES_SYNTH_STARTER_KNOBS_HPP
