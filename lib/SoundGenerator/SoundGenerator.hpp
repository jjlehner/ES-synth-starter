#ifndef ES_BETTERSOUNDINGKEYS
#define ES_BETTERSOUNDINGKEYS

#include <Arduino.h>
#include "Knob.hpp"
#include "ThreadSafeList.hpp"
#include <list>
#include "CANFrame.hpp"
#define _USE_MATH_DEFINES
#include <cmath>
#include "config.hpp"

extern volatile Knob k3;
extern volatile Knob k2;
extern volatile Knob k1;
extern ThreadSafeList<Note> notesPressed;

class SoundGenerator{
    private:
        int32_t shiftOctave(int32_t stepSize, int32_t octave);
        int32_t sawtooth(Note note);
        int32_t sine(Note note);
    public:
        SoundGenerator();
        int32_t getSound();
};

#endif //ES_BETTERSOUNDINGKEYS