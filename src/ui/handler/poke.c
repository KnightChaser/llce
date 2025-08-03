// src/ui/handler/poke.c
#include "../../utils/poke.h"
#include "../app_state.h"
#include "../logger.h"
#include "handler.h"
#include <stdlib.h>
#include <string.h>

/**
 * Handle the 'poke' command.
 * This command allows the user to write a value into a specific memory address
 * of the attached process.
 *
 * @param addr_str The address to write to, as a string.
 * @param type_str The type of value to write (byte, word, dword, qword).
 * @param value_str The value to write, as a string.
 */
void handle_poke(char *addr_str, char *type_str, char *value_str) {
    if (!g_app_state.attached) {
        log_printf(LOG_RED, "Error: attach to a process first.\n");
        return;
    }
    if (!addr_str || !type_str || !value_str) {
        log_printf(LOG_RED, "Usage: poke <addr> <type> <value>\n");
        return;
    }

    // Convert address and value strings to appropriate types
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
