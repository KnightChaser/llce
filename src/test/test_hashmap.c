// src/test/test_hashmap.c
#include "../datastructure/hashmap.h"
#include <assert.h>
#include <stdio.h>

void test_create_destroy(void) {
    printf("Running test: %s\n", __func__);
    hash_map_t *map = hash_map_create(16);
    assert(map != NULL);
    hash_map_destroy(map);
    printf("OK\n");
}

void test_put_get(void) {
    printf("Running test: %s\n", __func__);
    hash_map_t *map = hash_map_create(16);
    int value1 = 100;
    int value2 = 200;

    // Test insertion
    hash_map_put(map, 0x1000, &value1);
    hash_map_put(map, 0x2000, &value2);

    // Test retrieval
    int *ret1 = (int *)hash_map_get(map, 0x1000);
    int *ret2 = (int *)hash_map_get(map, 0x2000);
    assert(ret1 != NULL && *ret1 == 100);
    assert(ret2 != NULL && *ret2 == 200);

    // Test getting a non-existent key
    assert(hash_map_get(map, 0x3000) == NULL);

    // Test updating an existing key
    int value3 = 300;
    hash_map_put(map, 0x1000, &value3);
    int *ret3 = (int *)hash_map_get(map, 0x1000);
    assert(ret3 != NULL && *ret3 == 300);

    hash_map_destroy(map);
    printf("OK\n");
}

int main(void) {
    test_create_destroy();
    test_put_get();
    return 0;
}
