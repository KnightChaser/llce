// src/utils/pid.h
#pragma once
#include <stdbool.h>
#include <sys/types.h>

// Convert argv[1] to pid_t or exits on invalid
pid_t pid_from_argv(const char *arg);
// Get the process name from /proc/<pid>/comm
bool get_proc_name(pid_t pid, char *name_buf, size_t buf_size);
// Check if PID exits or we'd at least get EPERM(permission denied)
bool pid_exists(pid_t pid);
