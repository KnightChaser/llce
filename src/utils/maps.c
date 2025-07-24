// src/utils/maps.c
#include "maps.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Parse /proc/<pid>/maps and return first region with 'r'
 *
 * @param pid Process ID to check
 * @param start Pointer to store the base address of the region
 * @param len Pointer to store the length of the region
 * @return true if a readable region is found, false otherwise
 */
bool find_first_r_region(pid_t pid, uintptr_t *start, size_t *len) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/maps", pid);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        // Coult not open the file, likely due to permission issues
        perror("fopen");
        return false;
    }

    unsigned long s, e; // Start and end addresses
    char perms[5];

    /**
     * NOTE:
     * /proc/<pid>/maps lines look like:
     * start-end perms offset dev inode pathname
     * e.g.: 55aa9f3f5000-55aa9f417000 r--p 00000000 08:02 131219 /usr/bin/cat
     */
    while (fscanf(fp, "%lx-%lx %4s%*[^\n]\n", &s, &e, perms) == 3) {
        if (strchr(perms, 'r')) {
            *start = (uintptr_t)s;
            *len = (size_t)(e - s);
            fclose(fp);
            return true;
        }
    }
    fclose(fp);
    return false;
}
