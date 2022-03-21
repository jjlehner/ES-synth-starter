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
    return (int32_t) Vin*exp(- time);
}

int32_t SoundGenerator::clip(const int32_t inputVolume){
    return inputVolume > 255 ? 255 : inputVolume;
}


SoundGenerator::SoundGenerator() = default;

int32_t SoundGenerator::__getSound(Note note){
    uint32_t time = micros() - note.timePressed;
    int32_t stepSize = note.getStepSize();
    int32_t deltaPhaseAcc = stepSize;
    // Vout = this->volumeDecay(Vout, time);
    return deltaPhaseAcc;
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
    int32_t stepSize = 0;
    for(size_t i = 0; i < notes.second ; i++){
        stepSize += this -> __getSound(notes.first[i]);
    }
    if(notes.second){
        stepSize /= notes.second;
    }
    this->phaseAcc += stepSize;
    int32_t Vout = this->phaseAcc >> 24;
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