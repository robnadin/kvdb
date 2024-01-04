#pragma once

//#define LOGGING_SCEd
//#define LOGGING_UART

//#ifdef LOGGING

#include <psp2kern/kernel/debug.h>

#ifndef NDEBUG
extern "C" int ksceDebugPrintf(const char *fmt, ...);
#define LOG(fmt, ...) ksceDebugPrintf("[kvdb] " fmt, ##__VA_ARGS__)
#else
#define LOG(fmt, ...)
#endif
//#elif defined(LOGGING_UART)
//#include "uart.h"
//#define LOG(fmt, ...) uart::printf("[kvdb] " fmt, ##__VA_ARGS__)
//#else
//#define LOG(fmt, ...)
//#endif