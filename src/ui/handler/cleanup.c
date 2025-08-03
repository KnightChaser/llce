// src/ui/handler/cleanup.c
#include "../app_state.h"
#include "handler.h"
#include <memory.h>

/**
 * Cleanup the application state and free allocated memory.
 * This function is called when the user exits the application or when
 * a new process is attached.
 */
void cleanup_app_state(void) {
    if (g_app_state.current_scan) {
        free_mem_regions(g_app_state.current_scan,
                         g_app_state.current_scan_count);
    }
    if (g_app_state.previous_scan &&
        g_app_state.previous_scan != g_app_state.initial_scan) {
        // WARNING: If the previous scan is not the same as the initial scan,
        // free it separately.
        free_mem_regions(g_app_state.previous_scan,
                         g_app_state.previous_scan_count);
    }

    if (g_app_state.initial_scan) {
        free_mem_regions(g_app_state.initial_scan,
                         g_app_state.initial_scan_count);
    }
    memset(&g_app_state, 0, sizeof(g_app_state));
}
