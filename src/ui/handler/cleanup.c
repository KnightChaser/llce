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
    if (g_app_state.scan_a) {
        free_mem_regions(g_app_state.scan_a, g_app_state.scan_a_count);
    }
    if (g_app_state.scan_b) {
        free_mem_regions(g_app_state.scan_b, g_app_state.scan_b_count);
    }
    memset(&g_app_state, 0, sizeof(g_app_state));
}
