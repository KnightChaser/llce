// src/ui/handler/detect.c
#include "../../utils/scan.h"
#include "../app_state.h"
#include "../logger.h"
#include "handler.h"
#include <signal.h>
#include <stdio.h>

/**
 * Handle the 'detect' command.
 * This command compares two memory scans and detects changes between them.
 * It requires two scans to be performed first (attach and fullscan).
 *
 * @param page If true, the output will be paginated.
 */
void handle_detect(bool paginate) {
    if (!g_app_state.previous_scan || !g_app_state.current_scan) {
        log_printf(
            LOG_RED,
            "Error: Two scans are required. Use 'attach' then 'fullscan'.\n");
        return;
    }

    // 1) Detect alll changes into a flat buffer
    mem_change_t *changes = NULL;
    size_t count = 0;
    detect_memory_changes(g_app_state.previous_scan,
                          g_app_state.previous_scan_count,
                          g_app_state.current_scan,
                          g_app_state.current_scan_count, &changes, &count);

    if (!paginate) {
        // Truncate to first 20
        size_t shown = count < 20 ? count : 20;
        for (size_t i = 0; i < shown; i++) {
            printf("  -> Change at 0x%lx: 0x%02x → 0x%02x\n", changes[i].addr,
                   changes[i].old_value, changes[i].new_value);
        }

        if (count > shown) {
            log_printf(
                LOG_YELLOW,
                "%zu out of %zu results shown. Use 'detect page' to scroll.\n",
                shown, count);
        } else {
            log_printf(LOG_GREEN, "All %zu changes shown.\n", count);
        }
    } else {
        // Ignore SIGPIPE so writes to a closed pipe don't kill the original
        // program (llce)
        struct sigaction sa_ignore, sa_old;
        sa_ignore.sa_handler = SIG_IGN;
        sigemptyset(&sa_ignore.sa_mask);
        sa_ignore.sa_flags = 0;
        sigaction(SIGPIPE, &sa_ignore, &sa_old);

        // Pipe *all* lines (including color codes) into "less
        // -R" commands
        FILE *pager = popen("less -R", "w");
        if (!pager) {
            perror("Failed to launch pager (less -R)");
            // Fallback: dump everything to stdout
            for (size_t i = 0; i < count; i++) {
                fprintf(stdout, "  -> Change at 0x%lx: 0x%02x → 0x%02x\n",
                        changes[i].addr, changes[i].old_value,
                        changes[i].new_value);
            }
        } else {
            // Write all changes to the pager
            for (size_t i = 0; i < count; i++) {
                fprintf(pager, "  -> Change at 0x%lx: 0x%02x → 0x%02x\n",
                        changes[i].addr, changes[i].old_value,
                        changes[i].new_value);
            }
            pclose(pager);
        }

        // Restore previous SIGPIPE handler
        sigaction(SIGPIPE, &sa_old, NULL);
    }

    free_mem_changes(changes);
}
