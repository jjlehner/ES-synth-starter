//
// Created by Neel on 12/03/2022.
//

#include "CANFrame.hpp"

CANFrame::CANFrame(bool keyPressed, uint8_t octaveNum, uint8_t noteNum) : keyPressed(keyPressed),
                                                                             octaveNum(octaveNum),
                                                                             noteNum(noteNum) {}

void CANFrame::send() {
    uint8_t keyPressed = this->keyPressed ? 0x50 : 0x52;
    std::array<uint8_t, 8> frame = {
            keyPressed, this->octaveNum, this->noteNum, 0, 0, 0, 0, 0
    };

    CAN_TX(MESSAGE_ID, frame.data());
}

void CANFrame::receiveISR() {

    std::array<uint8_t, 8> incomingFrame;
    uint32_t ID;
    CAN_RX(ID, incomingFrame.data());
    xQueueSendFromISR(msgInQ, incomingFrame.data(), NULL);
}
