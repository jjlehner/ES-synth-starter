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
    uint8_t keyPressed;
    uint8_t octaveNum;
    uint8_t noteNum;

    static constexpr uint32_t MESSAGE_ID = 0x123;

public:

    void send(){
        std::array<uint8_t, 8> frame = {
            keyPressed, octaveNum, noteNum, 0, 0, 0, 0, 0
        };

        CAN_TX(MESSAGE_ID, frame.data());
    }

    static void receiveISR(){

        std::array<uint8_t,8> incomingFrame;
        uint32_t ID;
        CAN_RX(ID, incomingFrame.data());
        xQueueSendFromISR(msgInQ, incomingFrame.data(), NULL);
    }
};



#endif //ES_SYNTH_STARTER_CAN_FRAME_HPP
