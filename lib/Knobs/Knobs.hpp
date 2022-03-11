//
// Created by Neel on 11/03/2022.
//

#ifndef ES_SYNTH_STARTER_KNOBS_HPP
#define ES_SYNTH_STARTER_KNOBS_HPP

#include <cstdint>
#include <utility>
#include <array>
#include <Arduino.h>
enum class RotationDist : int8_t {
    Negative = -1,
    Positive = +1,
    NoChange = 0,
    Impossible = 127,
};

class Knobs {
private:
    bool prevA=false;
    bool prevB=false;
    int oldChange=0;
    int rotation=0;
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
    Knobs() : timeSinceDirStart(millis()) {}

    static inline std::pair<bool, bool> getAB(const std::array<uint8_t, 7> &keyArray, size_t idx) {
        bool A3 = keyArray[idx] >> 3;
        bool B3 = (keyArray[idx] & 0x4) >> 2;
        return std::make_pair(A3, B3);
    }

    int getChange(bool newA, bool newB) {
        int rowIndex = ((uint8_t) prevB << 1) + prevA;
        int colIndex = ((uint8_t) newB << 1) + newA;
        if(millis() - timeSinceDirStart > epsilon) {
            switch (rotationDist[rowIndex][colIndex]) {
                case RotationDist::Positive:
                    oldChange = 1;
                    rotation++;
                    timeSinceDirStart = millis();
                    break;
                case RotationDist::Negative:
                    oldChange = -1;
                    rotation--;
                    timeSinceDirStart = millis();
                    break;
                case RotationDist::NoChange:
                    oldChange = 0;
                    break;
                case RotationDist::Impossible:
                    rotation += 2*oldChange;
                    break;
            }
        }
        else{
            if(rotationDist[rowIndex][colIndex] != RotationDist::NoChange){
                timeSinceDirStart = millis();


            }
        }

        prevA = newA;
        prevB = newB;

        return rotation;
    }
};

constexpr const RotationDist Knobs::rotationDist[4][4];
#endif //ES_SYNTH_STARTER_KNOBS_HPP
