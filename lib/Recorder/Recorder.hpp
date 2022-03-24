//
// Created by Jonah Lehner on 21/03/2022.
//

#ifndef ES_SYNTH_STARTER_RECORDER_HPP
#define ES_SYNTH_STARTER_RECORDER_HPP

#include <atomic>
#include <cassert>
#include "ThreadSafeList.hpp"
#include "Arduino.h"
enum class RecorderState{
    IDLE, RECORD, PLAYBACK
};

class Recorder{
    static std::atomic<RecorderState> state;
    static ThreadSafeList<int32_t> voltsToStore;
public:
    static bool isRecording(){
        return state==RecorderState::RECORD ;
    }
    static bool isIdle(){
        return state==RecorderState::IDLE ;
    }
    static bool isPlayingback(){
        return state==RecorderState::PLAYBACK ;
    }
    static RecorderState getState(){
        return state;
    }
    static void startRecording(){
        auto idle = RecorderState::IDLE;
        if(state.compare_exchange_strong(idle, RecorderState::RECORD)){
            voltsToStore.clear();
        }
    }
    static void stopRecording(){
        auto record = RecorderState::RECORD;
        state.compare_exchange_strong(record, RecorderState::IDLE);
    }
    static void startPlayback(){
        auto idle = RecorderState::IDLE;
        state.compare_exchange_strong(idle, RecorderState::PLAYBACK);
    }
    static void stopPlayback(){
        auto playback = RecorderState::PLAYBACK;
        state.compare_exchange_strong(playback, RecorderState::IDLE);
    }
    static std::string getStateAsString(){
        switch (getState()) {
            case RecorderState::IDLE:
                return "IDLE";
            case RecorderState::RECORD:
                return "RECORDING";
            case RecorderState::PLAYBACK:
                return "PLAYBACK";
        }
        return "";
    }

};
#endif //ES_SYNTH_STARTER_RECORDER_HPP
