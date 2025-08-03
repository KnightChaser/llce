// src/ui/app_state.h
#pragma once
#include "../utils/probe.h"
#include <stdbool.h>
#include <sys/types.h>

typedef struct {
    pid_t pid;
    char proc_name[256];
    bool attached;

    // We need two slots to hold scans for comparison.
    mem_region_t *scan_a;
    size_t scan_a_count;
    mem_region_t *scan_b;
    size_t scan_b_count;
} app_state_t;

extern app_state_t g_app_state;
