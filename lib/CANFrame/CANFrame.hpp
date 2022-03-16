//
// Created by Neel on 12/03/2022.
//

#ifndef ES_SYNTH_STARTER_CAN_FRAME_HPP
#define ES_SYNTH_STARTER_CAN_FRAME_HPP

#include <cstdint>
#include <array>
#include <ES_CAN.h>
#include <STM32FreeRTOS.h>

extern QueueHandle_t msgInQ;

class CANFrame {
private:
    bool keyPressed;
    uint8_t octaveNum;
    uint8_t noteNum;

    static constexpr uint32_t MESSAGE_ID = 0x123;

public:
    CANFrame(bool keyPressed, uint8_t octaveNum, uint8_t noteNum);

    void send();

    static void receiveISR();

};


#endif //ES_SYNTH_STARTER_CAN_FRAME_HPP
