// src/ui/handler/fullscan.c
#include "../../utils/probe.h"
#include "../app_state.h"
#include "../logger.h"
#include "handler.h"
#include <stdio.h>

/**
 * Handle the 'fullscan' command.
 * This command performs a second memory scan on the attached process.
 * It requires the user to have already attached to a process using 'attach'.
 * The second scan is used to compare against the initial scan.
 */
void handle_fullscan(void) {
    if (!g_app_state.attached) {
        log_printf(LOG_RED,
                   "You must attach to a process first using 'attach'.\n");
        return;
    }

    // Run new scan
    mem_region_t *new_buf = NULL;
    size_t new_count = 0;
    log_printf(LOG_DEFAULT, "Performing next scan on %s... (PID: %d)\n",
               g_app_state.proc_name, g_app_state.pid);
    if (full_scan(g_app_state.pid, &new_buf, &new_count) != 0) {
        log_printf(LOG_RED, "Failed to perform the fullscan.\n");
        return;
    }

    // Shift history
    if (g_app_state.current_scan) {
        // free old previous scan result, unless it's the initial scan
        if (g_app_state.previous_scan &&
            g_app_state.previous_scan != g_app_state.initial_scan) {
            free_mem_regions(g_app_state.previous_scan,
                             g_app_state.previous_scan_count);
        }
        g_app_state.previous_scan = g_app_state.current_scan;
        g_app_state.previous_scan_count = g_app_state.current_scan_count;
    } else {
        // This is the firrst full scan, so, previous = initial
        g_app_state.previous_scan = g_app_state.initial_scan;
        g_app_state.previous_scan_count = g_app_state.initial_scan_count;
    }

    // Install new as current
    g_app_state.current_scan = new_buf;
    g_app_state.current_scan_count = new_count;
    log_printf(LOG_GREEN,
               "Full scan completed successfully. %zu regions found.\n",
               g_app_state.current_scan_count);

    log_printf(LOG_YELLOW, "You can now run 'detect' to see changes.\n");
}
