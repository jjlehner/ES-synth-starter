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
extern QueueHandle_t msgOutQ;

struct Note {
    uint8_t noteNum;
    uint8_t octaveNum;

    int32_t getStepSize();

    bool operator==(const Note &b);
};

class CANFrame {
private:
    bool keyPressed;
    uint8_t octaveNum;
    uint8_t noteNum;

    static constexpr uint32_t MESSAGE_ID = 0x123;

public:
    CANFrame(bool keyPressed, uint8_t octaveNum, uint8_t noteNum);

    CANFrame(std::array<uint8_t, 8> rawMessage);

    bool getKeyPressed();

    uint8_t getOctaveNum();

    uint8_t getNoteNum();

    void send();

    static void receiveISR();

};


#endif //ES_SYNTH_STARTER_CAN_FRAME_HPP
