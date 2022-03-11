//
// Created by Neel on 11/03/2022.
//

#include "Knobs.hpp"

Knobs::Knobs() : timeSinceDirStart(millis()) {}

void Knobs::updateRotation(bool newA, bool newB) volatile {
    int rowIndex = ((uint8_t) prevB << 1) + prevA;
    int colIndex = ((uint8_t) newB << 1) + newA;
    if(millis() - timeSinceDirStart > epsilon) {
        switch (rotationDist[rowIndex][colIndex]) {
            case RotationDist::Positive:
                oldChange = 1;
                rotation.increment();
                timeSinceDirStart = millis();
                break;
            case RotationDist::Negative:
                oldChange = -1;
                rotation.decrement();
                timeSinceDirStart = millis();
                break;
            case RotationDist::NoChange:
                oldChange = 0;
                break;
            case RotationDist::Impossible:
                if (oldChange > 0) {
                    rotation.increment();
                    rotation.increment();
                } else {
                    rotation.decrement();
                    rotation.decrement();
                }
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
}

size_t Knobs::getRotation() volatile {
    return rotation.getInternalCounter();
}

constexpr const RotationDist Knobs::rotationDist[4][4];
