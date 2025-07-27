// src/datastructure/hashmap.h
#pragma once
#include <stddef.h>
#include <stdint.h>

/**
 * hash_map_t is a simple hash map implementation that maps uintptr_t keys.
 * It's designed to be used for storing and retrieving values (void pointers),
 * so it does not handle memory management of the values themselves.
 * But it can store any type of data as long as you manage the memory. :)
 */
typedef struct hash_map_t hash_map_t;

hash_map_t *hash_map_create(size_t capacity);
void hash_map_destroy(hash_map_t *map);
void hash_map_put(hash_map_t *map, uintptr_t key, void *value);
void *hash_map_get(hash_map_t *map, uintptr_t key);
