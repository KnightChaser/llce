// src/utils/probe.c
#define _GNU_SOURCE // For process_vm_readv
#include "probe.h"
#include <asm-generic/errno-base.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

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

/**
 *  Checks if a VMA is readable.
 *
 *  @param vma The VMA to check.
 *  @return true if the VMA is readable, false otherwise.
 */
bool is_vma_readable(const vma_t *vma) {
    if (!vma) {
        return false;
    }
    return strchr(vma->perms, 'r') != NULL;
}

/**
 *  Checks if a VMA is writable.
 *
 *  @param vma The VMA to check.
 *  @return true if the VMA is writable, false otherwise.
 */
bool is_vma_writeable(const vma_t *vma) {
    if (!vma) {
        return false;
    }
    return strchr(vma->perms, 'w') != NULL;
}

/**
 * Arguments for a thread scanning a range of VMAs in a target process.
 *
 * @typedef scan_thread_arg_t
 * @struct scan_thread_arg_t
 * @param pid         Target process ID.
 * @param vmas        Array of VMAs to scan.
 * @param regions     Array to store memory region data.
 * @param start_index Start index in the VMA array (inclusive).
 * @param end_index   End index in the VMA array (exclusive).
 */
typedef struct {
    pid_t pid;
    vma_t *vmas;
    mem_region_t *regions;
    size_t start_index;
    size_t end_index;
} scan_thread_arg_t;

/**
 * Thread function to scan a range of VMAs in a target process.
 *
 * @param arg Pointer to a scan_thread_arg_t structure containing:
 *            - pid: Target process ID.
 *            - vmas: Array of VMAs to scan.
 *            - regions: Array to store memory region data.
 *            - start_index: Start index in the VMA array.
 *            - end_index: End index (exclusive) in the VMA array.
 * @return NULL Always returns NULL.
 */
static void *scan_thread_fn(void *arg) {
    scan_thread_arg_t *a = arg;
    for (size_t i = a->start_index; i < a->end_index; i++) {
        uintptr_t base = a->vmas[i].start;
        uintptr_t end = a->vmas[i].end;
        size_t len = end - base;
        uint8_t *buf = calloc(len, sizeof(uint8_t));
        if (!buf) {
            a->regions[i].data = NULL;
            perror("Failed to allocate memory for scan buffer");
            continue;
        }

        struct iovec local = {.iov_base = buf, // local buffer
                              .iov_len = len}; // local buffer's length
        struct iovec remote = {.iov_base = (void *)base, // remote address
                               .iov_len = len};          // remote length

        ssize_t bytes_read = process_vm_readv(a->pid, &local, 1, &remote, 1, 0);
        if (bytes_read < 1) {
            free(buf);
            a->regions[i].data = NULL;
        } else {
            a->regions[i].start = base;
            a->regions[i].len = (size_t)bytes_read;
            a->regions[i].data = buf; // Store the read data
        }
    }
    return NULL;
}

/**
 *  Performs a full scan of the memory of a target process.
 *  It uses multiple threads to read all readable VMAs in the process's
 * memory.
 *
 *  @param pid The process ID to scan.
 *  @param regions_out Pointer to store the array of memory regions found.
 *  @param count_out Pointer to store the number of memory regions found.
 *  @return 0 on success, or an error code on failure.
 */
int full_scan(pid_t pid,                  // [in]
              mem_region_t **regions_out, // [out]
              size_t *count_out           // [out]
) {
    // Get the list of VMAs for the target process
    size_t vma_count = 0;
    vma_t *vmas = get_vma_list(pid, &vma_count);
    if (!vmas) {
        return ENOMEM;
    }

    // Filter both readable and writeable VMAs to modify the regions later upon
    // the user's request
    size_t region_count = 0;
    for (size_t i = 0; i < vma_count; i++) {
        if (is_vma_readable(&vmas[i]) && is_vma_writeable(&vmas[i])) {
            region_count++;
        }
    }

    // Prepare the arrays to do the full scan
    mem_region_t *regions = calloc(region_count, sizeof(*regions));
    vma_t *filters = calloc(region_count, sizeof(*filters));
    if (!regions || !filters) {
        free(vmas);
        free(regions);
        free(filters);
        return ENOMEM;
    }

    // Copy both readable and writeable VMAs into filters[]
    size_t index = 0;
    for (size_t i = 0; i < vma_count; i++) {
        if (is_vma_readable(&vmas[i]) && is_vma_writeable(&vmas[i])) {
            filters[index] = vmas[i];
            index++;
        }
    }
    free(vmas);

    // Spawn thrads across cores
    long procs = sysconf(_SC_NPROCESSORS_ONLN);
    size_t num_threads = (procs > 0 ? (size_t)procs : 1);
    if (num_threads > region_count) {
        num_threads = region_count;
    }

    pthread_t *threads = calloc(num_threads, sizeof(*threads));
    scan_thread_arg_t *args = calloc(num_threads, sizeof(*args));
    if (!threads || !args) {
        free(filters);
        free(regions);
        free(threads);
        free(args);
        return ENOMEM;
    }

    // Create the thread and wait for them to finish
    for (size_t t = 0; t < num_threads; t++) {
        size_t start = t * (region_count / num_threads);
        size_t end = (t + 1) * (region_count / num_threads);
        // NOTE: all threads share the same region array, but each thread will
        // only read non-overlapping regions.
        args[t] = (scan_thread_arg_t){.pid = pid,
                                      .vmas = filters,
                                      .regions = regions,
                                      .start_index = start,
                                      .end_index = end};
        pthread_create(&threads[t], NULL, scan_thread_fn, &args[t]);
    }

    for (size_t t = 0; t < num_threads; t++) {
        pthread_join(threads[t], NULL);
    }

    // Clean up
    free(threads);
    free(args);
    free(filters);

    // Set output parameters
    *regions_out = regions;
    *count_out = region_count;

    return 0; // Success
}

/**
 *  Frees the memory allocated for an array of memory regions.
 *
 *  @param regions The array of memory regions to free.
 *  @param count The number of regions in the array.
 */
void free_mem_regions(mem_region_t *regions, size_t count) {
    if (!regions) {
        return;
    }
    for (size_t i = 0; i < count; i++) {
        // Free each region's data buffer
        free(regions[i].data);
    }

    // Finally, free the regions array itself
    free(regions);
}
