// src/ui/handler/handler.h
#include "../app_state.h"
#include "../logger.h"
#include "handler.h"

/**
 * Print the help message with available commands.
 * This function is called when the user types 'help' or when the application
 * starts.
 */
void print_prompt(void) {
    if (g_app_state.attached) {
        log_printf(LOG_BOLD_WHITE, "llce(%s:%d)> ", g_app_state.proc_name,
                   g_app_state.pid);
    } else {
        log_printf(LOG_BOLD_WHITE, "llce> ");
    }
}
