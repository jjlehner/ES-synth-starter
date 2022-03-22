//
// Created by Jonah Lehner on 21/03/2022.
//

#ifndef ES_SYNTH_STARTER_PHASEACCPOOL_HPP
#define ES_SYNTH_STARTER_PHASEACCPOOL_HPP


#include <array>
#include <algorithm>
#include <cassert>
#include <atomic>

class PhaseAccPool {
    static constexpr size_t PHASE_ACC_POOL_SIZE = 36;

public:
    static std::array<bool, PHASE_ACC_POOL_SIZE> phaseAccUsed;
    static std::array<int32_t, PHASE_ACC_POOL_SIZE> phaseAccPool;
    static std::atomic<size_t> accAquired;
    static size_t aquirePhaseAcc() {
        auto index =  std::find(phaseAccUsed.begin(), phaseAccUsed.end(), false);
        if(index == phaseAccUsed.end()){
            return 0;
        }
        else{
            *index = true;
        }
        //accAquired++;
        return index-phaseAccUsed.begin();
    }

    static void releasePhaseAcc(const size_t &index) {
        phaseAccPool[index] = 0;
        phaseAccUsed[index] = false;
        //accAquired--;
    }

};


#endif //ES_SYNTH_STARTER_PHASEACCPOOL_HPP
