// src/main.c
#include "utils/maps.h"
#include "utils/pid.h"
#include "utils/probe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Explains how to use the program.
 */
static void usage(const char *prog) {
    fprintf(stderr, "Usage: %s <pid>\n", prog);
}

// NOTE: Still under development, it just does
// a simple probe to check if process_vm_readv works
int main(int argc, char **argv) {
    if (argc != 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    pid_t pid = pid_from_argv(argv[1]);
    if (!pid_exists(pid)) {
        fprintf(stderr, "ERR: PID %d not found or accessible\n", pid);
        return EXIT_FAILURE;
    }

    uintptr_t start;
    size_t len;
    if (!find_first_r_region(pid, &start, &len)) {
        fprintf(stderr, "ERR: no readable region in /proc/%d/maps\n", pid);
        return EXIT_FAILURE;
    }

    // Perform a full scan of the process memory
    mem_region_t *regions;
    size_t rcount;
    int err = full_scan(pid, &regions, &rcount);
    if (err) {
        fprintf(stderr, "full_scan failed: %s\n", strerror(err));
        return EXIT_FAILURE;
    }

    printf("[+] Scanned %zu RW regions:\n", rcount);
    for (size_t i = 0; i < rcount; i++) {
        if (regions[i].data) {
            printf("  [%2zu] %#lx–%#lx (%zu bytes)\n", i, regions[i].start,
                   regions[i].start + regions[i].len, regions[i].len);
        } else {
            printf("  [%2zu] %#lx–%#lx  <read failed>\n", i, regions[i].start,
                   regions[i].start + regions[i].len);
        }
    }

    free_mem_regions(regions, rcount);
    return EXIT_SUCCESS;
}
