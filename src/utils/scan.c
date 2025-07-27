// src/utils/scan.c
#include "scan.h"
#include "../datastructure/hashmap.h"
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
    }

    // Append the new result to the array
    (*arr)[*n] = (scan_result_t){
        .addr = addr,
        .len = len,
    };
    (*n)++;
}

/**
 * Append a new memory change to the changes array.
 * If the array is full, it will be resized to accommodate more changes.
 *
 * @param arr Pointer to the changes array.
 * @param n Current number of changes in the array.
 * @param capacity Current capacity of the changes array.
 * @param addr Address of the changed memory location.
 * @param old_value Old value at that address.
 * @param new_value New value at that address.
 */
static void append_change(mem_change_t **arr, // [out]
                          size_t *n,          // [out]
                          size_t *capacity,   // [out]
                          uintptr_t addr,     // [in]
                          uint8_t old_value,  // [in]
                          uint8_t new_value   // [in]
) {
    if (*n == *capacity) {
        *capacity = *capacity ? *capacity * 2 : 64;
        *arr = realloc(*arr, (*capacity) * sizeof(mem_change_t));
        if (!*arr) {
            perror("Failed to allocate memory for memory changes");
            exit(EXIT_FAILURE);
        }
    }

    (*arr)[*n] = (mem_change_t){
        .addr = addr,
        .old_value = old_value,
        .new_value = new_value,
    };
    (*n)++;
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
 * @param type Type of the data to compare (SCAN_TYPE_BYTE, SCAN_TYPE_WORD, ...)
 * @param cmp Comparison operation (CMP_EQ, CMP_NE, CMP_GT, CMP_LT).
 * @param value Pointer to the value to compare against.
 * @param out Pointer to the output array of scan results.
 * @param out_count Pointer to the number of results found.
 */
int search_compare(mem_region_t *regions, // [in]
                   size_t rcount,         // [in]
                   scan_type_t type,      // [in]
                   cmp_op_t cmp,          // [in]
                   const void *value,     // [in]
                   scan_result_t **out,   // [out]
                   size_t *out_count      // [out]
) {
    *out = NULL;
    *out_count = 0;
    size_t capacity = 0;

    size_t type_size;
    switch (type) {
    case SCAN_TYPE_BYTE:
        type_size = sizeof(uint8_t); // 1 byte
        break;
    case SCAN_TYPE_WORD:
        type_size = sizeof(uint16_t); // 2 bytes
        break;
    case SCAN_TYPE_DWORD:
        type_size = sizeof(uint32_t); // 4 bytes
        break;
    case SCAN_TYPE_QWORD:
        type_size = sizeof(uint64_t); // 8 bytes
        break;
    default:
        fprintf(stderr, "ERR: Invalid scan type %d\n", type);
        return -1; // Invalid type
    }

    for (size_t i = 0; i < rcount; i++) {
        uint8_t *data = regions[i].data;
        if (!data) {
            continue;
        }
        size_t len = regions[i].len;

        // Loop through the memory, taking steps equal to the type size
        for (size_t offset = 0; offset + type_size <= len;
             offset += type_size) {
            uint64_t val = 0;
            uint64_t tgt = 0;
            bool hit = false;

            // Cast the pointers directly and dereference them,
            // avoid slow memcmp() calls
            switch (type) {
            case SCAN_TYPE_BYTE:
                val = *(uint8_t *)(data + offset);
                tgt = *(uint8_t *)value;
                break;
            case SCAN_TYPE_WORD:
                val = *(uint16_t *)(data + offset);
                tgt = *(uint16_t *)value;
                break;
            case SCAN_TYPE_DWORD:
                val = *(uint32_t *)(data + offset);
                tgt = *(uint32_t *)value;
                break;
            case SCAN_TYPE_QWORD:
                val = *(uint64_t *)(data + offset);
                tgt = *(uint64_t *)value;
                break;
            }

            // Perform the actual comparison based on the cmp operation
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
                fprintf(stderr, "ERR: Invalid comparison operation %d\n", cmp);
                return -1; // Invalid comparison operation
            }

            if (hit) {
                append_result(out, out_count, &capacity,
                              regions[i].start + offset, type_size);
            }
        }
    }

    return 0;
}

/**
 * Detect changes in memory regions by comparing two scans.
 * If a region exists in both scans, it checks for byte-by-byte changes.
 * If a region exists only in the new scan, it is considered a change from
 * "nothing" to "something".
 *
 * @param old_scan Array of memory regions from the old scan.
 * @param old_n Number of regions in the old scan.
 * @param new_scan Array of memory regions from the new scan.
 * @param new_n Number of regions in the new scan.
 * @param out_changes Pointer to the output array of memory changes.
 * @param out_count Pointer to the number of changes found.
 */
int detect_memory_changes(mem_region_t *old_scan,     // [in]
                          size_t old_n,               // [in]
                          mem_region_t *new_scan,     // [in]
                          size_t new_n,               // [in]
                          mem_change_t **out_changes, // [out]
                          size_t *out_count           // [out]
) {
    *out_changes = NULL;
    *out_count = 0;
    size_t capacity = 0;

    // Create a hash map from the old scan for quick lookups
    hash_map_t *old_map = hash_map_create(old_n > 0 ? old_n * 2 - 1 : 16);
    if (!old_map) {
        perror("Failed to create hash map");
        return -1;
    }

    for (size_t i = 0; i < old_n; i++) {
        // Only map regions that have valid data buffers
        if (old_scan[i].data) {
            hash_map_put(old_map, old_scan[i].start, &old_scan[i]);
        }
    }

    // Iterate through the new scan and compare against the old one via the
    // hashmap that was just created
    for (size_t i = 0; i < new_n; i++) {
        if (!new_scan[i].data) {
            // Skip regions without data
            continue;
        }

        mem_region_t *old_region =
            (mem_region_t *)hash_map_get(old_map, new_scan[i].start);

        if (old_region) {
            // Region exists in both scans, compare byte-by-byte
            size_t len = old_region->len < new_scan[i].len ? old_region->len
                                                           : new_scan[i].len;
            for (size_t offset = 0; offset < len; offset++) {
                if (old_region->data[offset] != new_scan[i].data[offset]) {
                    append_change(out_changes, out_count, &capacity,
                                  old_region->start + offset,
                                  old_region->data[offset],
                                  new_scan[i].data[offset]);
                }
            }
        } else {
            // Region is newly allocated (exits in new_scan but not in old_scan)
            // This is considered a change from "nothing" to "something"
            for (size_t offset = 0; offset < new_scan[i].len; offset++) {
                append_change(out_changes, out_count, &capacity,
                              new_scan[i].start + offset,
                              0, // Old value is considered 0 (NULL)
                              new_scan[i].data[offset]);
            }
        }
    }

    hash_map_destroy(old_map);

    return 0;
}

/**
 * Free the memory allocated for scan results.
 *
 * @param changes Pointer to the array of memory changes.
 */
void free_mem_changes(mem_change_t *changes) {
    if (changes) {
        free(changes);
    }
}
