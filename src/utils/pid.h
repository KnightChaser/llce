// src/utils/pid.h
#pragma once
#include <stdbool.h>
#include <sys/types.h>

// Convert argv[1] to pid_t or exits on invalid
pid_t pid_from_argv(const char *arg);

// Check if PID exits or we'd at least get EPERM(permission denied)
bool pid_exists(pid_t pid);
