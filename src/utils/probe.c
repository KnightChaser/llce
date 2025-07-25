// src/utils/probe.c
#include "probe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 *  Reads the memory map of a process from /proc/<pid>/maps.
 *
 *  @param pid The process ID to read the memory map from.
 *  @param count Pointer to store the number of VMAs read.
 *  @return A dynamically allocated array of vma_t structures containing the
 * VMAs.
 */
vma_t *get_vma_list(pid_t pid,     // [in]
                    size_t *count) // [out]
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/maps", pid);
    FILE *fp = fopen(path, "r");
    if (!fp) {
        perror("Failed to open maps file");
        return NULL;
    }

    size_t capacity = 16, index = 0;
    vma_t *list = calloc(capacity, sizeof(vma_t));
    if (!list) {
        fclose(fp);
        return NULL;
    }

    char line[PATH_MAX + 100];
    while (fgets(line, sizeof(line), fp)) {
        /**
         * NOTE:
         * Parse each line of the maps file.
         * /proc/<pid>/maps lines look like:
         * start-end perms offset dev inode pathname
         * e.g.: 55aa9f3f5000-55aa9f417000 r--p 00000000 08:02 131219
         * /usr/bin/cat
         */

        unsigned long s, e; // Start and end addresses
        char perms[5];
        if (sscanf(line, "%lx-%lx %4s %*s %*s %*s %[^\n]", &s, &e, perms,
                   list[index].path) != 4) {
            continue; // Skip malformed lines
        }

        // trim newline
        char *nl = strchr(line, '\n');
        if (nl) {
            *nl = '\0'; // Remove newline character
        }

        // crude path extraction: first '/' in the line, or empty
        char *p = strchr(line, '/');
        if (!p) {
            p = "";
        }

        if (index == capacity) {
            size_t newCapacity = capacity * 2;
            vma_t *tmp = realloc(list, newCapacity * sizeof(vma_t));
            if (!tmp) {
                free_vma_list(list);
                fclose(fp);
                return NULL; // Memory allocation failed
            }
            list = tmp;
            capacity = newCapacity;
        }

        list[index].start = (uintptr_t)s;
        list[index].end = (uintptr_t)e;
        memcpy(list[index].perms, perms, 5);
        strncpy(list[index].path, p, PATH_MAX);
        list[index].path[PATH_MAX - 1] = '\0'; // Ensure null termination

        index++;
    }
    fclose(fp);
    *count = index;
    return list;
}

/**
 *  Frees the memory allocated for a list of VMAs.
 *
 *  @param list The list of VMAs to free.
 */
void free_vma_list(vma_t *list) { free(list); }
