// src/utils/probe.c
#define _GNU_SOURCE // For process_vm_readv
#include "probe.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>

/**
 * Try reading one byte at addr via process_vm_readv.
 * Returns 0 on success, or errno on failure.
 *
 * @param pid Process ID to read from
 * @param addr Address to read from
 * @return 0 on success, or errno on failure
 */
int probe_vm_read(pid_t pid, uintptr_t addr) {
    char tmp;
    struct iovec l = {
        .iov_base = &tmp, // Local buffer to read into
        .iov_len = 1      // Read one byte
    };
    struct iovec r = {
        .iov_base = (void *)addr, // Remote address to read from
        .iov_len = 1              // Read one byte
    };

    ssize_t ret = process_vm_readv(pid, &l, 1, &r, 1, 0);
    if (ret == 1) {
        return 0;
    }
    return errno;
}

/**
 * Print a human-friendly error based on errno, with fprintf().
 *
 * @param err Error number to interpret
 */
void print_probe_error(int err) {
    switch (err) {
    case EPERM:
        fprintf(stderr,
                "FAIL: EPERM – missing CAP_SYS_PTRACE or ptrace_scope\n");
        break;
    case ESRCH:
        fprintf(stderr, "FAIL: ESRCH – PID vanished\n");
        break;
    case EFAULT:
        fprintf(stderr, "WARN: EFAULT – invalid addr or boundary crossed\n");
        break;
    default:
        fprintf(stderr, "FAIL: errno=%d (%s)\n", err, strerror(err));
    }
}
