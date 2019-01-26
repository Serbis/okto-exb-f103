//
// Created by serbis on 01.07.18.
//

#ifndef ACTORS_COLNODE_H
#define ACTORS_COLNODE_H

typedef struct Node {
    void *item;
    struct Node *next;
} Node;

typedef struct Node8 {
    uint8_t item;
    struct Node *next;
} Node8;


#endif //ACTORS_COLNODE_H
