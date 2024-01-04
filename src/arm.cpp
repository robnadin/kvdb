#include "arm.h"
#include "log.h"

bool arm::isThumb(SceThreadCpuRegisters const& registers) {
    auto regs = (registers.user.cpsr & 0x1F) == 0x10 ? (&registers.user) : (&registers.kernel);
    return (regs->cpsr & (1 << 5)) != 0;
}

constexpr uint32_t THUMB_32_BIT_MASK = 0x1f << 11;

constexpr uint32_t THUMB_INST_BL_32_BIT_MASK      = 0b11110000000000001101000000000000;
constexpr uint32_t THUMB_INST_BLX_32_BIT_MASK     = 0b11110000000000001100000000000000;

constexpr uint32_t ARM_INST_BL_BIT_MASK           = 0b00001011000000000000000000000000;
constexpr uint32_t ARM_INST_BLX_BIT_MASK          = 0b11111010000000000000000000000000;

constexpr uint16_t THUMB_INST_BLX_REG_16_BIT_MASK = 0b0100011110000000;

constexpr uint32_t ARM_INST_BLX_REG_BIT_MASK      = 0b00000001001011111111111100110000;

constexpr uint16_t THUMB_INST_BX_REG_16_BIT_MASK  = 0b0100011100000000;

constexpr uint32_t ARM_INST_BX_REG_BIT_MASK       = 0b00000001001011111111111100010000;

constexpr uint16_t THUMB_INST_B_16_BIT_MASK       = 0b1101000000000000;
constexpr uint16_t THUMB_INST2_B_16_BIT_MASK      = 0b1110000000000000;
constexpr uint32_t THUMB_INST_B_32_BIT_MASK       = 0b11110000000000001000000000000000;
constexpr uint32_t THUMB_INST2_B_32_BIT_MASK      = 0b11110000000000001001000000000000;

constexpr uint32_t ARM_INST_B_BIT_MASK            = 0b00001010000000000000000000000000;

constexpr uint32_t ARM_INST_BKPT                  = 0b11100001001000000000000001110000;

constexpr uint16_t THUMB_INST_BKPT_16_BIT         = 0b1011111000000000;

uint32_t arm::getThumbInstructionSize(ThumbInstruction instruction) {

    uint32_t instruction_size = 2;

    switch(instruction & THUMB_32_BIT_MASK) {
        case (0x1f << 11):
        case (0x1e << 11):
        case (0x1d << 11):
            instruction_size = 4;
            break;
        default:
            break;
    }

    return instruction_size;
}

uintptr_t arm::getNextInstructionAddr(SceThreadCpuRegisters const& registers, Instruction instruction) {
    auto regs = (registers.user.cpsr & 0x1F) == 0x10 ? (&registers.user) : (&registers.kernel);
    return regs->pc + getNextInstructionOffset(registers, instruction);
}

int32_t arm::getNextInstructionOffset(SceThreadCpuRegisters const& registers, Instruction instruction) {
    if (isThumb(registers)) {
        ThumbInstruction thumb_instruction = instruction;
        uint32_t thumb_instruction_size = getThumbInstructionSize(thumb_instruction);
        if (thumb_instruction_size == 4) {
            if ((thumb_instruction & THUMB_INST_BL_32_BIT_MASK) == THUMB_INST_BL_32_BIT_MASK) {
                uint16_t instr_l = thumb_instruction & 0xffff;
                uint16_t instr_h = (thumb_instruction >> 16) & 0xffff;
                uint8_t sign = (instr_h >> 10) & 0x1;
                uint32_t i1 = ~(((instr_l >> 13) & 0x1) ^ sign);
                uint32_t i2 = ~(((instr_l >> 11) & 0x1) ^ sign);
                uint32_t offset = (i1 << 23) | (i2 << 22) | ((instr_h & 0x2f) << 12) | (instr_l & 0x3f) << 1 | 0;
                int32_t offset_signed = sign ? -offset : offset;
                return offset_signed;
            }
        } else if (thumb_instruction_size == 2) {

        }
        else {
            LOG("error: wrong instruction size");
            return 0;
        }
    }
    else {

    }

    return 0;
}
