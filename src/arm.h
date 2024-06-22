#pragma once

#include <psp2kern/kernel/threadmgr/debugger.h>

namespace arm
{
    using Instruction = uint32_t;
    using ArmInstruction = uint32_t;
    using ThumbInstruction = uint32_t;
    using Thumb16Instruction = uint16_t;
    using Thumb32Instruction = uint32_t;

    constexpr uint32_t THUMB_32_BIT_MASK = 0x1f << 11;

    constexpr uint32_t THUMB_INST_BL_32_BIT_MASK      = 0b11010000000000001111000000000000;
    constexpr uint32_t THUMB_INST_BLX_32_BIT_MASK     = 0b11000000000000001111000000000000;

    constexpr uint32_t ARM_INST_BL_BIT_MASK           = 0b00001011000000000000000000000000;
    constexpr uint32_t ARM_INST_BLX_BIT_MASK          = 0b11111010000000000000000000000000;

    constexpr uint16_t THUMB_INST_BLX_REG_16_BIT_MASK = 0b0100011110000000;

    constexpr uint32_t ARM_INST_BLX_REG_BIT_MASK      = 0b00000001001011111111111100110000;

    constexpr uint16_t THUMB_INST_BX_REG_16_BIT_MASK  = 0b0100011100000000;

    constexpr uint32_t ARM_INST_BX_REG_BIT_MASK       = 0b00000001001011111111111100010000;

    constexpr uint16_t THUMB_INST_B_16_BIT_MASK       = 0b1101000000000000;
    constexpr uint16_t THUMB_INST2_B_16_BIT_MASK      = 0b1110000000000000;
    constexpr uint32_t THUMB_INST_B_32_BIT_MASK       = 0b10000000000000001111000000000000;
    constexpr uint32_t THUMB_INST2_B_32_BIT_MASK      = 0b10010000000000001111000000000000;

    constexpr uint32_t ARM_INST_B_BIT_MASK            = 0b00001010000000000000000000000000;

    constexpr uint32_t ARM_INST_BKPT                  = 0b11100001001000000000000001110000;

    constexpr uint16_t THUMB_INST_BKPT_16_BIT         = 0b1011111000000000;

    constexpr uint16_t THUMB_INST_IT_16_BIT           = 0b1011111100000000;

    constexpr uint16_t THUMB_INST_POP_16_BIT          = 0b1011110000000000;

    bool isThumb(SceThreadCpuRegisters const& registers);
    uint32_t getThumbInstructionSize(ThumbInstruction instruction);
    uintptr_t getNextInstructionAddr(SceUID pid, SceThreadCpuRegisters const& registers, Instruction instruction);
    int32_t getNextInstructionOffset(SceUID pid, SceThreadCpuRegisters const& registers, Instruction instruction);
    bool handleCondition(SceThreadCpuRegisters const& registers, uint8_t condition);
}
