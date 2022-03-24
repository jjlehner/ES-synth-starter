//
// Created by Jonah Lehner on 22/03/2022.
//

#include "Recorder.hpp"

std::atomic<RecorderState> Recorder::state(RecorderState::IDLE);
ThreadSafeList<int32_t> Recorder::voltsToStore;