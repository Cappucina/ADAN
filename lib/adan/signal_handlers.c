#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "logs.h"

#ifndef BUILDING_COMPILER_MAIN

static void runtime_signal_handler(int signum, siginfo_t* info, void* ctx) {
    (void)ctx;
    void* addr = info ? info->si_addr : NULL;

    switch (signum) {
        case SIGSEGV:
            log_error(NULL, 0, 0, "Segmentation fault at address %p. This is often caused by a stack overflow (infinite recursion) or an invalid memory access. Try limiting recursion or inspecting stack allocations.", addr);
            break;
        case SIGBUS:
            log_error(NULL, 0, 0, "Bus error (SIGBUS) at address %p: possible unaligned or invalid memory access.", addr);
            break;
        case SIGFPE:
            log_error(NULL, 0, 0, "Floating point exception (SIGFPE): arithmetic error (e.g., division by zero or overflow).\n");
            break;
        case SIGILL:
            log_error(NULL, 0, 0, "Illegal instruction (SIGILL): likely corrupt code or invalid instruction.");
            break;
        default:
            log_error(NULL, 0, 0, "Unhandled signal %d received.", signum);
            break;
    }

    _exit(128 + signum);
}

static void __attribute__((constructor)) init_signal_handlers(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = runtime_signal_handler;
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
}

#endif
