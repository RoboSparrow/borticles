#ifndef __QUADTREE_H__
#define __QUADTREE_H__

#include "utils.h"

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

typedef struct {
    vec2 pos;
    // id
    int id;
    // void *data;
}  QMember ;

typedef struct QNode {
    rect area; //aabb

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
unsigned int qnode_insert(QNode *node, vec2 pos, int id);
QNode *qnode_get(QNode *node, vec2 pos);
void qnode_walk(QNode *root, void (*descent)(QNode *node), void (*ascent)(QNode *node));
void qnode_destroy(QNode *node);

void qnode_print(FILE *fp, QNode *node);
#endif
