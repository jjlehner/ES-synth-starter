//
// Created by Neel on 12/03/2022.
//

#include "CANFrame.hpp"
#include <cmath>

CANFrame::CANFrame(bool keyPressed, uint8_t octaveNum, uint8_t noteNum) : keyPressed(keyPressed),
                                                                          octaveNum(octaveNum),
                                                                          noteNum(noteNum) {}

const static int32_t STEPSIZES[] = {int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -9. / 12.) / 22000.),
                                    int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -8. / 12.) /
                                            22000.0),
                                    int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -7. / 12.) /
                                            22000.0),
                                    int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -6. / 12.) /
                                            22000.0),
                                    int32_t(pow(2.0, 32.0) * 440.0 * pow(2.0, -5. / 12.) /
                                            22000.0),
                                    int32_t(pow(2., 32.) * 440. * pow(2., -4. / 12.) / 22000.),
                                    int32_t(pow(2., 32.) * 440. * pow(2., -3. / 12.) / 22000.),
                                    int32_t(pow(2., 32.) * 440. * pow(2., -2. / 12.) / 22000.),
                                    int32_t(pow(2., 32.) * 440. * pow(2., -1. / 12.) / 22000.),
                                    int32_t(pow(2., 32.) * 440. * pow(2., 0. / 12.) / 22000.),
                                    int32_t(pow(2., 32.) * 440. * pow(2., 1. / 12.) / 22000.),
                                    int32_t(pow(2., 32.) * 440. * pow(2.0, 2. / 12.) / 22000.),
                                    int32_t(0.0)};

void CANFrame::send() {
    uint8_t keyPressed = this->keyPressed ? 0x50 : 0x52;
    std::array<uint8_t, 8> frame = {
            keyPressed, this->octaveNum, this->noteNum, 0, 0, 0, 0, 0
    };
    if(uxQueueSpacesAvailable(msgOutQ)){
        xQueueSend(msgOutQ, frame.data(), portMAX_DELAY);
    }
}

void CANFrame::receiveISR() {

    std::array<uint8_t, 8> incomingFrame;
    uint32_t ID;
    CAN_RX(ID, incomingFrame.data());
    xQueueSendFromISR(msgInQ, incomingFrame.data(), NULL);
}

CANFrame::CANFrame(std::array<uint8_t, 8> rawMessage) : keyPressed(rawMessage[0] == 0x50),
                                                        octaveNum(rawMessage[1]),
                                                        noteNum(rawMessage[2]) {}

bool CANFrame::getKeyPressed() {
    return keyPressed;
}

uint8_t CANFrame::getOctaveNum() {
    return octaveNum;
}

uint8_t CANFrame::getNoteNum() {
    return noteNum;
}

int32_t Note::getStepSize() {
    return this->octaveNum > 4 ?
           STEPSIZES[this->noteNum] << (this->octaveNum - 4) :
           STEPSIZES[this->noteNum] >> (4 - this->octaveNum);
}

bool Note::operator==(const Note &b) {
    return (this->noteNum == b.noteNum) && (this->octaveNum == b.octaveNum);
}
