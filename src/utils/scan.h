// src/utils/scan.h
#pragma once
#include "probe.h" // mem_region_t
#include <stddef.h>
#include <stdint.h>

typedef enum {
    CMP_EQ, // Equal
    CMP_NE, // Not Equal
    CMP_GT, // Greater Than
    CMP_LT, // Less Than
} cmp_op_t;

typedef enum {
    SCAN_TYPE_BYTE,  // 1-byte integer (uint8_t)
    SCAN_TYPE_WORD,  // 2-byte integer (uint16_t)
    SCAN_TYPE_DWORD, // 4-byte integer (uint32_t)
    SCAN_TYPE_QWORD, // 8-byte integer (uint64_t)
    // NOTE: later, add things like SCAN_TYPE_FLOAT or SCAN_TYPE_DOUBLE :)
} scan_type_t;

// A single match result
typedef struct {
    uintptr_t addr;
    size_t len;
} scan_result_t;

// A single memory change event
typedef struct {
    uintptr_t addr;
    uint8_t old_value;
    uint8_t new_value;
} mem_change_t;

/**
 * Exact-byte search across all memory regions.
 *
 * pattern: pointer to value (any byte sequence)
 * pattern_len: length of that sequence
 */
int search_exact(mem_region_t *regions, size_t rcount, const void *pattern,
                 size_t pattern_len, scan_result_t **out, size_t *out_count);

/**
 * Numeric comparison search, optimized for different data types.
 *
 * type: The size of the data to compare (1, 2, 4, or 8 bytes).
 * value: Pointer to the value to compare against (must match the type).
 * cmp: one of cmp_op_t values (CMP_EQ, CMP_NE, CMP_GT, CMP_LT)
 */
int search_compare(mem_region_t *regions, size_t rcount, scan_type_t type,
                   cmp_op_t cmp, const void *value, scan_result_t **out,
                   size_t *out_count);

/**
 * Detect changes in memory regions by comparing two scans.
 */
int detect_memory_changes(mem_region_t *old_scan, size_t old_n,
                          mem_region_t *new_scan, size_t new_n,
                          mem_change_t **out_changes, size_t *out_count);

/**
 * Free the memory allocated for memory changes.
 */
void free_mem_changes(mem_change_t *changes);
