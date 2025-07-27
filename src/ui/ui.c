// src/ui/ui.c
#include "ui.h"
#include "../utils/pid.h"
#include "logger.h"
#include <stdio.h>
#include <string.h>

// Application state
typedef struct {
    pid_t pid;
    char proc_name[256];
    bool attached;
    // TODO: Later, store the scan list here later...
} app_state_t;

// Global instance of the application state
static app_state_t g_state;

// FOrward declarations for command helpers
static void handle_attach(char *arg);
static void handle_fullscan(void);
static void handle_help(void);

/**
 * Initialize the application state.
 * This function sets the initial values for the global state.
 */
static void print_prompt(void) {
    if (g_state.attached) {
        log_printf(LOG_BOLD_WHITE, "llce(%s:%d)> ", g_state.proc_name,
                   g_state.pid);
    } else {
        log_printf(LOG_BOLD_WHITE, "llce> ");
    }
}

/**
 * Run the overall user interface loop.
 * (REPL - Read-Eval-Print Loop)
 */
void run_ui(void) {
    char line[256];
    memset(&g_state, 0, sizeof(g_state)); // Initialize the state

    log_printf(LOG_GREEN,
               "Welcome to llce - the command-line cheat engine for Linux.\n");
    handle_help();

    while (true) {
        print_prompt();
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        line[strcspn(line, "\n")] = 0; // Remove trailing newline

        char *command = strtok(line, " ");
        char *arg = strtok(NULL, " ");
        if (!command) {
            // empty line
            continue;
        }

        if (strcmp(command, "attach") == 0) {
            handle_attach(arg);
        } else if (strcmp(command, "fullscan") == 0) {
            handle_fullscan();
        } else if (strcmp(command, "help") == 0) {
            handle_help();
        } else if (strcmp(command, "exit") == 0 ||
                   strcmp(command, "quit") == 0) {
            log_printf(LOG_GREEN, "Exiting llce. Goodbye!\n");
            break;
        } else {
            log_printf(LOG_RED,
                       "Unknown command: %s. Type 'help' for a list of help!\n",
                       command);
        }
    }

    log_printf(LOG_GREEN, "Exiting llce. Goodbye!\n");
}

/**
 * Display the help message with available commands.
 * Invoked when the user types "help".
 */
static void handle_help(void) {
    log_printf(LOG_YELLOW, "Available commands:\n");
    log_printf(LOG_DEFAULT, "  - ");
    log_printf(LOG_GREEN, "attach <pid>");
    log_printf(LOG_DEFAULT,
               " : Attach to a process and perform an initial memory scan.\n");
    log_printf(LOG_DEFAULT, "  - ");
    log_printf(LOG_GREEN, "fullscan");
    log_printf(LOG_DEFAULT,
               "     : Perform a new full scan of the attached process.\n");
    // Add more commands here as they are implemented
    log_printf(LOG_DEFAULT, "  - ");
    log_printf(LOG_GREEN, "help");
    log_printf(LOG_DEFAULT, "         : Show this help message.\n");
    log_printf(LOG_DEFAULT, "  - ");
    log_printf(LOG_GREEN, "exit");
    log_printf(LOG_DEFAULT, "         : Close the application.\n");
}

static void handle_attach(char *arg) {
    if (!arg) {
        log_printf(LOG_RED, "Usage: attach <pid>\n");
        return;
    }

    pid_t pid = pid_from_argv(arg);
    if (!pid_exists(pid)) {
        log_printf(LOG_RED, "No process with PID %d found.\n", pid);
        return;
    }

    // TODO: Actually run full_scan and store the results in g_state
    log_printf(
        LOG_GREEN,
        "Successfully attached to PID %d. Initial scan would run here.\n", pid);

    g_state.pid = pid;

    // For now, we'll just use the pid as the name. A more advanced version
    // could read /proc/<pid>/comm.
    snprintf(g_state.proc_name, sizeof(g_state.proc_name), "pid_%d", pid);
    g_state.attached = true;
}

static void handle_fullscan(void) {
    if (!g_state.attached) {
        log_printf(
            LOG_RED,
            "You must attach to a process first using 'attach <pid>'.\n");
        return;
    }

    // TODO: Run full_scan and update g_state
    log_printf(LOG_GREEN, "Performing full scan on PID %d...\n", g_state.pid);
}
