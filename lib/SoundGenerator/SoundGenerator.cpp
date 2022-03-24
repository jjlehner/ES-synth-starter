#include "SoundGenerator.hpp"

int32_t raw_sinewave[250] = {
        0,3,6,9,12,16,19,22,25,28,31,34,38,41,44,47,50,53,
        55,58,61,64,67,69,72,75,77,80,82,85,87,89,92,94,96,
        98,100,102,104,106,108,109,111,112,114,115,117,118,
        119,120,121,122,123,124,125,125,126,126,127,127,127,
        127,127,127,127,127,127,127,126,126,125,125,124,123,
        122,121,120,119,118,117,115,114,112,111,109,108,106,
        104,102,100,98,96,94,92,89,87,85,82,80,77,75,72,69,67,
        64,61,58,55,53,50,47,44,41,38,34,31,28,25,22,19,16,12,
        9,6,3,0,-3,-6,-9,-12,-16,-19,-22,-25,-28,-31,-34,-38,
        -41,-44,-47,-50,-53,-55,-58,-61,-64,-67,-69,-72,-75,
        -77,-80,-82,-85,-87,-89,-92,-94,-96,-98,-100,-102,-104,
        -106,-108,-109,-111,-112,-114,-115,-117,-118,-119,-120,
        -121,-122,-123,-124,-125,-125,-126,-126,-127,-127,-127,
        -127,-127,-127,-127,-127,-127,-127,-126,-126,-125,-125,
        -124,-123,-122,-121,-120,-119,-118,-117,-115,-114,-112,
        -111,-109,-108,-106,-104,-102,-100,-98,-96,-94,-92,-89,
        -87,-85,-82,-80,-77,-75,-72,-69,-67,-64,-61,-58,-55,
        -53,-50,-47,-44,-41,-38,-34,-31,-28,-25,-22,-19,-16,
        -12,-9,-6,-3
    };

int32_t SoundGenerator::volumeDecay(const int32_t Vin, const int32_t time){
    return (int32_t) Vin*exp(- time/1.0e6);
}

int32_t SoundGenerator::clip(const int32_t inputVolume){
    return inputVolume > 255 ? 255 : inputVolume;
}

int32_t SoundGenerator::shiftOctave(const int32_t stepSize, const int32_t octave){
    return octave - 4 > 0 ? stepSize << (octave -4) : stepSize >> (4 - octave);
}

SoundGenerator::SoundGenerator() = default;

int32_t SoundGenerator::sawtooth(Note note){
    uint32_t time = micros() - note.timePressed;
    int32_t stepSize = note.getStepSize();
    int32_t octave = (int32_t) k2.getRotation();
    stepSize = this->shiftOctave(stepSize, octave);
    int32_t deltaPhaseAcc = stepSize;
    return deltaPhaseAcc;
}

int32_t SoundGenerator::sine(Note note){
    uint32_t time = micros() - note.timePressed;
    int32_t frequency = note.getFrequency();
    int32_t octave = (int32_t) k2.getRotation();
    frequency = octave - 4 > 0 ? frequency << (octave -4) : frequency >> (4 - octave);
    size_t idx = (size_t) round(250*frequency* time/1e6) % 250;
    return raw_sinewave[idx];
}

int32_t SoundGenerator::getSound(){
    auto notes = notesPressed.read();
    int16_t waveform = k1.getRotation()-8;
    int32_t Vout = 0;
    if (waveform == 0){ // Sawtooth
        for(size_t i = 0; i < notes.second ; i++){
            PhaseAccPool::setPhaseAcc(notes.first[i].indexPhaseAcc, PhaseAccPool::phaseAcc(notes.first[i].indexPhaseAcc) + this -> sawtooth(notes.first[i]));
            Vout += PhaseAccPool::phaseAcc(notes.first[i].indexPhaseAcc) >> 24;
        }
    } else { // Sine
        for(size_t i = 0; i < notes.second ; i++){
            // PhaseAccPool::setPhaseAcc(notes.first[i].indexPhaseAcc, this -> sine(notes.first[i]));
            Vout = this -> sine(notes.first[i]);
        }
    }
    Vout = Vout >> (8-k3.getRotation()/2);
    return Vout;
}