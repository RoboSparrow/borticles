#ifndef __QNODE_H__
#define __QNODE_H__

#include <stdio.h>
#include "vec.h"

// A quadtree with a  capacity of 4 itmes per quad
//    - the tree splits on the position of an item (not considering it's dimensions!)
//    - duplicate positions are not allowed (skipped)
//
//    nw(x,y)
//    x────────────┬────────────┐
//    │            │            │
//    │            │            │
//    │     nw     │     ne     │
//    │            │            │
//    │            │            │
//    ├────────────x────────────┤
//    │            │ c(x,y)     │
//    │            │            │
//    │     sw     │     se     │
//    │            │            │
//    │            │            │
//    └────────────┴────────────x
//                         se(x,y)

#define QNODE_CAPACITY 4

typedef enum{
    QDIR_NW,
    QDIR_NE,
    QDIR_SE,
    QDIR_SW
} QDir;

typedef struct {
    vec2 pos;
    // id
    int id;
    // void *data;
} QMember;

typedef struct QNode {
    rect area; //aabb

    struct QNode *parent;

    struct QNode *nw;
    struct QNode *ne;
    struct QNode *se;
    struct QNode *sw;

    unsigned int depth;

    // boxes and data strictly need to be kept in sync
    size_t sz;
    QMember *members; // array of void pointers
} QNode;

QNode *qnode_create(rect area);
QNode *qnode_insert(QNode *node, vec2 pos, int id);
QNode *qnode_get(QNode *node, vec2 pos);
QNode *qnode_contains(QNode *root, rect area);
void qnode_walk(QNode *root, void (*descent)(QNode *node), void (*ascent)(QNode *node));
void qnode_destroy(QNode *node);

size_t qnode_count(QNode *root); // counts quads! TODO, count on creation
void qnode_print(FILE *fp, QNode *node);

// exposed for tests
unsigned int _check_pos_inside(vec2 pos, rect outer);
unsigned int _check_rect_inside(rect inner, rect outer);
unsigned int _check_rect_overlaps(rect inner, rect outer);

#endif
