#include "step.h"
#include "packet.h"
#include "debugger.h"
#include "log.h"
#include "arm.h"

#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/processmgr.h>

#include <stdlib.h>
#include <string.h>

StepCommand::StepCommand(Debugger *debugger)
    : m_debugger(debugger)
{
}

bool StepCommand::is(const Packet *packet) const
{
    return packet->recv_buf[0] == 's';
}

int StepCommand::execute(Packet *packet)
{
    LOG("step\n");
    //auto addr = 0u;

    //if (strlen(packet->recv_buf) > 1)
    //{
    //    addr = strtoul(packet->recv_buf+1, nullptr, 16);
    //}

    auto target = m_debugger->target();
    ThreadCpuRegisters register_sets;
    ksceKernelGetThreadCpuRegisters(target->excpt_tid, &register_sets);

    auto registers = (register_sets.user.cpsr & 0x1F) == 0x10 ? (&register_sets.user) : (&register_sets.kernel);

    auto pc_addr = registers->pc;

    if (arm::isThumb(register_sets))
    {
        if (pc_addr == m_debugger->m_pc_prev_addr) {
            LOG("thumb: restore instruction as step didn't work\n");
            LOG("thumb: addr: 0x%08x\n", m_debugger->m_pc_addr);
            LOG("thumb: instruction: 0x%08x\n", m_debugger->m_instruction);
            ksceKernelRxMemcpyKernelToUserForPid(target->pid, (void*)m_debugger->m_pc_addr, &m_debugger->m_instruction, 4);
        }

        // TODO: error check better
        auto instruction = 0u;
        auto res = ksceKernelMemcpyUserToKernelForPid(target->pid, &instruction, (void*)pc_addr, 2);

        if ((instruction & 0xffff) == 0xffff) {
            LOG("thumb: restore instruction as gdb set it to 0xffff\n");
            LOG("thumb: addr: 0x%08x\n", m_debugger->m_pc_addr);
            LOG("thumb: instruction: 0x%08x\n", m_debugger->m_instruction);
            ksceKernelRxMemcpyKernelToUserForPid(target->pid, (void*)m_debugger->m_pc_addr, &m_debugger->m_instruction, 4);
            ksceKernelMemcpyUserToKernelForPid(target->pid, &instruction, (void*)pc_addr, 2);
        }

        m_debugger->m_pc_prev_addr = pc_addr;

        //LOG("thumb: 0x%08x\n", instruction);

        uint32_t instruction_size = arm::getThumbInstructionSize(instruction);

        uintptr_t current_instruction = pc_addr;
        LOG("thumb: current instruction: 0x%08x\n", current_instruction);
        LOG("thumb instruction size: %" PRIu32 "\n", instruction_size);
        res = ksceKernelMemcpyUserToKernelForPid(target->pid, &instruction, (void*)pc_addr, instruction_size);
        uintptr_t next_instruction = arm::getNextInstructionAddr(target->pid, register_sets, instruction);

        if (pc_addr != next_instruction) {
            m_debugger->m_pc_addr = next_instruction;

            LOG("thumb: next instruction: 0x%08x\n", next_instruction);

            res = ksceKernelMemcpyUserToKernelForPid(target->pid, &instruction, (void*)m_debugger->m_pc_addr, 4);
            LOG("thumb: 0x%08x\n", instruction);

            m_debugger->m_instruction = instruction;

            uint32_t undef = 0xffffffff;
            res = ksceKernelRxMemcpyKernelToUserForPid(target->pid, (void*)m_debugger->m_pc_addr, &undef, 4);
            //LOG("thumb: write step ret: %d\n", res);

            if (res < 0)
            {
                LOG("step failed: %d\n", res);
                packet->send("E0B");
                return res;
            }
        }
    }
    else
    {
        // TODO: error check better
        auto instruction = 0u;
        auto res = ksceKernelMemcpyUserToKernelForPid(target->pid, &instruction, (void*)pc_addr, 4);

        LOG("arm: 0x%04x\n", instruction);

        if (m_debugger->m_pc_addr != pc_addr + 4) {
            m_debugger->m_pc_addr = pc_addr + 4;

            res = ksceKernelMemcpyUserToKernelForPid(target->pid, &instruction, (void*)m_debugger->m_pc_addr, 4);
            LOG("arm: 0x%04x\n", instruction);

            m_debugger->m_instruction = instruction;

            uint32_t undef = 0xffffffff;
            res = ksceKernelRxMemcpyKernelToUserForPid(target->pid, (void*)m_debugger->m_pc_addr, &undef, 4);
            LOG("arm: write step ret: %d\n", res);

            if (res < 0)
            {
                LOG("step failed: %d\n", res);
                packet->send("E0B");
                return res;
            }
        }
    }

    LOG("step successful\n");

    ksceKernelChangeThreadSuspendStatus(target->excpt_tid, 2);
    ksceKernelResumeProcess(target->pid);

    ksceKernelSuspendProcess(target->pid, 0x11);

    packet->send("S05");

    return 0;
}

unsigned int StepCommand::next_address(unsigned int current_address)
{
    return 0; 
}