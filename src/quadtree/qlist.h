#ifndef __QARRAY_H__
#define __QARRAY_H__

#include "vec.h"
#include "quadtree/qnode.h"

#define QTREE_RENDER_MAX 1000 * 4

struct QList {
    vec2 vertexes[QTREE_RENDER_MAX];
    size_t v_count;
};

void qlist_create(QNode *root, struct QList *qa);
void qlist_print(FILE *fp, struct QList *quads);
#endif
