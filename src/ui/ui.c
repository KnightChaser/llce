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

        if (!command) {
            continue;
        }

        if (strcmp(command, "help") == 0) {
            // Show help message
            handle_help();
        } else if (strcmp(command, "attach") == 0) {
            // Attach to a process and start session
            handle_attach(arg1);
        } else if (strcmp(command, "fullscan") == 0) {
            // Perform a full scan of the process memory
            handle_fullscan();
        } else if (strcmp(command, "detect") == 0) {
            // Detect the changs of process and its memory layout
            bool paginate = false;
            if (arg1 && strcmp(arg1, "page") == 0) {
                paginate = true;
            } else if (arg1) {
                log_printf(LOG_RED,
                           "Invalid argument for 'detect'. Use 'detect page' "
                           "for page scan.\n");
            }
            handle_detect(paginate);
        } else if (strcmp(command, "search") == 0) {
            // Search for a value in the process memory
            handle_search(arg1, arg2);
        } else if (strcmp(command, "poke") == 0) {
            // Poke (memory write) a value in the process memory
            handle_poke(arg1, arg2, arg3);
        } else if (strcmp(command, "exit") == 0) {
            // Exit the application
            break;
        } else {
            // Wrong command!! >_<
            log_printf(LOG_RED, "Unknown command: %s\n", command);
            handle_help();
        }
    }

    cleanup_app_state();
    log_printf(LOG_GREEN, "Exiting llce. Goodbye!\n");
}
