//
// Created by serbis on 01.07.18.
//

#ifndef ACTORS_LBQ8_H
#define ACTORS_LBQ8_H

#include <stdint.h>
#include <malloc.h>
#include "colnode.h"
#include "../../oscl/include/threads.h"

typedef struct LinkedBlockingQueue8 {
    uint16_t capacity;
    uint16_t count;
    Node8 *head;
    Node8 *last;
    mutex_t *mutex;

    void (*enqueue)(void*, uint8_t);
    uint8_t (*dequeue)(void*);
    uint16_t (*size)(void*);
} LinkedBlockingQueue8;

void del_LQB8(LinkedBlockingQueue8 *queue);
LinkedBlockingQueue8* new_LQB8(uint16_t capacity);

#endif //ACTORS_LBQ9_H
