//
// Created by Neel on 12/03/2022.
//

#ifndef ES_SYNTH_STARTER_CAN_FRAME_HPP
#define ES_SYNTH_STARTER_CAN_FRAME_HPP

#include <cstdint>
#include <array>

class CANFrame {
private:
    bool isPressed;
    uint8_t octaveNum;
    uint8_t noteNum;
public:


    void send(){
        std::array<uint8_t, 8> message;

    }
};


#endif //ES_SYNTH_STARTER_CAN_FRAME_HPP
