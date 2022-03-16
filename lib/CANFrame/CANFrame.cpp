//
// Created by Neel on 12/03/2022.
//

#include "CANFrame.hpp"

CANFrame::CANFrame(uint8_t keyPressed, uint8_t octaveNum, uint8_t noteNum) : keyPressed(keyPressed),
                                                                             octaveNum(octaveNum),
                                                                             noteNum(noteNum) {}

void CANFrame::send() {
    std::array<uint8_t, 8> frame = {
            keyPressed, octaveNum, noteNum, 0, 0, 0, 0, 0
    };

    CAN_TX(MESSAGE_ID, frame.data());
}

void CANFrame::receiveISR() {

    std::array<uint8_t, 8> incomingFrame;
    uint32_t ID;
    CAN_RX(ID, incomingFrame.data());
    xQueueSendFromISR(msgInQ, incomingFrame.data(), NULL);
}
