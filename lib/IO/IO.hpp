//
// Created by Jonah Lehner on 18/03/2022.
//

#ifndef ES_SYNTH_STARTER_IO_HPP
#define ES_SYNTH_STARTER_IO_HPP
#include <cstdint>
#include <arduino.h>
extern const int C0_PIN;
extern const int C1_PIN;
extern const int C2_PIN;
extern const int C3_PIN;

extern const int REN_PIN;

extern const int RA0_PIN;
extern const int RA1_PIN;
extern const int RA2_PIN;

struct IO{
    static void setRow(uint8_t rowIdx) {
        digitalWrite(REN_PIN, LOW);
        digitalWrite(RA0_PIN, rowIdx & 0x1);
        digitalWrite(RA1_PIN, rowIdx & 0x2);
        digitalWrite(RA2_PIN, rowIdx & 0x4);
        digitalWrite(REN_PIN, HIGH);
    }

    static bool getMatrixValue(size_t row, size_t col){
        setRow(row);
        switch(col){
            case 0:
                return digitalRead(C0_PIN);
            case 1:
                return digitalRead(C1_PIN);
            case 2:
                return digitalRead(C2_PIN);
            case 3:
                return digitalRead(C3_PIN);
        }
        return false;
    }

    static constexpr size_t FLAT_KEY_MATRIX_LENGTH = 28;

};


#endif //ES_SYNTH_STARTER_IO_HPP
