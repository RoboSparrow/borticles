#include <stdlib.h>
#include <math.h>

#include "log.h"
#include "utils.h"
#include "quadtree.h"

static int count = 0;

static unsigned int _has_duplicate_pos(QNode *node, vec2 pos) {
    if (!node->sz || !node->members) {
        return 0;
    }
    for (size_t i = 0; i < node->sz; i++) {
        if (node->members[i].pos.x == pos.x && node->members[i].pos.y == pos.y) {
            return 1;
        }
    }
    return 0;
}

unsigned int _check_pos_inside(vec2 pos, rect outer) {
    return outer.x <= pos.x
        && outer.y <= pos.y
        && outer.x + outer.width >= pos.x
        && outer.y + outer.height >= pos.y;
}

static QNode *_get_quadrant(vec2 pos, QNode *node) {
    if (!node || !node->nw) {
        return NULL;
    }
    if(_check_pos_inside(pos, node->nw->area)) {
        return node->nw;
    }
    if(_check_pos_inside(pos, node->ne->area)) {
        return node->ne;
    }
    if(_check_pos_inside(pos, node->se->area)) {
        return node->se;
    }
    if(_check_pos_inside(pos, node->sw->area)) {
        return node->sw;
    }
    return NULL;
}

static void _split(QNode *node) {
    if (node->nw) {
        return;
    }

    float x = node->area.x;
    float y = node->area.y;
    float w = node->area.width/2;
    float h = node->area.height/2;

    unsigned int depth = node->depth + 1;

    node->nw = qnode_create((rect){x,     y,     w, h});
    node->nw->depth = depth;
    node->ne = qnode_create((rect){x + w, y,     w, h});
    node->ne->depth = depth;
    node->se = qnode_create((rect){x + w, y + h, w, h});
    node->se->depth = depth;
    node->sw = qnode_create((rect){x,     y + h, w, h});
    node->sw->depth = depth;

    size_t i;
    int res;
    QNode *child;
    for (i = 0; i < node->sz; i++) {
        child = _get_quadrant(node->members[i].pos, node);
        if(child) {
            res = qnode_insert(child, node->members[i].pos, node->members[i].id);
            if (res == 0) {
                LOG_ERROR_F("error inserting node member %d", node->members[i].id);
            }
        }
    }
    node->sz = 0;
    freez(node->members);
    node->members = NULL;
}

static  int _count_quads(QNode *root) {
    count ++;
    if(root->nw != NULL) _count_quads(root->nw);
    if(root->ne != NULL) _count_quads(root->ne);
    if(root->sw != NULL) _count_quads(root->sw);
    if(root->se != NULL) _count_quads(root->se);
}

QNode *qnode_create(rect area) {
    QNode *node = calloc(1, sizeof(QNode));
    EXIT_IF(node == NULL, "failed to allocate for QNode");

    node->area = area;
    node->sz = 0;

    return node;
}

void qnode_destroy(QNode *node) {
    if (!node) {
        return;
    }
    qnode_destroy(node->nw);
    qnode_destroy(node->ne);
    qnode_destroy(node->se);
    qnode_destroy(node->sw);
    freez(node->members);
    free(node);
}

unsigned int qnode_insert(QNode *node, vec2 pos, int id) {
    // printf("---- insert id %d (%f,%f) into depth %d in area {%f,%f,%f,%f} (sz: %ld)\n", id, pos.x, pos.y, node->depth, node->area.x, node->area.y, node->area.x + node->area.width, node->area.y + node->area.height, node->sz);

    if(!_check_pos_inside(pos, node->area)) {
        return 0;
    }

    if (_has_duplicate_pos(node, pos)) {
        return 0;
    }

    QNode *child;

    // has children, then pass on
    if (node->nw) {
        child = _get_quadrant(pos, node);
        if (child) {
            return qnode_insert(child, pos, id);
        }
        return 0;
    }

    // leaf node

    if (!node->members) {
        node->members = malloc(QNODE_CAPACITY * sizeof(QNode));
        EXIT_IF(node->members == NULL, "failed to allocate for QNode members");
    }

    //  has a free slot
    if (node->sz < QNODE_CAPACITY) {
        node->members[node->sz].pos = pos;
        node->members[node->sz].id = id;
        node->sz++;
        return 1;
    }

    // othwerwise split (this node wont be a leaf from now)
    _split(node);
    return qnode_insert(node, pos, id);
}

/**
 * collisions can only appear between members within a quad node
 *
 * find the smallest node (leaf node) who encapsulates the given box area
 */
QNode *qnode_get(QNode *node, vec2 pos) {
    if (!_check_pos_inside(pos, node->area)) {
        return NULL;
    }

    // leaf node
    if (!node->nw) {
        return node;
    }

    QNode *child = _get_quadrant(pos, node);
    return qnode_get(child, pos);
}

void qnode_walk(QNode *root, void (*descent)(QNode *node), void (*ascent)(QNode *node)) {
    (*descent)(root);
    if(root->nw != NULL) qnode_walk(root->nw, descent, ascent);
    if(root->ne != NULL) qnode_walk(root->ne, descent, ascent);
    if(root->sw != NULL) qnode_walk(root->sw, descent, ascent);
    if(root->se != NULL) qnode_walk(root->se, descent, ascent);
    (*ascent)(root);
}

size_t qnode_count(QNode *root) {
    count = 0;
    if (root) {
        _count_quads(root);
    }
    return count;
}

void qnode_print(FILE *fp, QNode *node) {
    if(!fp) {
        return;
    }

    if(!node) {
        fprintf(fp, "NULL\n");
        return;
    }

    fprintf(fp,
        "{"
        " area: {x:%f, y:%f, w:%f, h:%f},"
        " nw:%c, ne:%c, se:%c, sw:%c,"
        " depth: %d, sz: %ld,",
        node->area.x, node->area.y, node->area.width, node->area.height,
        (node->nw) ? 'y' : '-', (node->ne) ? 'y' : '-', (node->se) ? 'y' : '-', (node->sw) ? 'y' : '-',
        node->depth, node->sz
    );
    if (node->members) {
        fprintf(fp, " members: [");
        for (size_t i = 0; i < node->sz; i++) {
            fprintf(fp, "%d%s", node->members[i].id, (i < node->sz - 1) ? ", " : "");
        }
        fprintf(fp, "]}");
    } else {
        fprintf(fp, " members: <NULL>");
    }
    fprintf(fp, " }\n");
}
