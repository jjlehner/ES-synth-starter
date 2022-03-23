#include "SoundGenerator.hpp"

// KeyHandler::KeyHandler(int32_t stepSize){
//     this->stepSize = stepSize;
// }

// inline void KeyHandler::checkPress(const int32_t stepSize){
//     if (this->beingPressed == false && stepSize != 0){ // starting press
//         this->beingPressed = true;
//         this->keyPressTime = 0;
//     }
//     return;
// }

// inline int32_t KeyHandler::volumeDecay(const int32_t inputVolume){
//     this->keyPressTime += 1./22000.;
//     return (int32_t) inputVolume*exp(- this->keyPressTime);
// }

// inline int32_t KeyHandler::clip(const int32_t inputVolume){
//     return inputVolume > 255 ? 255 : inputVolume;
// }

// int32_t KeyHandler::getKeyOutput(const int32_t stepSize){
//     this->checkPress(stepSize);
//     this->phaseAcc += stepSize;
//     int32_t Vout = phaseAcc >> 24;
//     Vout= Vout >> (8 -k3.getRotation()/2);
//     Vout = this->volumeDecay(Vout);
//     Vout = this->clip(Vout);
//     return Vout;
// }

int32_t SoundGenerator::volumeDecay(int32_t Vin, const int32_t time){
    return (int32_t) Vin*exp(- time/1.0e6);
}

int32_t SoundGenerator::clip(const int32_t inputVolume){
    return inputVolume > 255 ? 255 : inputVolume;
}

int32_t SoundGenerator::shiftOctave(int32_t stepSize, double octave){
    octave -= 4.;
    stepSize *= (int32_t) pow(2.0, octave);
    return stepSize;
}

SoundGenerator::SoundGenerator() = default;

int32_t SoundGenerator::sawtooth(Note note){
    uint32_t time = micros() - note.timePressed;
    int32_t stepSize = note.getStepSize();
    // double octave = (double) k2.getRotation();
    // stepSize = this->shiftOctave(stepSize, octave);
    int32_t deltaPhaseAcc = stepSize;
    // deltaPhaseAcc = this->volumeDecay(deltaPhaseAcc, time);
    return deltaPhaseAcc;
}

int32_t SoundGenerator::sine(Note note){
    uint32_t time = micros() - note.timePressed;
    double frequency = (double) note.getFrequency();
    // Quick sine approximation
    double pi = 3.1415926535;
    double x = 2.*pi* (double) time / 1.0e6;
    // double sinex = 16.*x*(pi-x) / (5. * pi * pi - 4.*x*(pi-x));
    // double sinx = 4.*x / (pi*pi) * (pi-x);
    double sinx = -0.417698*x*x + 1.312236*x-0.050465;
    // double sineout = sin(2.*3.1415926535* (double) time / 1.0e6);
    int32_t ret_val = 256/2* sinx;
    return ret_val;
}

int32_t SoundGenerator::getSound(){
    // auto t = notesPressed.read();
    // if(t.second){
    //     phaseAcc += __getSound(t.first[0]);
    // }
    // int32_t Vout = phaseAcc >> 24;
    // Vout = Vout >> (8 - k3.getRotation() / 2);

    // return Vout;
    auto notes = notesPressed.read();
    size_t waveform = k1.getRotation()-8;
    int32_t Vout = 0;
    if (waveform == 0){ // Sawtooth
        for(size_t i = 0; i < notes.second ; i++){
//        PhaseAccPool::phaseAccPool[notes.first[i].indexPhaseAcc] += this -> __getSound(notes.first[i]);
//        Vout += PhaseAccPool::phaseAccPool[notes.first[i].indexPhaseAcc] >> 24;

        PhaseAccPool::setPhaseAcc(notes.first[i].indexPhaseAcc, PhaseAccPool::phaseAcc(notes.first[i].indexPhaseAcc) + this -> sawtooth(notes.first[i]));
        Vout += PhaseAccPool::phaseAcc(notes.first[i].indexPhaseAcc) >> 24;
    }
    } else { // Sine
        for(size_t i = 0; i < notes.second ; i++){
//        PhaseAccPool::phaseAccPool[notes.first[i].indexPhaseAcc] += this -> __getSound(notes.first[i]);
//        Vout += PhaseAccPool::phaseAccPool[notes.first[i].indexPhaseAcc] >> 24;

        PhaseAccPool::setPhaseAcc(notes.first[i].indexPhaseAcc, PhaseAccPool::phaseAcc(notes.first[i].indexPhaseAcc) + this -> sine(notes.first[i]));
        Vout += PhaseAccPool::phaseAcc(notes.first[i].indexPhaseAcc) >> 24;
    }
    }
    
    //if(notes.second){
    //    stepSize /= (int32_t) notes.second;
    //}

    Vout = Vout >> (8-k3.getRotation()/2);
    return Vout;


    // return this->clip(this->phaseAcc);
    // int32_t keyOutputs[12] = {0};
    // int8_t numPressedKeys = 0;
    // // Get the outputs of the pressed keys
    // for(int i=0; i<12; i++){
    //     keyOutputs[i] = this->keys[i].getKeyOutput(stepSizes[i]);
    //     numPressedKeys += stepSizes[i] > 0 ? 1 : 0;
    // }
    // // Average over non-zero values
    // int32_t finalOutput = 0;
    // for(int i=0; i<12; i++){
    //     finalOutput += stepSizes[i] > 0 ? keyOutputs[i] : 0;
    // }
    // finalOutput /= numPressedKeys;
    // Return
}