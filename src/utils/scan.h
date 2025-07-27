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

// A single match result
typedef struct {
    uintptr_t addr;
    size_t len;
} scan_result_t;

/**
 * Exact-byte search across all memory regions.
 *
 * pattern: pointer to value (any byte sequence)
 * pattern_len: length of that sequence
 */
int search_exact(mem_region_t *regions, size_t rcount, const void *pattern,
                 size_t pattern_len, scan_result_t **out, size_t *out_count);

/**
 * Numeric comparison search
 *
 * value: pointer to data types, such as int, float, double, etc.
 * value_len: size of the data type (e.g., sizeof(int), sizeof(float))
 * cmp: one of cmp_op_t values (CMP_EQ, CMP_NE, CMP_GT, CMP_LT)
 */
int search_compare(mem_region_t *regions, size_t rcount, cmp_op_t cmp,
                   const void *value, size_t value_len, scan_result_t **out,
                   size_t *out_count);

/**
 * Detect changes in memory regions by comparing two scans.
 */
int detect_memory_changes(mem_region_t *old_scan, size_t old_n,
                          mem_region_t *new_scan, size_t new_n,
                          uintptr_t **out_addrs, size_t *out_count);

/**
 * Free the memory allocated for scan results.
 */
void free_scan_results(scan_result_t *res);
