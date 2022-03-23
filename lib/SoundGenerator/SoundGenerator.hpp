#ifndef ES_BETTERSOUNDINGKEYS
#define ES_BETTERSOUNDINGKEYS

#include <Arduino.h>
#include "Knobs.hpp"
#include "ThreadSafeList.hpp"
#include <list>
#include "CANFrame.hpp"
#define _USE_MATH_DEFINES
#include <cmath>

extern volatile Knobs k3;
extern volatile Knobs k2;
extern volatile Knobs k1;
extern ThreadSafeList<Note> notesPressed;

// class KeyHandler{
//     private:
//         double keyPressTime;
//         bool beingPressed;
//         static int32_t phaseAcc;
//         inline int32_t volumeDecay(const int32_t inputVolume);
//         inline int32_t clip(const int32_t inputVolume);
//         inline void checkPress(const int32_t stepSize);
//     public:
//         int32_t stepSize;
//         KeyHandler(int32_t stepSize);
//         int32_t getKeyOutput(const int32_t stepSize);
// };

class SoundGenerator{
    private:
        int32_t volumeDecay(int32_t Vin, const int32_t time);
        int32_t clip(const int32_t inputVolume);
        int32_t shiftOctave(int32_t stepSize, double octave);
        int32_t sawtooth(Note note);
        int32_t sine(Note note);
        uint32_t phaseAcc;
    public:
        SoundGenerator();
        int32_t getSound();
};

#endif //ES_BETTERSOUNDINGKEYS