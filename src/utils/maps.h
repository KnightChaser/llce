// src/utils/maps.h
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

// Parse /proc/<pid>/maps and return first region with ‘r’
// start: base address, len: region length
bool find_first_r_region(pid_t pid, uintptr_t *start, size_t *len);
