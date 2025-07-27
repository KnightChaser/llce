// src/datastructure/hashmap.c
#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

// A single entry in a hash map bucket
typedef struct hash_map_entry_t {
    uintptr_t key;
    void *value;
    struct hash_map_entry_t *next;
} hash_map_entry_t;

// The main hash map structure
// NOTE: "strct hash_map_t" is redefined as "hash_map_t" in the header file
struct hash_map_t {
    size_t capacity;
    hash_map_entry_t **buckets;
};

/**
 * Calculate the index(hash value) for a given key.
 * This uses a simple hash function based on the key's value.
 *
 * @param key The key to hash.
 * @return The index in the hash map's bucket array.
 */
static size_t hash_key(uintptr_t key) {
    size_t hash = 0xcbf29ce484222325;
    for (size_t i = 0; i < sizeof(key); ++i) {
        hash ^= (key >> (i * 8)) & 0xFF;
        hash *= 0x100000001b3;
    }
    return hash;
}

/**
 * Create a new hash map with the specified capacity.
 * The capacity is the number of buckets in the hash map.
 *
 * @param capacity The number of buckets in the hash map.
 * @return A pointer to the newly created hash map, or NULL on failure.
 */
hash_map_t *hash_map_create(size_t capacity) {
    if (capacity == 0) {
        return NULL;
    }

    hash_map_t *map = calloc(1, sizeof(hash_map_t));
    if (!map) {
        return NULL;
    }

    map->capacity = capacity;
    map->buckets = calloc(capacity, sizeof(hash_map_entry_t *));
    if (!map->buckets) {
        free(map);
        return NULL;
    }

    return map;
}

/**
 * Insert a key-value pair into the hash map.
 * If the key already exists, the value is updated.
 *
 * @param map The hash map to insert into.
 * @param key The key to insert.
 * @param value The value to associate with the key.
 */
void hash_map_destroy(hash_map_t *map) {
    if (!map) {
        return;
    }

    for (size_t i = 0; i < map->capacity; i++) {
        hash_map_entry_t *entry = map->buckets[i];
        while (entry) {
            hash_map_entry_t *next = entry->next;
            free(entry);
            entry = next;
        }
    }

    free(map->buckets);
    free(map);
}

/**
 * Insert a key-value pair into the hash map.
 * If the key already exists, the value is updated.
 *
 * @param map The hash map to insert into.
 * @param key The key to insert.
 * @param value The value to associate with the key.
 */
void hash_map_put(hash_map_t *map, uintptr_t key, void *value) {
    if (!map || !value) {
        return;
    }
    size_t index = hash_key(key) % map->capacity;
    hash_map_entry_t *entry = map->buckets[index];

    // Check if the key already exists in the bucket
    while (entry) {
        if (entry->key == key) {
            entry->value = value;
            return;
        }
        entry = entry->next;
    }

    // If key is not found, create a new entry
    hash_map_entry_t *new_entry = calloc(1, sizeof(hash_map_entry_t));
    if (!new_entry) {
        return;
    }

    new_entry->key = key;
    new_entry->value = value;
    new_entry->next = map->buckets[index];
    map->buckets[index] = new_entry;
}

/**
 * Retrieve a value from the hash map by its key.
 * If the key does not exist, NULL is returned.
 *
 * @param map The hash map to retrieve from.
 * @param key The key to look up.
 * @return The value associated with the key, or NULL if not found.
 */
void *hash_map_get(hash_map_t *map, uintptr_t key) {
    if (!map) {
        return NULL;
    }
    size_t index = hash_key(key) % map->capacity;
    hash_map_entry_t *entry = map->buckets[index];

    while (entry) {
        if (entry->key == key) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}
