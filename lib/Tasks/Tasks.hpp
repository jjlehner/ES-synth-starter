//
// Created by Neel on 11/03/2022.
//

#ifndef ES_SYNTH_STARTER_TASKS_HPP
#define ES_SYNTH_STARTER_TASKS_HPP

#include "ThreadSafeArray.hpp"
#include "ThreadSafeList.hpp"
#include "Knobs.hpp"
#include "CANFrame.hpp"
#include "config.hpp"
#include <U8g2lib.h>

extern ThreadSafeArray threadSafeArray;

extern volatile Knobs k0;
extern volatile Knobs k1;
extern volatile Knobs k2;
extern volatile Knobs k3;

extern const int C0_PIN;
extern const int C1_PIN;
extern const int C2_PIN;
extern const int C3_PIN;

extern const int REN_PIN;

extern const int RA0_PIN;
extern const int RA1_PIN;
extern const int RA2_PIN;

extern U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2;

extern std::atomic<int32_t> currentStepSize;

extern SemaphoreHandle_t CAN_TX_Semaphore;
namespace Tasks {
    [[noreturn]] void scanKeysTask(__attribute__((unused)) void *pvParameters);

    [[noreturn]] void displayUpdateTask(__attribute__((unused)) void *pvParameters);

    [[noreturn]] void decodeTask(__attribute__((unused)) void *pvParameters);

    [[noreturn]] void transmitTask(__attribute__((unused)) void *pvParameters);
}

#endif //ES_SYNTH_STARTER_TASKS_HPP
