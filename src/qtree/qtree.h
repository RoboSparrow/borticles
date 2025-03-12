#ifndef __QTREE_H__
#define __QTREE_H__

#include <stdio.h>
#include "vec.h"

#define QUAD_FAILED -1
#define QUAD_INSERTED 0
#define QUAD_REPLACED 2

////
//   Quadrants
//
//   nw(x,y)
//   x────────────┬────────────┐
//   │            │            │
//   │            │            │
//   │     nw     │     ne     │
//   │            │            │
//   │            │            │
//   ├────────────x────────────┤
//   │            │ c(x,y)     │
//   │            │            │
//   │     sw     │     se     │
//   │            │            │
//   │            │            │
//   └────────────┴────────────x
//                        se(x,y)
////

typedef struct QNode {
    struct QNode *parent;

    struct QNode *ne;
    struct QNode *nw;
    struct QNode *se;
    struct QNode *sw;

    vec2 self_nw;
    vec2 self_se;

    // barnes- hut
    float mass;
    vec2 com; // center of mass: is == pos if node is a leaf

    // data
    vec2 pos;
    void *data; // this is the data position vector and not node the node pos: TODO rename
} QNode;

typedef struct QTree {
    QNode *root;
    unsigned int length;
} QTree;

QTree *qtree_create(vec2 window_nw, vec2 window_se);
void qtree_destroy(QTree *tree);

int qtree_insert(QTree *tree, void *data, vec2 pos, float mass);

QNode *qtree_find(QTree *tree, vec2 pos);
QNode *qtree_find_nearest(QTree *tree, vec2 pos);

QNode *qnode_create(QNode *parent);
void qnode_destroy(QNode *node);

int qnode_isempty(QNode *node);
int qnode_isleaf(QNode *node);
int qnode_ispointer(QNode *node);

int qnode_within_area(QNode *node, vec2 nw, vec2 se);
int qnode_overlaps_area(QNode *node, vec2 nw, vec2 se);

void qnode_walk(QNode *node, void (*descent)(QNode *node), void (*ascent)(QNode *node));

void qtree_print(FILE *fp, QTree *tree);
void qnode_print(FILE *fp, QNode *node);

////
// QList
////

typedef struct QList {
    size_t len;
    size_t grow;
    size_t max;
    QNode **nodes;
} QList;

QList *qlist_create(size_t max);
QList *qlist_append(QList *list, QNode *node);

void qlist_fill(QList *list, QNode *root);

void qlist_reset(QList *list);
void qlist_destroy(QList *list);

void qlist_print(FILE *fp, QList *list);

QList *qtree_find_in_area(QTree *tree, vec2 pos, float radius, QList *list);

#endif
