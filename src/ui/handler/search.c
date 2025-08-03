// src/ui/handler/search.c
#include "../../utils/scan.h"
#include "../app_state.h"
#include "../logger.h"
#include "handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Handle the 'search' command.
 * This command allows the user to search for a specific value in the scan data.
 *
 * @param type_str The type of value to search for (byte, word, dword, qword).
 * @param value_str The value to search for, as a string.
 */
void handle_search(char *type_str, char *value_str) {
    // Decide which memory-snapshot to search.
    // If no scan is available, we can't search.
    // If any exist, use the most recent one.
    mem_region_t *regions;
    size_t regions_count;
    if (g_app_state.current_scan) {
        regions = g_app_state.current_scan;
        regions_count = g_app_state.current_scan_count;
    } else if (g_app_state.previous_scan) {
        regions = g_app_state.previous_scan;
        regions_count = g_app_state.previous_scan_count;
    } else if (g_app_state.initial_scan) {
        regions = g_app_state.initial_scan;
        regions_count = g_app_state.initial_scan_count;
    } else {
        log_printf(LOG_RED,
                   "No scan data available. Please perform a scan first.\n");
        return;
    }

    if (!type_str || !value_str) {
        log_printf(LOG_RED, "Usage: search <type> <value>\n");
        log_printf(LOG_YELLOW, "Types: byte, word, dword, qword\n");
        return;
    }

    scan_type_t type;
    if (strcmp(type_str, "byte") == 0) {
        type = SCAN_TYPE_BYTE;
    } else if (strcmp(type_str, "word") == 0) {
        type = SCAN_TYPE_WORD;
    } else if (strcmp(type_str, "dword") == 0) {
        type = SCAN_TYPE_DWORD;
    } else if (strcmp(type_str, "qword") == 0) {
        type = SCAN_TYPE_QWORD;
    } else {
        log_printf(LOG_RED, "Unknown search type: %s\n", type_str);
        return;
    }

    uint64_t value = strtoull(value_str, NULL, 0); // Base 0 auto-detects 0x hex

    scan_result_t *results = NULL;
    size_t count = 0;
    search_compare(regions,       // Memory regions to search
                   regions_count, // Number of regions
                   type,          // Type of value to search for (e.g. byte)
                   CMP_EQ,        // Comparison type (equal)
                   &value,        // Pointer to the value to search for
                   &results,      // Output: array of results
                   &count         // Output: number of matches found
    );

    log_printf(LOG_GREEN, "Found %zu matches for value %lu (0x%lx).\n", count,
               value, value);

    // TODO: store results in g_app_state.last_results for narrowing
    free(results);
}
