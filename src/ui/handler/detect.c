// src/ui/handler/detect.c
#include "../../utils/scan.h"
#include "../app_state.h"
#include "../logger.h"
#include "handler.h"
#include <stdio.h>

/**
 * Handle the 'detect' command.
 * This command compares two memory scans and detects changes between them.
 * It requires two scans to be performed first (attach and fullscan).
 */
void handle_detect(void) {
    if (!g_app_state.previous_scan || !g_app_state.current_scan) {
        log_printf(
            LOG_RED,
            "Error: Two scans are required. Use 'attach' then 'fullscan'.\n");
        return;
    }
    log_printf(LOG_DEFAULT, "Detecting changes...\n");
    mem_change_t *changes = NULL;
    size_t count = 0;
    detect_memory_changes(g_app_state.previous_scan,
                          g_app_state.previous_scan_count,
                          g_app_state.current_scan,
                          g_app_state.current_scan_count, &changes, &count);
    log_printf(LOG_GREEN, "Detected %zu changes.\n", count);
    for (size_t i = 0; i < count && i < 20; i++) { // Limit output
        printf("  -> Change at 0x%lx, value: 0x%02x -> 0x%02x\n",
               changes[i].addr, changes[i].old_value, changes[i].new_value);
    }
    if (count > 20)
        log_printf(LOG_YELLOW, "  ... (output truncated)\n");
    free_mem_changes(changes);
}
