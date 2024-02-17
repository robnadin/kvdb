#include "readmemory.h"
#include "packet.h"
#include "debugger.h"
#include "target.h"
#include "hex.h"
#include "log.h"

#include <psp2kern/kernel/sysmem.h>

#include <stdlib.h>
#include <inttypes.h>

ReadMemoryCommand::ReadMemoryCommand(Debugger *debugger)
    : m_debugger(debugger)
{
}

bool ReadMemoryCommand::is(const Packet *packet) const
{
    return packet->recv_buf[0] == 'm';
}

int ReadMemoryCommand::execute(Packet *packet)
{
    auto target = m_debugger->target();

    // TODO: probably should check these
    auto addr = strtoul(packet->recv_buf+1, nullptr, 16);
    auto length = strtoul(strchr(packet->recv_buf, ',')+1, nullptr, 16);

    //LOG("0x%08x\n", addr);

    // half size because our dest write is hex characters
    // -1 because we need space for null terminator
    auto copy_length = length < (packet->size()/2-1) ? length : (packet->size()/2-1);

    // TODO: check result better
    auto res = ksceKernelMemcpyUserToKernelForPid(target->pid, packet->recv_buf, (void*)addr, copy_length);

    if (res < 0)
    {
        packet->send("E0B");
        return res;
    }
    
    hex::to_string(packet->send_buf, packet->recv_buf, copy_length);
    //auto hex = strchr(packet->send_buf, ':') + 1;

    //LOG("read memory: address: 0x%08x\n", addr);

    //LOG("read memory: payload: 0x%s\n", packet->send_buf);

    //LOG("read memory: length: %" PRIu32 "\n", length);

    return 0;
}
