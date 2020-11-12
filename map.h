//
// Created by izzy on 11/2/20.
//

#ifndef COMMON_MAP_H
#define COMMON_MAP_H

#include <pthread.h>

typedef struct Map_s {
    int _capacity;
    int _entryCount;
    int* keys;
    void** values;

    pthread_rwlock_t lock;
} Map;

Map* map_create();
void map_insert(Map* map, int key, void* value);
void* map_get(Map* map, int key);
void map_delete(Map* map, int key);
void map_print(Map* map);
void map_destroy(Map* map);

#endif // COMMON_MAP_H
