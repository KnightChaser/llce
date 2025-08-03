// src/ui/handler/attach.h
#include "../../utils/pid.h"
#include "../../utils/probe.h"
#include "../app_state.h"
#include "../logger.h"
#include "handler.h"

/**
 * Handle the 'attach' command.
 * This command allows the user to attach to a running process by its PID.
 * It performs an initial scan of the process's memory to identify readable
 * and writable regions.
 *
 * @param arg The argument passed to the command, expected to be a PID.
 *           If no argument is provided, an error message is displayed.
 */
void handle_attach(char *arg) {
    if (!arg) {
        log_printf(LOG_RED, "Usage: attach <pid>\n");
        return;
    }

    // NOTE: Clean up any previous state since we are attaching to a new process
    extern app_state_t g_app_state;
    cleanup_app_state();

    pid_t pid = pid_from_argv(arg);
    if (!pid_exists(pid)) {
        log_printf(LOG_RED, "Process with PID %d does not exist.\n", pid);
        return;
    }

    // Attach to the process and conduct the initial scan.
    g_app_state.pid = pid;
    get_proc_name(pid, g_app_state.proc_name, sizeof(g_app_state.proc_name));
    g_app_state.attached = true;

    log_printf(LOG_DEFAULT,
               "Attaching to %s (PID: %d). Performing initial scan...\n",
               g_app_state.proc_name, g_app_state.pid);
    if (full_scan(pid, &g_app_state.scan_a, &g_app_state.scan_a_count) != 0) {
        log_printf(LOG_RED, "Failed to perform initial scan for PID %d.\n",
                   pid);
        cleanup_app_state();
        return;
    }
    log_printf(LOG_GREEN,
               "Initial scan complete. Found %zu readable/writable regions.\n",
               g_app_state.scan_a_count);
    log_printf(
        LOG_YELLOW,
        "You can now run 'search' or perform a 'fullscan' for comparison.\n");
}
