// src/utils/probe.h
#pragma once
#include <linux/limits.h>
#include <stdint.h>
#include <sys/types.h>

// Single VMA entry from /proc/<pid>/maps
typedef struct {
    uintptr_t start;
    uintptr_t end;
    char perms[5];
    char path[PATH_MAX];
} vma_t;

vma_t *get_vma_list(pid_t pid, size_t *count);

void free_vma_list(vma_t *list);
