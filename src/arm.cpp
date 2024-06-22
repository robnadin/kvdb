#include "arm.h"
#include "log.h"

#include <cinttypes>

#include <psp2kern/kernel/sysmem/data_transfers.h>

bool arm::isThumb(SceThreadCpuRegisters const& registers) {
    auto regs = (registers.user.cpsr & 0x1F) == 0x10 ? (&registers.user) : (&registers.kernel);
    return (regs->cpsr & (1 << 5)) != 0;
}

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

uintptr_t arm::getNextInstructionAddr(SceUID pid, SceThreadCpuRegisters const& registers, Instruction instruction) {
    auto regs = (registers.user.cpsr & 0x1F) == 0x10 ? (&registers.user) : (&registers.kernel);
    return regs->pc + getNextInstructionOffset(pid, registers, instruction);
}

int32_t arm::getNextInstructionOffset(SceUID pid, SceThreadCpuRegisters const& registers, Instruction instruction) {
    int32_t instruction_offset = 0;
    if (isThumb(registers)) {
        ThumbInstruction thumb_instruction = instruction;
        uint32_t thumb_instruction_size = getThumbInstructionSize(thumb_instruction);
        instruction_offset = thumb_instruction_size;
        if (thumb_instruction_size == 4) {
            //LOG("BL hex: 0x%08x\n", THUMB_INST_BL_32_BIT_MASK);
            /*if ((thumb_instruction & THUMB_INST_BL_32_BIT_MASK) == THUMB_INST_BL_32_BIT_MASK) {
                LOG("thumb BL instruction\n");
                uint16_t instr_h = thumb_instruction & 0xffff;
                uint16_t instr_l = (thumb_instruction >> 16) & 0xffff;
                LOG("thumb instr high: %04x instr low: %04x\n", instr_h, instr_l);
                uint8_t sign = (instr_h >> 10) & 0x1;
                uint32_t i1 = ~(((instr_l >> 13) & 0x1) ^ sign) & 0x1;
                uint32_t i2 = ~(((instr_l >> 11) & 0x1) ^ sign) & 0x1;
                uint32_t offset = ((i1 & 0x1) << 23) | ((i2 & 0x1) << 22) | ((instr_h & 0x3ff) << 12) | (instr_l & 0x7ff) << 1 | 0;

                LOG("thumb sign: %01x i1: %04x i2: %04x offset: %" PRIu32 "\n", sign, i1, i2, offset);

                int32_t offset_signed = sign ? -offset : offset;
                LOG("thumb BL offset:%" PRIi32 "\n", offset_signed);
                instruction_offset = 4 + offset_signed;
            }
            else if ((thumb_instruction & THUMB_INST_BLX_32_BIT_MASK) == THUMB_INST_BLX_32_BIT_MASK) {
                LOG("thumb BLX instruction\n");
                uint16_t instr_h = thumb_instruction & 0xffff;
                uint16_t instr_l = (thumb_instruction >> 16) & 0xffff;
                LOG("thumb instr high: %04x instr low: %04x\n", instr_h, instr_l);
                uint8_t sign = (instr_h >> 10) & 0x1;
                uint32_t i1 = ~(((instr_l >> 13) & 0x1) ^ sign) & 0x1;
                uint32_t i2 = ~(((instr_l >> 11) & 0x1) ^ sign) & 0x1;
                uint32_t offset = ((i1 & 0x1) << 23) | ((i2 & 0x1) << 22) | ((instr_h & 0x3ff) << 12) | (instr_l & 0x3ff) << 2 | 0;

                LOG("thumb sign: %01x i1: %04x i2: %04x offset: %" PRIu32 "\n", sign, i1, i2, offset);

                int32_t offset_signed = sign ? -offset : offset;
                LOG("thumb BL offset:%" PRIi32 "\n", offset_signed);
                instruction_offset = 4 + offset_signed;
            }
            else*/ 
            if ((thumb_instruction & THUMB_INST_B_32_BIT_MASK) == THUMB_INST_B_32_BIT_MASK) {
                uint16_t instr_h = thumb_instruction & 0xffff;
                uint16_t instr_l = (thumb_instruction >> 16) & 0xffff;
                LOG("thumb instr high: %04x instr low: %04x\n", instr_h, instr_l);
                uint8_t sign = (instr_h >> 10) & 0x1;
                uint8_t cond = (instr_h >> 6) & 0xf;
                uint8_t imm6 = instr_h & 0x1f;
                uint16_t imm11 = instr_l & 0x7ff;
                uint32_t j1 = ~(((instr_l >> 13) & 0x1) ^ sign) & 0x1;
                uint32_t j2 = ~(((instr_l >> 11) & 0x1) ^ sign) & 0x1;
                uint32_t offset = ((j2 & 0x1) << 19) | ((j1 & 0x1) << 18) | ((imm6 & 0x1f) << 12) | (imm11 & 0x7ff) << 1 | 0;

                int32_t offset_signed = sign ? -offset : offset;
                LOG("thumb BL offset:%" PRIi32 "\n", offset_signed);
                instruction_offset = 4 + (handleCondition(registers, cond) ? 0 : offset_signed);
            }
            else if ((thumb_instruction & THUMB_INST2_B_32_BIT_MASK) == THUMB_INST2_B_32_BIT_MASK) {
                uint16_t instr_h = thumb_instruction & 0xffff;
                uint16_t instr_l = (thumb_instruction >> 16) & 0xffff;
                LOG("thumb instr high: %04x instr low: %04x\n", instr_h, instr_l);
                uint8_t sign = (instr_h >> 10) & 0x1;
                uint8_t imm10 = instr_h & 0x3ff;
                uint16_t imm11 = instr_l & 0x7ff;
                uint32_t i1 = ~(((instr_l >> 13) & 0x1) ^ sign) & 0x1;
                uint32_t i2 = ~(((instr_l >> 11) & 0x1) ^ sign) & 0x1;
                uint32_t offset = ((i1 & 0x1) << 23) | ((i2 & 0x1) << 22) | ((imm10 & 0x3ff) << 12) | (imm11 & 0x7ff) << 1 | 0;

                int32_t offset_signed = sign ? -offset : offset;
                LOG("thumb BL offset:%" PRIi32 "\n", offset_signed);
                instruction_offset = 4 + offset_signed;
            }
        } else if (thumb_instruction_size == 2) {
            if ((thumb_instruction & THUMB_INST_B_16_BIT_MASK) == THUMB_INST_B_16_BIT_MASK) {
                LOG("thumb B instruction\n");
                uint8_t imm8 = thumb_instruction & 0xff;
                uint8_t cond = (thumb_instruction >> 8) & 0xf;
                uint32_t offset = ((imm8 & 0xff) << 1) | 0;
                int32_t offset_signed = (imm8 & (1 << 7)) ? -offset : offset;
                LOG("thumb B offset:%" PRIi32 "\n", offset_signed);
                instruction_offset = (handleCondition(registers, cond) ? offset_signed + 4 : instruction_offset);
            }
            else if ((thumb_instruction & THUMB_INST2_B_16_BIT_MASK) == THUMB_INST2_B_16_BIT_MASK) {
                LOG("thumb B 2 instruction\n");
                uint16_t imm11 = thumb_instruction & 0x7ff;
                uint32_t offset = ((imm11 & 0x7ff) << 1) | 0;
                int32_t offset_signed = (imm11 & (1 << 10)) ? -offset : offset;
                LOG("thumb B 2 offset:%" PRIi32 "\n", offset_signed);
                instruction_offset = 4 + offset_signed;
            }
            else if ((thumb_instruction & THUMB_INST_IT_16_BIT) == THUMB_INST_IT_16_BIT) {
                LOG("thumb IT instruction\n");
                uint8_t firstcond = (thumb_instruction >> 4) & 0xf;
                uint8_t mask = thumb_instruction & 0xf;
                if (mask != 0) {
                    bool res = handleCondition(registers, firstcond);

                    uint8_t mask_copy = mask;
                    uint8_t instruction_count = 0;
                    while ((mask_copy & 0xf) != 0) {
                        mask_copy = mask_copy << 1;
                        ++instruction_count;
                    }

                    LOG("thumb IT block instruction count: %" PRIu8 "\n", instruction_count);

                    instruction_offset += res ? 0 : instruction_count * 2;
                }
            }
            else if ((thumb_instruction & THUMB_INST_POP_16_BIT) == THUMB_INST_POP_16_BIT) {
                LOG("thumb POP instruction\n");
                uint8_t P = (thumb_instruction >> 7) & 0x1;
                if (P != 0) {
                    uint8_t register_list = thumb_instruction & 0xff;
                    uint8_t bit_count = 0;
                    while (register_list != 0) {
                        if (register_list & 0x1) {
                            ++bit_count;
                        }
                        register_list = register_list >> 1;
                    }

                    auto regs = (registers.user.cpsr & 0x1F) == 0x10 ? (&registers.user) : (&registers.kernel);
                    auto sp = regs->sp;
                    uintptr_t next_instruction_addr = 0;
                    ksceKernelMemcpyUserToKernelForPid(pid, &next_instruction_addr, (void*)(sp + bit_count * 4), 4);
                    LOG("thumb POP: restored pc: 0x%08x\n", next_instruction_addr - 1);
                    instruction_offset = next_instruction_addr - 1 - regs->pc;
                }
            }
        }
        else {
            LOG("error: wrong instruction size\n");
            return 0;
        }
    }
    else {

    }

    return instruction_offset;
}

bool arm::handleCondition(SceThreadCpuRegisters const& registers, uint8_t condition) {
    auto regs = (registers.user.cpsr & 0x1F) == 0x10 ? (&registers.user) : (&registers.kernel);
    uint32_t N = (regs->cpsr >> 31) & 0x1;
    uint32_t Z = (regs->cpsr >> 30) & 0x1;
    uint32_t C = (regs->cpsr >> 29) & 0x1;
    uint32_t V = (regs->cpsr >> 28) & 0x1;
    LOG("cpsr reg: 0x%08x\n", regs->cpsr);
    bool res = false;
    switch(condition & 0xf) {
    case 0x0:
        res = (Z == 1);
        LOG("Equal: %s\n", res ? "true" : "false");
        break;
    case 0x1:
        res = (Z == 0);
        LOG("Not Equal: %s\n", res ? "true" : "false");
        break;
    case 0x2:
        res = (C == 1);
        LOG("Carry Set: %s\n", res ? "true" : "false");
        break;
    case 0x3:
        res = (C == 0);
        LOG("Carry Clear: %s\n", res ? "true" : "false");
        break;
    case 0x4:
        res = (N == 1);
        LOG("Minus, Negative: %s\n", res ? "true" : "false");
        break;
    case 0x5:
        res = (N == 0);
        LOG("Plus, Positive or Zero: %s\n", res ? "true" : "false");
        break;
    case 0x6:
        res = (V == 1);
        LOG("Overflow: %s\n", res ? "true" : "false");
        break;
    case 0x7:
        res = (V == 0);
        LOG("No Overflow: %S\n", res ? "true" : "false");
        break;
    case 0x8:
        res = (C == 1 && Z == 0);
        LOG("Unsigned Higher: %s\n", res ? "true" : "false");
        break;
    case 0x9:
        res = (C == 0 || Z == 1);
        LOG("Unsigned Lower or Same: %s\n", res ? "true" : "false");
        break;
    case 0xA:
        res = (N == V);
        LOG("Signed Greater Than or Equal: %s\n", res ? "true" : "false");
        break;
    case 0xB:
        res = (N != V);
        LOG("Signed Less Than: %s\n", res ? "true" : "false");
        break;
    case 0xC:
        res = (Z == 0 && N == V);
        LOG("Signed Greater Than: %s\n", res ? "true" : "false");
        break;
    case 0xD:
        res = (Z == 1 || N != V);
        LOG("Signed Less Than or Equal: %s\n", res ? "true" : "false");
        break;
    case 0xE:
        res = true;
        LOG("error: wrong instruction size\n");
        break;
    }
    return res;
}