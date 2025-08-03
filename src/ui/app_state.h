// src/ui/app_state.h
#pragma once
#include "../utils/probe.h"
#include <stdbool.h>
#include <sys/types.h>

typedef struct {
    pid_t pid;
    char proc_name[256];
    bool attached;

    // Full scan history
    mem_region_t *initial_scan;
    size_t initial_scan_count;
    mem_region_t *previous_scan;
    size_t previous_scan_count;
    mem_region_t *current_scan;
    size_t current_scan_count;
} app_state_t;

extern app_state_t g_app_state;
