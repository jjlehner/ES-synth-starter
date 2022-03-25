//
// Created by Neel on 11/03/2022.
//

#include "Tasks.hpp"
#include "STM32FreeRTOS.h"
#include <U8g2lib.h>
#include <bitset>
#include "Knobs.hpp"
#include "ThreadSafeList.hpp"
#include "Recorder.hpp"

enum class SwitchStateChange {
    PRESSED, RELEASED, NO_CHANGE
};


namespace {
    static const char *NOTES[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "No note"};
    typedef uint8_t Switch;

    const Switch INDEX_KEY_C = 0;
    const Switch INDEX_KEY_C_SHARP = 1;
    const Switch INDEX_KEY_D = 2;
    const Switch INDEX_KEY_D_SHARP = 3;
    const Switch INDEX_KEY_E = 4;
    const Switch INDEX_KEY_F = 5;
    const Switch INDEX_KEY_F_SHARP = 6;
    const Switch INDEX_KEY_G = 7;
    const Switch INDEX_KEY_G_SHARP = 8;
    const Switch INDEX_KEY_A = 9;
    const Switch INDEX_KEY_A_SHARP = 10;
    const Switch INDEX_KEY_B = 11;
    const Switch INDEX_KEY_NO_KEY = 12;

    void read(std::bitset<IO::FLAT_KEY_MATRIX_LENGTH> &inputs, size_t row) {
        inputs.set(row * 4 + 3, digitalRead(C0_PIN));
        inputs.set(row * 4 + 2, digitalRead(C1_PIN));
        inputs.set(row * 4 + 1, digitalRead(C2_PIN));
        inputs.set(row * 4, digitalRead(C3_PIN));
    }

// decodes the hexcode to an idx
    inline uint8_t decode_to_idx(uint16_t bits) {
        bits = ~bits;
        for (size_t i = 0; i < 12; i++) {
            if (bits & 1) {
                return 11 - i;
            }
            bits >>= 1;
        }
        return 12;
    }
}

std::array<SwitchStateChange, 12> findKeyStateChanges(const std::bitset<IO::FLAT_KEY_MATRIX_LENGTH> &newKeyArray, const std::bitset<IO::FLAT_KEY_MATRIX_LENGTH> &oldKeyArray) {
    std::array<SwitchStateChange, 12> keyStateChanges;
    keyStateChanges.fill(SwitchStateChange::NO_CHANGE);
    auto releasedKeys = (newKeyArray & ~oldKeyArray) >> (IO::FLAT_KEY_MATRIX_LENGTH - 12);
    auto pressedKeys = (~newKeyArray & oldKeyArray) >> (IO::FLAT_KEY_MATRIX_LENGTH - 12);
    for (int i = 0; i < 12; i++) {
        keyStateChanges[i] = releasedKeys[i] ? SwitchStateChange::RELEASED : keyStateChanges[i];
        keyStateChanges[i] = pressedKeys[i] ? SwitchStateChange::PRESSED : keyStateChanges[i];
    }
    return keyStateChanges;
}

std::array<SwitchStateChange, 4> findKnobStateChange(const std::bitset<IO::FLAT_KEY_MATRIX_LENGTH> &newKeyArray, const std::bitset<IO::FLAT_KEY_MATRIX_LENGTH> &oldKeyArray){
    std::array<SwitchStateChange, 4> keyStateChanges;
    keyStateChanges.fill(SwitchStateChange::NO_CHANGE);
    auto releasedKeys = (newKeyArray & ~oldKeyArray);
    auto pressedKeys = (~newKeyArray & oldKeyArray);

    auto knobIndexs = {3,2,7,6};
    size_t i = 0;
    for(auto knobIndex : knobIndexs){
        keyStateChanges[i] = releasedKeys[knobIndex] ? SwitchStateChange::RELEASED : keyStateChanges[knobIndex];
        keyStateChanges[i] = pressedKeys[knobIndex] ? SwitchStateChange::PRESSED : keyStateChanges[knobIndex];
        i++;
    }

    return keyStateChanges;
}
void Tasks::scanKeysTask(__attribute__((unused)) void *pvParameters) {
    notesPressed = ThreadSafeList<Note>();

    std::bitset<IO::FLAT_KEY_MATRIX_LENGTH> inputs;
    std::bitset<IO::FLAT_KEY_MATRIX_LENGTH> old_inputs;
#ifdef PROFILING
    for(size_t _ = 0; _ < PROFILING_REPEATS; _++){
#else
    const TickType_t xFrequency = 20 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (true) {
#endif

        for (size_t i = 0; i < 7; i++) {
            IO::setRow(i);
            delayMicroseconds(3);
            read(inputs, 6 - i);
        }

        bool a, b;
        std::tie(a, b) = Knobs::getAB(inputs, 3);
        k3.updateRotation(a, b);
        std::tie(a, b) = Knobs::getAB(inputs, 2);
        k2.updateRotation(a, b);
        std::tie(a, b) = Knobs::getAB(inputs, 1);
        k1.updateRotation(a, b);
        std::tie(a, b) = Knobs::getAB(inputs, 0);
        k0.updateRotation(a, b);

        auto keyStateChanges = findKeyStateChanges(inputs, old_inputs);
        for (size_t i = 0; i < keyStateChanges.size(); i++) {
            if (keyStateChanges[i] != SwitchStateChange::NO_CHANGE) {
                CANFrame(keyStateChanges[i] == SwitchStateChange::PRESSED, k2.getRotation(), keyStateChanges.size() - 1 - i).send();
            }
            if (keyStateChanges[i] == SwitchStateChange::PRESSED) {
                notesPressed.push_back(Note{
                        static_cast<uint8_t>((keyStateChanges.size() - 1 - i)), (uint8_t) k2.getRotation(), micros(), PhaseAccPool::aquirePhaseAcc()});
            }
            if (keyStateChanges[i] == SwitchStateChange::RELEASED) {
                auto note = notesPressed.find(Note{static_cast<uint8_t>((keyStateChanges.size() - 1 - i)), (uint8_t) k2.getRotation(), 0});
                while(note.second){
                    notesPressed.remove(Note{static_cast<uint8_t>((keyStateChanges.size() - 1 - i)), (uint8_t) note.first.octaveNum, 0});
                    PhaseAccPool::releasePhaseAcc(note.first.indexPhaseAcc);
                    note = notesPressed.find(Note{static_cast<uint8_t>((keyStateChanges.size() - 1 - i)), (uint8_t) note.first.octaveNum, 0});
                }
            }
        }
        auto knobStateChanges = findKnobStateChange(inputs, old_inputs);
        if(knobStateChanges[0] == SwitchStateChange::PRESSED){
           if(!Recorder::isRecording()) Recorder::startRecording();
           else Recorder::stopRecording();
        }
        if(knobStateChanges[1] == SwitchStateChange::PRESSED){
            if(!Recorder::isPlayingback()) Recorder::startPlayback();
            else Recorder::stopPlayback();
        }

        old_inputs = inputs;
        #ifndef PROFILING
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        #endif
    }
}

void Tasks::displayUpdateTask(__attribute__((unused)) void *pvParameters) {
    char control[16];
    char note[16];
    char waveform[16];

#ifdef PROFILING
    for(size_t _ = 0; _ < PROFILING_REPEATS; _++){
#else
    const TickType_t xFrequency = 100 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (true) {
#endif
        //Update display
        u8g2.clearBuffer();         // clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
        u8g2.setCursor(2, 10);
        if(k3.getRotation() < 10){

            sprintf(control,"Vol: 0%d,  Oct: %d",k3.getRotation(), k2.getRotation());
        }
        else{
            sprintf(control,"Vol: %d,  Oct: %d",k3.getRotation(), k2.getRotation());
        }
        u8g2.print(control);
        u8g2.setCursor(2, 20);
        auto key_index = notesPressed.readISR();
        if(key_index.second){
            sprintf(note, "Note: %s", NOTES[key_index.first[0].noteNum]);
        }
        else{
            sprintf(note, "Note:");
        }
        u8g2.print(note);
        u8g2.setCursor(2, 30);
        sprintf(waveform, "Waveform: %d (%s)", k1.getRotation(), k1.getRotation() < 8 ? "Saw" : "Sine");
        u8g2.print(waveform);
        // u8g2.drawStr(2, 20, NOTES[decode_to_idx(pressed_key_hex)]);
        //u8g2.drawStr(2, 20, std::to_string(PhaseAccPool::accAquired).c_str());
        u8g2.sendBuffer();          // transfer internal memory to the display

        //Toggle LED
        digitalToggle(LED_BUILTIN);
#ifndef PROFILING
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
#endif
    }
}

void Tasks::decodeTask(__attribute__((unused)) void *pvParameters) {
    std::array<uint8_t, 8> RX_Message;
#ifdef PROFILING
    for(size_t _ = 0; _ < PROFILING_REPEATS; _++){
#else
    while (true) {
#endif
        xQueueReceive(msgInQ, RX_Message.data(), portMAX_DELAY);
        auto RX_Frame = CANFrame(RX_Message);
        auto note = Note{RX_Frame.getNoteNum(), (uint8_t) (RX_Frame.getOctaveNum()),micros()};
        if (RX_Frame.getKeyPressed()) {
            note.indexPhaseAcc = PhaseAccPool::aquirePhaseAcc();
            notesPressed.push_back(note);
        } else {
            PhaseAccPool::releasePhaseAcc(note.indexPhaseAcc);
            notesPressed.remove(note);
        }
    }
}

void Tasks::transmitTask(__attribute__((unused)) void *pvParameters) {
    std::array<uint8_t, 8> msgOut;
#ifdef PROFILING
    for(size_t _ = 0; _ < PROFILING_REPEATS_TRANSMIT_TASK; _++){
#else
    while (true) {
#endif
        xQueueReceive(msgOutQ, msgOut.data(), portMAX_DELAY);
        xSemaphoreTake(CAN_TX_Semaphore, portMAX_DELAY);
        CAN_TX(0x123, msgOut.data());
    }
}