//
// Created by Neel on 11/03/2022.
//

#include "Tasks.hpp"
#include <Arduino.h>
#include "STM32FreeRTOS.h"
#include <U8g2lib.h>
#include <bitset>
#include <sstream>
#include "Knobs.hpp"
#include "CANFrame.hpp"


namespace {
    const static int32_t STEPSIZES[] = {int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -9. / 12.) / 22000.),
                                        int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -8. / 12.) / 22000.0),
                                        int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -7. / 12.) / 22000.0),
                                        int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -6. / 12.) / 22000.0),
                                        int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -5. / 12.) / 22000.0),
                                        int32_t(pow(2., 32.) * 440. * pow(2., -4. / 12.) / 22000.),
                                        int32_t(pow(2., 32.) * 440. * pow(2., -3. / 12.) / 22000.),
                                        int32_t(pow(2., 32.) * 440. * pow(2., -2. / 12.) / 22000.),
                                        int32_t(pow(2., 32.) * 440. * pow(2., -1. / 12.) / 22000.),
                                        int32_t(pow(2., 32.) * 440. * pow(2., 0. / 12.) / 22000.),
                                        int32_t(pow(2., 32.) * 440. * pow(2., 1. / 12.) / 22000.),
                                        int32_t(pow(2., 32.) * 440. * pow(2.0, 2. / 12.) / 22000.), int32_t(0.0)};

    static const char *NOTES[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B", "No Key"};

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

    inline int shift_n(const int PIN, size_t n) {
        return digitalRead(PIN) << n;
    }

    uint8_t readCols() {
        return shift_n(C0_PIN, 3) + shift_n(C1_PIN, 2) + shift_n(C2_PIN, 1) + shift_n(C3_PIN, 0);
    }

    void read(std::bitset<24> &inputs, size_t row) {
        inputs.set(row * 4 + 3, digitalRead(C0_PIN));
        inputs.set(row * 4 + 2, digitalRead(C1_PIN));
        inputs.set(row * 4 + 1, digitalRead(C2_PIN));
        inputs.set(row * 4, digitalRead(C3_PIN));
    }

    void setRow(uint8_t rowIdx) {
        digitalWrite(REN_PIN, LOW);
        digitalWrite(RA0_PIN, rowIdx & 0x1);
        digitalWrite(RA1_PIN, rowIdx & 0x2);
        digitalWrite(RA2_PIN, rowIdx & 0x4);
        digitalWrite(REN_PIN, HIGH);
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

void Tasks::scanKeysTask(__attribute__((unused)) void *pvParameters) {
    const TickType_t xFrequency = 20 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint16_t to_be_printed = 0x0;
    uint16_t prev_to_be_printed = 0x0;

    std::bitset<24> inputs;

    while (true) {
        for (size_t i = 0; i < 6; i++) {
            setRow(i);
            delayMicroseconds(3);
            read(inputs, 5 - i);
        }
        to_be_printed = (inputs >> 12).to_ulong();

//        if (prev_to_be_printed ^ to_be_printed) {
//            Switch keyNum = decode_to_idx(prev_to_be_printed ^ to_be_printed);
//        }

        bool a, b;
        std::tie(a, b) = Knobs::getAB(inputs, 3);
        k3.updateRotation(a, b);
        std::tie(a, b) = Knobs::getAB(inputs, 2);
        k2.updateRotation(a, b);
        std::tie(a, b) = Knobs::getAB(inputs, 1);
        k1.updateRotation(a, b);
        std::tie(a, b) = Knobs::getAB(inputs, 0);
        k0.updateRotation(a, b);

        auto keyStateChanges = threadSafeArray.findKeyStateChanges(inputs);
        for(size_t i = 0; i < keyStateChanges.size(); i++){
            if(keyStateChanges[i] != KeyStateChange::NO_CHANGE){
                CANFrame(keyStateChanges[i] == KeyStateChange::PRESSED, 4, keyStateChanges.size() - i).send();
            }
        }
        threadSafeArray.write(inputs);
        currentStepSize.store(STEPSIZES[decode_to_idx(to_be_printed)], std::memory_order_relaxed);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void Tasks::displayUpdateTask(__attribute__((unused)) void *pvParameters) {
    const TickType_t xFrequency = 100 / portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (true) {
        //Update display
        u8g2.clearBuffer();         // clear the internal memory
        u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
        uint16_t pressed_key_hex = threadSafeArray.read();
        u8g2.setCursor(2, 10);
        u8g2.print(pressed_key_hex, HEX);
        u8g2.drawStr(2, 20, NOTES[decode_to_idx(pressed_key_hex)]);
        u8g2.sendBuffer();          // transfer internal memory to the display

        //Toggle LED
        digitalToggle(LED_BUILTIN);
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void Tasks::decodeTask(__attribute__((unused)) void *pvParameters) {
    std::array<uint8_t , 8> RX_Message;
    while(true){
        xQueueReceive(msgInQ, RX_Message.data(), portMAX_DELAY);
        std::ostringstream ss;
        for(auto data : RX_Message){
            ss << data;
        }
        Serial.println(ss.str().c_str());
    }
}