//
// Created by Jonah Lehner on 21/03/2022.
//

#ifndef ES_SYNTH_STARTER_PHASEACCPOOL_HPP
#define ES_SYNTH_STARTER_PHASEACCPOOL_HPP


#include <array>
#include <algorithm>
#include <cassert>
#include <atomic>
#include "STM32FreeRTOS.h"

class PhaseAccPool {
    static constexpr size_t PHASE_ACC_POOL_SIZE = 36;

    static std::array<bool, PHASE_ACC_POOL_SIZE> phaseAccUsed;
    static std::array<int32_t, PHASE_ACC_POOL_SIZE> phaseAccPool;
public:

    static size_t aquirePhaseAcc() {
        taskENTER_CRITICAL();
        auto it =  std::find(phaseAccUsed.begin(), phaseAccUsed.end(), false);
        if(it == phaseAccUsed.end()){
            return 0;
        }
        else{
            *it = true;
        }
        //accAquired++;
        auto index = it-phaseAccUsed.begin();
        taskEXIT_CRITICAL();

        return index;
    }

    static void releasePhaseAcc(const size_t &index) {
        taskENTER_CRITICAL();
        phaseAccPool[index] = 0;
        phaseAccUsed[index] = false;
        taskEXIT_CRITICAL();
    }

    static int32_t phaseAcc(const size_t& index){
        UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
        auto retVal= phaseAccPool[index];
        taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
        return retVal;

    }

    static void setPhaseAcc(const size_t& index, const int32_t & val){
        UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
        phaseAccPool[index] = val;
        taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
    }

};


#endif //ES_SYNTH_STARTER_PHASEACCPOOL_HPP
