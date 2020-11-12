//
// Created by izzy on 11/2/20.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "map.h"

void map_rehash(Map* map);
int map_index(Map* map, int key);

Map* map_create() {
    Map* map = (Map*)malloc(sizeof(Map));
    map->_entryCount = 0;
    // Don't care to mess with initial capacities
    map->_capacity = 4;
    map->keys = (int*)malloc(map->_capacity*sizeof(int));
    for (int i = 0; i < map->_capacity; i++) {
        map->keys[i] = -1;
    }
    map->values = (void**)malloc(map->_capacity*sizeof(void*));

    // Read write lock for the map
    pthread_rwlock_init(&map->lock, NULL);

    return map;
}

/**
 * Inserts a value at [key]. Thread safe.
 *
 * @param map the map to insert a value on.
 * @param key the key.
 * @param value the value.
 */
void map_insert(Map* map, int key, void* value) {
    pthread_rwlock_wrlock(&map->lock);

    if (map->_entryCount == map->_capacity) {
        map_rehash(map);
    }

    int insert_index = key%map->_capacity;
    while (map->keys[insert_index] != -1 && map->keys[insert_index] != key) {
        insert_index = (insert_index+1)%map->_capacity;
    }
    map->keys[insert_index] = key;
    map->values[insert_index] = value;
    map->_entryCount++;

    pthread_rwlock_unlock(&map->lock);
}

/*
 * Doubles the capacity of the map and rehashes.
 *
 * NOTE: this function is internal.
 */
void map_rehash(Map* map) {
    int oldCapacity = map->_capacity;
    int* oldKeys = map->keys;
    void** oldValues = map->values;
    map->_capacity *= 2;
    map->_entryCount = 0;
    map->keys = (int*)malloc(map->_capacity*sizeof(int));
    for (int i = 0; i < map->_capacity; i++) {
        map->keys[i] = -1;
    }
    map->values = (void*)malloc(map->_capacity*sizeof(void*));

    for (int i = 0; i < oldCapacity; i++) {
        if (oldKeys[i] != -1) {
            map_insert(map, oldKeys[i], oldValues[i]);
        }
    }

    free(oldKeys);
    free(oldValues);
}

/**
 * Gets a value given a key. Thread safe.
 *
 * @param map the map to retrieve the value from
 * @param key the key.
 * @return the value, NULL if not found.
 */
void* map_get(Map* map, int key) {
    pthread_rwlock_rdlock(&map->lock);

    int index = map_index(map, key);
    if (index == -1) {
        return NULL;
    }
    void* value = map->values[index];

    pthread_rwlock_unlock(&map->lock);
    return value;
}

/**
 * Removes a value from a map. Thread safe.
 *
 * @param map the map to remove a value from.
 * @param key the key whose value needs to be removed.
 */
void map_delete(Map* map, int key) {
    pthread_rwlock_wrlock(&map->lock);

    int index = map_index(map, key);
    if (index != -1) {
        map->keys[map_index(map, key)] = -1;
        map->_entryCount--;
    }

    pthread_rwlock_unlock(&map->lock);
}

/*
 * Gets the index of an item. Internal.
 */
int map_index(Map* map, int key) {
    int index = key%map->_capacity;
    int original_index = index;
    int looped = 0;
    while(map->keys[index] != -1 && map->keys[index] != key) {
        index = (index+1)%map->_capacity;
        looped = 1;
    }

    if (index == original_index && looped) {
        return -1;
    }
    return index;
}

/**
 * Prints the set of keys and their indices. Thread safe.
 *
 * @param map the map to get the info from.
 */
void map_print(Map* map) {
    pthread_rwlock_rdlock(&map->lock);

    printf("Map (%d/%d):\n", map->_entryCount, map->_capacity);
    for (int i = 0; i < map->_capacity; i++) {
        if (map->keys[i] != -1) {
            printf("\tKey %d at index %d\n", map->keys[i], i);
        }
    }

    pthread_rwlock_unlock(&map->lock);
}

/**
 * Cleans up a map.
 *
 * @param map the map to destroy.
 */
void map_destroy(Map* map) {
    free(map->keys);
    free(map->values);
    pthread_rwlock_destroy(&map->lock);
    free(map);
}
