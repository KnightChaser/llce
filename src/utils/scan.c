// src/utils/scan.c
#include "scan.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Append a new scan result to the results array.
 * If the array is full, it will be resized to accommodate more results.
 *
 * @param arr Pointer to the results array.
 * @param n Current number of results in the array.
 * @param capacity Current capacity of the results array.
 * @param addr Address of the found match.
 * @param len Length of the found match.
 */
static void append_result(scan_result_t **arr, size_t *n, size_t *capacity,
                          uintptr_t addr, size_t len) {
    // Double the capacity if needed
    if (*n == *capacity) {
        *capacity = *capacity ? *capacity * 2 : 64;
        *arr = realloc(*arr, (*capacity) * sizeof(scan_result_t));
        if (!*arr) {
            perror("Failed to allocate memory for scan results");
            exit(EXIT_FAILURE);
        }

        // Append the new result to the array
        if (*arr) {
            (*arr)[*n] = (scan_result_t){
                .addr = addr,
                .len = len,
            };
            (*n)++;
        }
    }
}

/**
 * Append a new address to the addresses array.
 * If the array is full, it will be resized to accommodate more addresses.
 *
 * @param arr Pointer to the addresses array.
 * @param n Current number of addresses in the array.
 * @param capacity Current capacity of the addresses array.
 * @param addr Address to append.
 */
static void append_addr(uintptr_t **arr, size_t *n, size_t *capacity,
                        uintptr_t addr) {
    if (*n == *capacity) {
        *capacity = *capacity ? *capacity * 2 : 64;
        *arr = realloc(*arr, (*capacity) * sizeof(uintptr_t));
        if (!*arr) {
            perror("Failed to allocate memory for addresses");
            exit(EXIT_FAILURE);
        }
    }
    (*arr)[(*n)++] = addr;
}

/**
 * Search for specific byte-pattern (not always with a numerical value) in
 * memory regions based on a comparison operation.
 *
 * @param regions Array of memory regions to search.
 * @param rcount Number of memory regions.
 * @param pattern Pointer to the value to compare against.
 * @param pattern_len Size of the value type (e.g., sizeof(int)).
 * @param out Pointer to the output array of scan results.
 * @param out_count Pointer to the number of results found.
 */
int search_exact(mem_region_t *regions, // [in]
                 size_t rcount,         // [in]
                 const void *pattern,   // [in]
                 size_t pattern_len,    // [in]
                 scan_result_t **out,   // [out]
                 size_t *out_count      // [out]
) {
    *out = NULL;
    *out_count = 0;
    size_t capacity = 0;

    for (size_t i = 0; i < rcount; i++) {
        uint8_t *data = regions[i].data;
        size_t len = regions[i].len;
        for (size_t offset = 0; offset + pattern_len <= len; offset++) {
            if (memcmp(data + offset, pattern, pattern_len) == 0) {
                append_result(out, out_count, &capacity,
                              regions[i].start + offset, pattern_len);
            }
        }
    }
    return 0;
}

/**
 * Search for numeric values in memory regions based on a comparison
 * operation.
 *
 * @param regions Array of memory regions to search.
 * @param rcount Number of memory regions.
 * @param cmp Comparison operation (CMP_EQ, CMP_NE, CMP_GT, CMP_LT).
 * @param value Pointer to the value to compare against.
 * @param value_len Size of the value type (e.g., sizeof(int)).
 * @param out Pointer to the output array of scan results.
 * @param out_count Pointer to the number of results found.
 */
int search_compare(mem_region_t *regions, // [in]
                   size_t rcount,         // [in]
                   cmp_op_t cmp,          // [in]
                   const void *value,     // [in]
                   size_t value_len,      // [in]
                   scan_result_t **out,   // [out]
                   size_t *out_count      // [out]
) {
    *out = NULL;
    *out_count = 0;
    size_t capacity = 0;

    for (size_t i = 0; i < rcount; i++) {
        uint8_t *data = regions[i].data;
        size_t len = regions[i].len;
        for (size_t offset = 0; offset + value_len <= len; offset++) {
            // NOTE: load the value as an unsigned integer for simplicity
            uint64_t val = 0;
            uint64_t tgt = 0;
            memcpy(&val, data + offset, value_len);
            memcpy(&tgt, value, value_len);
            bool hit = false;

            switch (cmp) {
            case CMP_EQ:
                hit = (val == tgt);
                break;
            case CMP_NE:
                hit = (val != tgt);
                break;
            case CMP_GT:
                hit = (val > tgt);
                break;
            case CMP_LT:
                hit = (val < tgt);
                break;
            default:
                perror("Unknown comparison operation");
                return -1;
            }

            if (hit) {
                append_result(out, out_count, &capacity,
                              regions[i].start + offset, value_len);
            }
        }
    }
    return 0;
}

/**
 * Detect changes in memory regions by comparing two scans.
 *
 * @param old_scan Old memory scan results.
 * @param old_n Number of regions in the old scan.
 * @param new_scan New memory scan results.
 * @param new_n Number of regions in the new scan.
 * @param out_addrs Pointer to the output array of changed addresses.
 * @param out_count Pointer to the number of changed addresses found.
 */
int detect_memory_changes(mem_region_t *old_scan, // [in]
                          size_t old_n,           // [in]
                          mem_region_t *new_scan, // [in]
                          size_t new_n,           // [in]
                          uintptr_t **out_addrs,  // [out]
                          size_t *out_count       // [out]
) {
    *out_addrs = NULL;
    *out_count = 0;
    size_t capacity = 0;

    for (size_t i = 0; i < old_n && i < new_n; i++) {
        // NOTE: assume the same ordering and size
        size_t len = old_scan[i].len < new_scan[i].len ? old_scan[i].len
                                                       : new_scan[i].len;
        for (size_t offset = 0; offset < len; offset++) {
            if (old_scan[i].data[offset] != new_scan[i].data[offset]) {
                // If the bytes differ, we consider it a change
                append_addr(out_addrs, out_count, &capacity,
                            old_scan[i].start + offset);
            }
        }
    }
    return 0;
}

/**
 * Free the memory allocated for scan results.
 *
 * @param res Pointer to the scan result array to free.
 */
void free_scan_results(scan_result_t *res) {
    if (res) {
        free(res);
    }
}
