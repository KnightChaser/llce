// src/ui/ui.c
#include "ui.h"
#include "../utils/pid.h"
#include "../utils/poke.h"
#include "../utils/probe.h"
#include "../utils/scan.h"
#include "app_state.h"
#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global instance of the application state
static app_state_t g_app_state;

// FOrward declarations for command helpers
// TODO: Split such functions into a separate file
// (the function gonna be huge later)
static void handle_attach(char *arg);
static void handle_fullscan(void);
static void handle_detect(void);
static void handle_search(char *type_str, char *value_str);
static void handle_poke(char *addr_str, char *type_str, char *value_str);
static void handle_help(void);
static void cleanup_app_state(void);

/**
 * Initialize the application state.
 * This function sets the initial values for the global state.
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
 * Run the overall user interface loop.
 * (REPL - Read-Eval-Print Loop)
 */
void run_ui(void) {
    char line[256];
    memset(&g_app_state, 0, sizeof(g_app_state)); // Initialize the state

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
        if (!command) {
            // skip empty line
            continue;
        }

        char *arg1 = strtok(NULL, " ");
        char *arg2 = strtok(NULL, " ");
        char *arg3 = strtok(NULL, " ");

        if (strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "help") == 0) {
            handle_help();
        } else if (strcmp(command, "attach") == 0) {
            handle_attach(arg1);
        } else if (strcmp(command, "fullscan") == 0) {
            handle_fullscan();
        } else if (strcmp(command, "detect") == 0) {
            handle_detect();
        } else if (strcmp(command, "search") == 0) {
            handle_search(arg1, arg2);
        } else if (strcmp(command, "poke") == 0) {
            handle_poke(arg1, arg2, arg3);
        } else {
            log_printf(LOG_RED, "Unknown command: %s\n", command);
            handle_help();
        }
    }

    cleanup_app_state();
    log_printf(LOG_GREEN, "Exiting llce. Goodbye!\n");
}

/**
 * Display the help message with available commands.
 * Invoked when the user types "help".
 */
static void handle_help(void) {
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

/**
 * Cleanup the application state.
 * This function frees any allocated memory and resets the state.
 *
 * @param arg The first argument contains the PID to attach to.
 */
static void handle_attach(char *arg) {
    if (!arg) {
        log_printf(LOG_RED, "Usage: attach <pid>\n");
        return;
    }

    cleanup_app_state();
    pid_t pid = pid_from_argv(arg);
    if (!pid_exists(pid)) {
        log_printf(LOG_RED,
                   "Process with PID %d does not exist nor accessible.\n", pid);
        g_app_state.attached = false;
        return;
    }

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

/**
 * Handle the full scan of a targeted process.
 */
static void handle_fullscan(void) {
    if (!g_app_state.attached) {
        log_printf(LOG_RED,
                   "You must attach to a process first using 'attach'.\n");
        return;
    }

    // Free the previous second scan if it exists
    // TODO: later, we have to store three different scans:
    // - Intial scan (scan_initial)
    // - Latest-1 scan (scan_a)
    // - Latest scan (scan_b)
    if (g_app_state.scan_b) {
        free_mem_regions(g_app_state.scan_b, g_app_state.scan_b_count);
        g_app_state.scan_b = NULL;
    }

    log_printf(LOG_DEFAULT, "Performing second scan on %s...\n",
               g_app_state.proc_name);
    if (full_scan(g_app_state.pid, &g_app_state.scan_b,
                  &g_app_state.scan_b_count) != 0) {
        log_printf(LOG_RED, "Failed to perform second scan.\n");
        return;
    }
    log_printf(LOG_GREEN, "Second scan complete. Found %zu regions.\n",
               g_app_state.scan_b_count);
    log_printf(LOG_YELLOW, "You can now run 'detect' to see changes.\n");
}

/**
 * Handle the memory diff detection between two scans.
 */
static void handle_detect(void) {
    if (!g_app_state.scan_a || !g_app_state.scan_b) {
        log_printf(
            LOG_RED,
            "Error: Two scans are required. Use 'attach' then 'fullscan'.\n");
        return;
    }
    log_printf(LOG_DEFAULT, "Detecting changes...\n");
    mem_change_t *changes = NULL;
    size_t count = 0;
    detect_memory_changes(g_app_state.scan_a, g_app_state.scan_a_count,
                          g_app_state.scan_b, g_app_state.scan_b_count,
                          &changes, &count);
    log_printf(LOG_GREEN, "Detected %zu changes.\n", count);
    for (size_t i = 0; i < count && i < 20; i++) { // Limit output
        printf("  -> Change at 0x%lx, value: 0x%02x -> 0x%02x\n",
               changes[i].addr, changes[i].old_value, changes[i].new_value);
    }
    if (count > 20)
        log_printf(LOG_YELLOW, "  ... (output truncated)\n");
    free_mem_changes(changes);
}

/**
 * Handle searching for a specific value in the first scan.
 * This function supports searching for byte, word, dword, and qword values.
 *
 * @param type_str The type of value to search for (byte, word, dword, qword).
 * @param value_str The value to search for as a string.
 */
static void handle_search(char *type_str, char *value_str) {
    if (!g_app_state.scan_a) {
        log_printf(LOG_RED, "Error: No scan data available. Use 'attach'.\n");
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
    search_compare(g_app_state.scan_a, g_app_state.scan_a_count, type, CMP_EQ,
                   &value, &results, &count);

    log_printf(LOG_GREEN, "Found %zu matches for value %lu (0x%lx).\n", count,
               value, value);
    // TODO: We need a way to *store* these results for the next scan.
    // For now, we just print them.
    free(results); // search_compare doesn't have a dedicated free function yet
}

static void handle_poke(char *addr_str, char *type_str, char *value_str) {
    if (!g_app_state.attached) {
        log_printf(LOG_RED, "Error: attach to a process first.\n");
        return;
    }
    if (!addr_str || !type_str || !value_str) {
        log_printf(LOG_RED, "Usage: poke <addr> <type> <value>\n");
        return;
    }

    uintptr_t addr = strtoull(addr_str, NULL, 0);
    uint64_t val = strtoull(value_str, NULL, 0);
    int rc;

    if (strcmp(type_str, "byte") == 0) {
        uint8_t b = (uint8_t)val;
        rc = poke_mem(g_app_state.pid, addr, &b, sizeof(b));
        if (rc == 0) {
            log_printf(LOG_GREEN, "Wrote byte 0x%02x -> 0x%lx\n", b, addr);
        } else {
            log_printf(LOG_RED, "poke failed: %s\n", strerror(rc));
        }
    } else if (strcmp(type_str, "word") == 0) {
        uint16_t w = (uint16_t)val;
        rc = poke_mem(g_app_state.pid, addr, &w, sizeof(w));
        if (rc == 0) {
            log_printf(LOG_GREEN, "Wrote word 0x%04x -> 0x%lx\n", w, addr);
        } else {
            log_printf(LOG_RED, "poke failed: %s\n", strerror(rc));
        }
    } else if (strcmp(type_str, "dword") == 0) {
        uint32_t d = (uint32_t)val;
        rc = poke_mem(g_app_state.pid, addr, &d, sizeof(d));
        if (rc == 0) {
            log_printf(LOG_GREEN, "Wrote dword 0x%08x -> 0x%lx\n", d, addr);
        } else {
            log_printf(LOG_RED, "poke failed: %s\n", strerror(rc));
        }
    } else if (strcmp(type_str, "qword") == 0) {
        uint64_t q = val;
        rc = poke_mem(g_app_state.pid, addr, &q, sizeof(q));
        if (rc == 0) {
            log_printf(LOG_GREEN, "Wrote qword 0x%016lx -> 0x%lx\n", q, addr);
        } else {
            log_printf(LOG_RED, "poke failed: %s\n", strerror(rc));
        }
    } else {
        log_printf(LOG_RED, "Unknown type: %s\n", type_str);
    }
}

/**
 * Cleanup the application state and free allocated memory.
 * This function is called when the user exits the application.
 */
static void cleanup_app_state(void) {
    if (g_app_state.scan_a) {
        free_mem_regions(g_app_state.scan_a, g_app_state.scan_a_count);
    }
    if (g_app_state.scan_b) {
        free_mem_regions(g_app_state.scan_b, g_app_state.scan_b_count);
    }
    memset(&g_app_state, 0, sizeof(g_app_state));
}
