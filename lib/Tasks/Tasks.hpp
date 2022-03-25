//
// Created by Neel on 11/03/2022.
//

#ifndef ES_SYNTH_STARTER_TASKS_HPP
#define ES_SYNTH_STARTER_TASKS_HPP

#include "ThreadSafeList.hpp"
#include "Knob.hpp"
#include "CANFrame.hpp"
#include "config.hpp"
#include "IO.hpp"
#include <U8g2lib.h>


extern volatile Knob k0;
extern volatile Knob k1;
extern volatile Knob k2;
extern volatile Knob k3;

extern U8G2_SSD1305_128X32_NONAME_F_HW_I2C u8g2;

extern std::atomic<int32_t> currentStepSize;

extern const int OUT_PIN;
extern ThreadSafeList<Note> notesPressed;

extern SemaphoreHandle_t CAN_TX_Semaphore;
namespace Tasks {
#ifdef PROFILING
     void scanKeysTask(__attribute__((unused)) void *pvParameters);

     void displayUpdateTask(__attribute__((unused)) void *pvParameters);

     void decodeTask(__attribute__((unused)) void *pvParameters);

     void transmitTask(__attribute__((unused)) void *pvParameters);
#else
    [[noreturn]] void scanKeysTask(__attribute__((unused)) void *pvParameters);

    [[noreturn]] void displayUpdateTask(__attribute__((unused)) void *pvParameters);

    [[noreturn]] void decodeTask(__attribute__((unused)) void *pvParameters);

    [[noreturn]] void transmitTask(__attribute__((unused)) void *pvParameters);
#endif
}

#endif //ES_SYNTH_STARTER_TASKS_HPP
