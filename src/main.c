// src/main.c
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

    // NOTE: Perform a full scan of the process memory
    mem_region_t *old_scan, *new_scan;
    size_t old_n, new_n;

    if (full_scan(pid, &old_scan, &old_n) != 0) {
        fprintf(stderr, "Failed to perform full scan on PID %d\n", pid);
        return EXIT_FAILURE;
    }

    usleep(1000000); // 1 sec

    if (full_scan(pid, &new_scan, &new_n) != 0) {
        fprintf(stderr, "Failed to perform subsequent memory scan on PID %d\n",
                pid);
        free_mem_regions(old_scan, old_n);
        return EXIT_FAILURE;
    }

    mem_change_t *changes = NULL;
    size_t changes_count;
    detect_memory_changes(old_scan, old_n, new_scan, new_n, &changes,
                          &changes_count);
    printf("Detected %zu changes in memory regions of PID %d:\n", changes_count,
           pid);
    for (size_t i = 0; i < changes_count; i++) {
        printf("Change at address: 0x%lx, value: 0x%02x -> 0x%02x\n",
               changes[i].addr, changes[i].old_value, changes[i].new_value);
        if (i > 10) {
            printf("... (truncated)\n");
            break; // Limit output to first 10 changes
        }
    }
    printf("\n");

    // NOTE: Search values
    uint32_t search_value = 100;
    scan_result_t *results = NULL;
    size_t result_count = 0;
    printf("Scanning PID %d for value: %u\n", pid, search_value);

    int err = search_compare(old_scan, old_n, SCAN_TYPE_DWORD, CMP_EQ,
                             &search_value, &results, &result_count);
    if (err == 0) {
        printf("Found %zu matches for value %u:\n", result_count, search_value);
        for (size_t i = 0; i < result_count; i++) {
            printf("Match at address: 0x%lx, length: %zu\n", results[i].addr,
                   results[i].len);
            if (i > 10) {
                printf("... (truncated)\n");
                break; // Limit output to first 10 matches
            }
        }
    } else {
        fprintf(stderr, "Failed to search for value %u in PID %d: %d\n",
                search_value, pid, err);
    }

    // NOTE: Clean up all allocated memory
    free_mem_changes(changes);
    free_mem_regions(old_scan, old_n);
    free_mem_regions(new_scan, new_n);

    return EXIT_SUCCESS;
}
