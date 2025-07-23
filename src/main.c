// src/main.c
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <pid>\n", prog);
}

static bool pid_exists(pid_t pid) {
    if (pid <= 0) {
        return false;
    }
    if (kill(pid, 0) == 0) {
        // Process exists
        return true;
    }

    // The PID exits, but we might lack permissions to check it.
    return errno = EPERM;
}

static bool read_first_r_region(pid_t pid, intptr_t *start, size_t *len) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/maps", pid);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("fopen");
        return false;
    }

    // /proc/<pid>/maps lines look like:
    // start-end perms offset dev inode pathname
    // e.g.: 55aa9f3f5000-55aa9f417000 r--p 00000000 08:02 131219 /usr/bin/cat
    char perms[5] = {0};
    unsigned long s, e; // s: start address, e: end address
    bool ok = false;
    while (fscanf(fp, "%lx-%lx %4s%*[^\n]\n", &s, &e, perms) == 3) {
        if (strchr(perms, 'r')) {
            *start = (intptr_t)s;
            *len = (size_t)(e - s);
            ok = true;
            break;
        }
    }

    fclose(fp);
    return ok;
}

static int probe_vm_read(pid_t pid, intptr_t addr) {
    char byte = 0;
    struct iovec local = {.iov_base = &byte, .iov_len = 1};
    struct iovec remote = {.iov_base = (void *)addr, .iov_len = 1};

    ssize_t ret = process_vm_readv(pid, &local, 1, &remote, 1, 0);
    if (ret == 1) {
        // success
        return 0;
    }
    return errno;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    char *endp = NULL;
    long v = strtol(argv[1], &endp, 10); // Convert argument to long
    if (*endp || v <= 0 || v > INT_MAX) {
        fprintf(stderr, "Invalid PID: %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    pid_t pid = (pid_t)v;

    if (!pid_exists(pid)) {
        fprintf(stderr,
                "Process with PID %d does not exist or you lack permissions.\n",
                pid);
        return EXIT_FAILURE;
    }

    intptr_t start = 0;
    size_t len = 0;
    if (!read_first_r_region(pid, &start, &len)) {
        fprintf(stderr,
                "Failed to read memory map for PID %d(=> /proc/%d/maps) (or no "
                "readable region found).\n",
                pid, pid);
        return EXIT_FAILURE;
    }

    int err = probe_vm_read(pid, start);
    if (err == 0) {
        printf("Memory read for process %d was successful: %lx\n", pid,
               (unsigned long)start);
        printf("We are ready for the further job.\n");
        return EXIT_SUCCESS;
    }

    switch (err) {
    case EPERM:
        fprintf(
            stderr,
            "[FAIL] EPERM: No permission (CAP_SYS_PTRACE / Yama / LSM?).\n");
        break;
    case ESRCH:
        fprintf(stderr, "[FAIL] ESRCH: PID vanished.\n");
        break;
    case EFAULT:
        fprintf(stderr, "[WARN] EFAULT: Address invalid. Region parsing may be "
                        "wrong; try another VMA.\n");
        break;
    default:
        fprintf(stderr, "[FAIL] process_vm_readv errno=%d (%s)\n", err,
                strerror(err));
        break;
    }
    return EXIT_FAILURE;
}
