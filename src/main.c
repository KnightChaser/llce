// src/main.c
#include <stdio.h>
#include <stdlib.h>

#include "utils/maps.h"
#include "utils/pid.h"
#include "utils/probe.h"

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

    size_t n;
    vma_t *vmas = get_vma_list(pid, &n);
    for (size_t i = 0; i < n; i++) {
        printf("%#lx-%#lx %4s %s\n", vmas[i].start, vmas[i].end, vmas[i].perms,
               vmas[i].path);
    }
    free_vma_list(vmas);

    return EXIT_FAILURE;
}
