#include "swbreak.h"
#include "packet.h"
#include "debugger.h"
#include "target.h"
#include "hex.h"
#include "log.h"
#include "utils.h"
#include "arm.h"

#include <psp2kern/kernel/sysmem.h>

#include <stdlib.h>
#include <inttypes.h>
#include <string>

SWBreakCommand::SWBreakCommand(Debugger *debugger)
: m_debugger(debugger)
{
}

bool SWBreakCommand::is(const Packet *packet) const
{
    return begins_with(packet->recv_buf, "Z0");
}

int SWBreakCommand::execute(Packet *packet)
{
    auto target = m_debugger->target();

    auto addr = strtoul(strchr(packet->recv_buf, ',') + 1, nullptr, 16);
    auto length = strtoul(strchr(strchr(packet->recv_buf, ',') + 1, ',') + 1, nullptr, 16);

    LOG("swbreak: address: 0x%08x\n", addr);

    LOG("swbreak: length: %" PRIu32 "\n", length);

    if (m_debugger->m_sw_breakpoint_count < 16) {
        SWBreak& breakpt = m_debugger->m_sw_breakpoints[m_debugger->m_sw_breakpoint_count];
        breakpt.addr = addr;
        breakpt.size = length;

        if (length == 2) {
            ksceKernelMemcpyUserToKernelForPid(target->pid, &breakpt.inst, (void*)addr, 2);
            ksceKernelRxMemcpyKernelToUserForPid(target->pid, (void*)addr, &arm::THUMB_INST_BKPT_16_BIT, 2);
        }
        else if (length == 4) {
            ksceKernelMemcpyUserToKernelForPid(target->pid, &breakpt.inst, (void*)addr, 4);
            ksceKernelRxMemcpyKernelToUserForPid(target->pid, (void*)addr, &arm::ARM_INST_BKPT, 4);
        }

        ++m_debugger->m_sw_breakpoint_count;
    }

    packet->send("OK");
    return 0;
}
