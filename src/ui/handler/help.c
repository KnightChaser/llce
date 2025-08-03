// src/ui/handler/help.c
#include "../logger.h"
#include "handler.h"

/**
 * Display the help message with available commands.
 * This function lists all commands that the user can use in the application,
 * along with a brief description of each command.
 */
void handle_help(void) {
    log_printf(LOG_YELLOW, "Available commands:\n");
    log_printf(LOG_GREEN, "  attach <pid>              ");
    log_printf(LOG_DEFAULT, ": Attach to a process and run initial scan.\n");
    log_printf(LOG_GREEN, "  fullscan                  ");
    log_printf(LOG_DEFAULT, ": Perform a second scan to compare against.\n");
    log_printf(LOG_GREEN, "  detect                    ");
    log_printf(LOG_DEFAULT,
               ": Show changes between the first and second scan.\n");
    log_printf(LOG_GREEN, "  poke <addr> <type> <value> ");
    log_printf(LOG_DEFAULT, ": Write a value into target memory. Types: byte, "
                            "word, dword, qword\n");
    log_printf(LOG_GREEN, "  search <type> <value>     ");
    log_printf(LOG_DEFAULT, ": Search for a value in the first scan.\n");
    log_printf(LOG_DEFAULT, "                            ");
    log_printf(LOG_YELLOW, "  Types: byte, word, dword, qword\n");
    log_printf(LOG_GREEN, "  help                      ");
    log_printf(LOG_DEFAULT, ": Show this help message.\n");
    log_printf(LOG_GREEN, "  exit                      ");
    log_printf(LOG_DEFAULT, ": Close the application.\n");
}
