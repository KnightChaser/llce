// src/ui/ui.c
#include "ui.h"
#include "app_state.h"
#include "handler/handler.h"
#include "logger.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

app_state_t g_app_state;

/**
 * Cleanup the application state and free allocated memory.
 * This function is called when the user exits the application.
 */

/**
 * Print the help message with available commands.
 * This function is called when the user types 'help' or when the application
 * starts.
 */
static void print_prompt(void) {
    if (g_app_state.attached) {
        log_printf(LOG_BOLD_WHITE, "llce(%s:%d)> ", g_app_state.proc_name,
                   g_app_state.pid);
    } else {
        log_printf(LOG_BOLD_WHITE, "llce> ");
    }
}

/**
 * Handle the overall UI loop for the command-line interface.
 */
void run_ui(void) {
    char line[256];
    memset(&g_app_state, 0, sizeof(g_app_state));

    log_printf(LOG_GREEN,
               "Welcome to llce - the command-line cheat engine for Linux.\n");
    handle_help();

    while (true) {
        print_prompt();
        if (!fgets(line, sizeof(line), stdin))
            break;
        line[strcspn(line, "\n")] = 0;

        char *command = strtok(line, " ");
        char *arg1 = strtok(NULL, " ");
        char *arg2 = strtok(NULL, " ");
        char *arg3 = strtok(NULL, " ");

        if (!command)
            continue;
        if (strcmp(command, "help") == 0)
            handle_help();
        else if (strcmp(command, "attach") == 0)
            handle_attach(arg1);
        else if (strcmp(command, "fullscan") == 0)
            handle_fullscan();
        else if (strcmp(command, "detect") == 0)
            handle_detect();
        else if (strcmp(command, "search") == 0)
            handle_search(arg1, arg2);
        else if (strcmp(command, "poke") == 0)
            handle_poke(arg1, arg2, arg3);
        else if (strcmp(command, "exit") == 0)
            break;
        else {
            log_printf(LOG_RED, "Unknown command: %s\n", command);
            handle_help();
        }
    }

    cleanup_app_state();
    log_printf(LOG_GREEN, "Exiting llce. Goodbye!\n");
}
