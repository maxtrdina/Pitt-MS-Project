//
// Created by izzy on 11/11/20.
//

#ifndef COMMON_LIST_H
#define COMMON_LIST_H

#include <pthread.h>

typedef struct List_s {
    int capacity;
    int size;
    void **items

    pthread_rwlock_t lock;
} List;

/*List* list_create();
void list_add(List* list, void* item);
void list_remove(List* list, int index);
void* list_get(List* list, int index);
void list_destroy(List* list);*/

#endif // COMMON_LIST_H
