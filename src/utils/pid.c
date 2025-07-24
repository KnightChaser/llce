// src/utils/pid.c
#include "pid.h"
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Convert argv[1] to pid_t or exits on invalid
 *
 * @param arg Argument string to Convert
 * @return pid_t Process ID
 */
pid_t pid_from_argv(const char *arg) {
    char *end;
    long v = strtol(arg, &end, 10);
    if (*end || v <= 0 || v > INT_MAX) {
        fprintf(stderr, "Invalid PID: %s\n", arg);
        exit(EXIT_FAILURE);
    }
    return (pid_t)v;
}

/**
 * Check if PID exists or we'd at least get EPERM (permission denied)
 *
 * @param pid Process ID to check
 * @return true if PID exists, false otherwise
 */
bool pid_exists(pid_t pid) {
    if (kill(pid, 0) == 0) {
        return true;
    }
    return errno == EPERM;
}
