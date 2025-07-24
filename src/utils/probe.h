// src/utils/probe.h
#pragma once
#include <stdint.h>
#include <sys/types.h>

// Try reading one byte at addr via process_vm_readv.
// Returns 0 on success, or errno on failure.
int probe_vm_read(pid_t pid, uintptr_t addr);

// Print a human-friendly error based on errno.
void print_probe_error(int err);
