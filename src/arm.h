#pragma once

#include <psp2kern/kernel/threadmgr/debugger.h>

namespace arm
{
    using Instruction = uint32_t;
    using ArmInstruction = uint32_t;
    using ThumbInstruction = uint32_t;
    using Thumb16Instruction = uint16_t;
    using Thumb32Instruction = uint32_t;

    bool isThumb(SceThreadCpuRegisters const& registers);
    uint32_t getThumbInstructionSize(ThumbInstruction instruction);
    uintptr_t getNextInstructionAddr(SceThreadCpuRegisters const& registers, Instruction instruction);
    int32_t getNextInstructionOffset(SceThreadCpuRegisters const& registers, Instruction instruction);
}
