//
// Created by Jonah Lehner on 21/03/2022.
//

#include "PhaseAccPool.hpp"



std::array<bool, PhaseAccPool::PHASE_ACC_POOL_SIZE> PhaseAccPool::phaseAccUsed{};
std::array<int32_t, PhaseAccPool::PHASE_ACC_POOL_SIZE> PhaseAccPool::phaseAccPool{};

