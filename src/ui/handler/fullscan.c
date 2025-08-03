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

    /**
     * Free the previous second scan if it exists
     * TODO: later, we have to store three different scans:
     * - Intial scan (scan_initial)
     * - Latest-1 scan (scan_a)
     * - Latest scan (scan_b)
     */
    if (g_app_state.scan_b) {
        free_mem_regions(g_app_state.scan_b, g_app_state.scan_b_count);
        g_app_state.scan_b = NULL;
    }

    log_printf(LOG_DEFAULT, "Performing second scan on %s...\n",
               g_app_state.proc_name);
    if (full_scan(g_app_state.pid, &g_app_state.scan_b,
                  &g_app_state.scan_b_count) != 0) {
        log_printf(LOG_RED, "Failed to perform second scan.\n");
        return;
    }
    log_printf(LOG_GREEN, "Second scan complete. Found %zu regions.\n",
               g_app_state.scan_b_count);
    log_printf(LOG_YELLOW, "You can now run 'detect' to see changes.\n");
}
