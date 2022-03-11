//
// Created by Neel on 11/03/2022.
//

#include "Tasks.hpp"
#include <Arduino.h>
#include "STM32FreeRTOS.h"
#include <U8g2lib.h>
#include <array>
#include "Knobs.hpp"

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


    inline int shift_n(const int PIN, size_t n) {
        return digitalRead(PIN) << n;
    }

    uint8_t readCols() {
        return shift_n(C0_PIN, 3) + shift_n(C1_PIN, 2) + shift_n(C2_PIN, 1) + shift_n(C3_PIN, 0);
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
    std::array<uint8_t, 7> keyArray{};
    Knobs k3;
    while (true) {
        for (size_t i = 0; i < 7; i++) {
            setRow(i);
            delayMicroseconds(3);
            uint8_t keys = readCols();
            keyArray[i] = keys;
        }
        uint16_t to_be_printed = (keyArray[0] << 4 * 2) + (keyArray[1] << 4 * 1) + keyArray[2];

        bool a, b;

        std::tie(a, b) = Knobs::getAB(keyArray, 3);
        __atomic_store_n(&debugOut, k3.getChange(a, b), __ATOMIC_RELAXED);
        threadSafeArray.write(keyArray);
        __atomic_store_n(&currentStepSize, STEPSIZES[decode_to_idx(to_be_printed)], __ATOMIC_RELAXED);
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