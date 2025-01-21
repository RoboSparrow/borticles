#include <stdlib.h>
#include <math.h>

#include "qnode.h"

#include "log.h"
#include "utils.h"

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

static QNode *_get_quadrant(vec2 pos, QNode *node) {
    if (!node || !node->nw) {
        return NULL;
    }
    if (_check_pos_inside(pos, node->nw->area)) {
        return node->nw;
    }
    if (_check_pos_inside(pos, node->ne->area)) {
        return node->ne;
    }
    if (_check_pos_inside(pos, node->se->area)) {
        return node->se;
    }
    if (_check_pos_inside(pos, node->sw->area)) {
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
    node->nw->parent = node;
    node->nw->depth = depth;

    node->ne = qnode_create((rect){x + w, y,     w, h});
    node->ne->parent = node;
    node->ne->depth = depth;

    node->se = qnode_create((rect){x + w, y + h, w, h});
    node->se->parent = node;
    node->se->depth = depth;

    node->sw = qnode_create((rect){x,     y + h, w, h});
    node->sw->parent = node;
    node->sw->depth = depth;

    size_t i;
    QNode *child, *res;
    for (i = 0; i < node->sz; i++) {
        child = _get_quadrant(node->members[i].pos, node);
        if (child) {
            res = qnode_insert(child, node->members[i].pos, node->members[i].id);
            if (res == NULL) {
                LOG_ERROR_F("error inserting node member %d", node->members[i].id);
            }
        }
    }

    node->sz = 0;
    freez(node->members);
    node->members = NULL;
}

static void _count_quads(QNode *root) {
    count ++;
    if (root->nw != NULL) _count_quads(root->nw);
    if (root->ne != NULL) _count_quads(root->ne);
    if (root->sw != NULL) _count_quads(root->sw);
    if (root->se != NULL) _count_quads(root->se);
}

/**
 * get the region of a child node
 */
static int _get_region(QNode *root) {
    if (!root->parent) return -1;

    if (root->parent->nw == root) return QDIR_NW;
    if (root->parent->ne == root) return QDIR_NE;
    if (root->parent->se == root) return QDIR_SE;
    if (root->parent->sw == root) return QDIR_SW;

    return -1;
}

unsigned int _check_pos_inside(vec2 pos, rect outer) {
    return outer.x <= pos.x
        && outer.y <= pos.y
        && outer.x + outer.width >= pos.x
        && outer.y + outer.height >= pos.y;
}

unsigned int _check_rect_inside(rect inner, rect outer) {
    return inner.x >= outer.x
        && inner.y >= outer.y
        && (inner.x + inner.width)  <= (outer.x + outer.width)
        && (inner.y + inner.height) <= (outer.y + outer.height);
}

unsigned int _check_rect_overlaps(rect inner, rect outer) {
    return inner.x < (outer.x + outer.width)
        && inner.y < (outer.y + outer.height)
        && (inner.x + inner.width)  > outer.x
        && (inner.y + inner.height) > outer.y;
}

QNode *qnode_create(rect area) {
    QNode *node = calloc(1, sizeof(QNode));
    EXIT_IF(node == NULL, "failed to allocate for QNode");

    node->area = area;
    node->sz = 0;
    node->parent = NULL;
    node->members = NULL;

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
    freez(node);
}

QNode *qnode_insert(QNode *node, vec2 pos, int id) {
    // printf("---- insert id %d (%f,%f) into depth %d in area {%f,%f,%f,%f} (sz: %ld)\n", id, pos.x, pos.y, node->depth, node->area.x, node->area.y, node->area.x + node->area.width, node->area.y + node->area.height, node->sz);

    if (!_check_pos_inside(pos, node->area)) {
        return NULL;
    }

    if (_has_duplicate_pos(node, pos)) {
        return NULL;
    }

    QNode *child;

    // has children, then pass on
    if (node->nw) {
        child = _get_quadrant(pos, node);
        if (child) {
            return qnode_insert(child, pos, id);
        }
        return NULL;
    }

    // leaf node

    if (!node->members) {
        node->members = malloc(QNODE_CAPACITY * sizeof(QMember));
        EXIT_IF(node->members == NULL, "failed to allocate for QNode members");
    }

    //  has a free slot

    if (node->sz < QNODE_CAPACITY) {
        node->members[node->sz].pos = pos;
        node->members[node->sz].id = id;
        node->sz++;
        return node;
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

/**
 * Finds the lowest (smallest) quad encompassing a given area
 */
QNode *qnode_contains(QNode *root, rect area) {
    // leaf node
    if (!root->nw) {
        if (_check_rect_inside(area, root->area)) {
            return root;
        }
        return NULL;
    }

    // if one of the children encloses the area then pass search on
    if (_check_rect_inside(area, root->nw->area)) {
        return qnode_contains(root->nw, area);
    }
    if (_check_rect_inside(area, root->ne->area)) {
        return qnode_contains(root->ne, area);
    }
    if (_check_rect_inside(area, root->se->area)) {
        return qnode_contains(root->se, area);
    }
    if (_check_rect_inside(area, root->sw->area)) {
        return qnode_contains(root->sw, area);
    }

    return root;
}

void qnode_walk(QNode *root, void (*descent)(QNode *node), void (*ascent)(QNode *node)) {
    (*descent)(root);

    if (root->nw != NULL) {
        qnode_walk(root->nw, descent, ascent);
    }
    if (root->ne != NULL) {
        qnode_walk(root->ne, descent, ascent);
    }
    if (root->sw != NULL) {
        qnode_walk(root->sw, descent, ascent);
    }
    if (root->se != NULL) {
        qnode_walk(root->se, descent, ascent);
    }
    if (ascent) {
        (*ascent)(root);
    }
}

size_t qnode_count(QNode *root) {
    count = 0;
    if (root) {
        _count_quads(root);
    }
    return count;
}

void qnode_print(FILE *fp, QNode *node) {
    if (!fp) {
        return;
    }

    if (!node) {
        fprintf(fp, "NULL\n");
        return;
    }

    fprintf(fp,
        "{"
        " area: {x:%f, y:%f, w:%f, h:%f},"
        " parent: %s,"
        " nw:%c, ne:%c, se:%c, sw:%c,"
        " depth: %d, sz: %ld,",
        node->area.x, node->area.y, node->area.width, node->area.height,
        (node->parent) ? "y" : "<NULL>",
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
