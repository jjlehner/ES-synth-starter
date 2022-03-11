//
// Created by Neel on 11/03/2022.
//

#include "Knobs.hpp"

Knobs::Knobs() : timeSinceDirStart(millis()) {}

int Knobs::getChange(bool newA, bool newB) {
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

constexpr const RotationDist Knobs::rotationDist[4][4];
