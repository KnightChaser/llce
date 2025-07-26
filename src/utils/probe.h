// src/utils/probe.h
#pragma once
#include <linux/limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

// Single VMA entry from /proc/<pid>/maps
typedef struct {
    uintptr_t start;     // region base address
    uintptr_t end;       // region end address
    char perms[5];       // permissions (e.g., "r--p")
    char path[PATH_MAX]; // path to the mapped file (if any)
} vma_t;

vma_t *get_vma_list(pid_t pid, size_t *count);
void free_vma_list(vma_t *list);
bool is_vma_readable(const vma_t *vma);
bool is_vma_writeable(const vma_t *vma);

// Memory-blob structure for the full scan
typedef struct {
    uintptr_t start; // region base
    size_t len;      // bytes actually read
    uint8_t *data;   // malloc'd buffer
} mem_region_t;

int full_scan(pid_t pid, mem_region_t **regions, size_t *count);
void free_mem_regions(mem_region_t *regions, size_t count);
