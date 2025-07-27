// src/utils/pid.c
#include "pid.h"
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 * Get the process name from /proc/<pid>/comm
 *
 * @param pid Process ID to get the name for
 * @param name_buf Buffer to store the process name
 * @param buf_size Size of the buffer
 * @return true on success, false on failure
 */
bool get_proc_name(pid_t pid, char *name_buf, size_t buf_size) {
    char path[64] = {0};
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("Failed to open process comm file");
        return false;
    }

    if (fgets(name_buf, buf_size, fp) == NULL) {
        // Remove trailing newline if it exists
        name_buf[strcspn(name_buf, "\n")] = '\0';
        fclose(fp);
        return true;
    }

    fclose(fp);
    return true;
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
