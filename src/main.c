// src/main.c
#include "utils/maps.h"
#include "utils/pid.h"
#include "utils/probe.h"
#include "utils/scan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

    mem_region_t *old_scan, *new_scan;
    size_t old_n, new_n;
    full_scan(pid, &old_scan, &old_n);
    usleep(100000); // Sleep for 100ms to simulate some time passing
    full_scan(pid, &new_scan, &new_n);

    uintptr_t *changes;
    size_t changes_count;
    detect_memory_changes(old_scan, old_n, new_scan, new_n, &changes,
                          &changes_count);

    printf("Detected %zu changes in memory:\n", changes_count);
    for (size_t i = 0; i < changes_count; i++) {
        printf("Change at address: 0x%lx\n", changes[i]);
    }

    free(changes);
    free_mem_regions(old_scan, old_n);
    free_mem_regions(new_scan, new_n);

    return EXIT_SUCCESS;
}
