#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "qtree.h"
#include "log.h"
#include "utils.h"

////
// QNode
////

// forward declarations
static int _node_split(QTree *tree, QNode *node);
void qnode_print(FILE *fp, QNode *node);

/**
 * checks if a point is inside a given area
 */

static int _vec2_within(vec2 pos, vec2 nw, vec2 se) {
    return pos.x >= nw.x && pos.y >= nw.y && pos.x <= se.x && pos.y <= se.y;
}

/**
 * Checks if a pos is with an node boundary.
 */
static int _node_contains(QNode *node, vec2 pos) {
    // printf(" -- nw: {%f, %f}, se: {%f, %f}, pos: {%f, %f}\n", node->self_nw.x, node->self_nw.y, node->self_se.x, node->self_se.y, pos.x, pos.y);
    return _vec2_within(pos, node->self_nw, node->self_se);
}

/**
 * Gets the matching quadrant child node for a given position.
 */
static QNode *_node_quadrant(QNode *node, vec2 pos) {
    // printf("-- nw: {%f, %f}, se: {%f, %f}, pos: {%f, %f}\n", node->self_nw.x, node->self_nw.y, node->self_se.x, node->self_se.y, pos.x, pos.y);
    if (_node_contains(node->nw, pos)) {
        return node->nw;
    }
    if (_node_contains(node->ne, pos)) {
        return node->ne;
    }
    if (_node_contains(node->sw, pos)) {
        return node->sw;
    }
    if (_node_contains(node->se, pos)) {
        return node->se;
    }
    return NULL;
}

/**
 * Sets the node boundaries
 */
static void _set_bounds(QNode *node, vec2 nw, vec2 se) {
    node->self_nw = nw;
    node->self_se = se;
}

/**
 * Clears data properites in a node
 */
static void _node_clear_data(QNode *node) {
    node->pos = (vec2){0.f};
    node->data = NULL;
    node->mass = 0.f;
    node->com = (vec2){0.f};
}

static void _node_update_gravity(QNode *node, vec2 pos, float mass)  {
    if (!node) {
        return;
    }

    node->com.x = ( node->com.x * node->mass + pos.x * mass) / (node->mass + mass);
    node->com.y = ( node->com.y * node->mass + pos.y * mass) / (node->mass + mass);
    node->mass += mass;
}

/**
 * Inserts an entity into a tree node. The node might be split into four childs, or the  already existing entity in this node might be replaced
 * Note: The position bounds must be checked by callee (qtree_insert())
 */
static int _node_insert(QTree *tree, QNode *node, void *data, vec2 pos, float mass) {
    if (!tree || !node || !data) {
        return QUAD_FAILED;
    }

    // update mass and center of mass for this node

    // 1. insert into THIS (empty) node (just created before)
    if (qnode_isempty(node)) {
        node->pos = pos;
        node->data = data;
        _node_update_gravity(node, pos, mass);
        _node_update_gravity(node->parent, pos, mass);
        return QUAD_INSERTED;
    }

    // 2. replace THIS node OR split and insert into CHILDREN
    if (qnode_isleaf(node)) {
        // 2.1 pos match: replace
        if (node->pos.x == pos.x && node->pos.y == pos.y) {
            node->pos = pos;
            node->data = data;
            _node_update_gravity(node, pos, mass);
            _node_update_gravity(node->parent, pos, mass);
            return QUAD_REPLACED;
        }

        // 2.2 split node (and also mv previous node)
        if (_node_split(tree, node) == QUAD_FAILED) {
            return QUAD_FAILED;
        }

        // 2.3. insertcurrent node
        return _node_insert(tree, node, data, pos, mass);
    }

    // 3. insert into one of THIS CHILDREN
    if (qnode_ispointer(node)) {
        QNode *child = _node_quadrant(node, pos);
        if (!child) {
            return QUAD_FAILED;
        }
        return _node_insert(tree, child, data, pos, mass);
    }

    return QUAD_FAILED;
}

/**
 * Spits a quadrant nodes into 4 child quadrants.
 * Moves a existing entity node into the matching quadrant.
 */
static int _node_split(QTree *tree, QNode *node) {
    if (!tree || !node) {
        return QUAD_FAILED;
    }

    QNode *nw = qnode_create(node);
    QNode *ne = qnode_create(node);
    QNode *sw = qnode_create(node);
    QNode *se = qnode_create(node);

    if (!nw || !ne || !sw || !se) {
        return QUAD_FAILED;
    }

    void *data = node->data;
    vec2 pos = node->pos;
    float mass  = node->mass;

    // nw(x,y)            hw
    // x────────────┬────────────┐
    // │            │            │
    // │            │            │
    // │     nw     │     ne     │ hh
    // │            │            │
    // │            │            │
    // ├────────────x────────────┤
    // │            │ c(x,y)     │
    // │            │            │
    // │     sw     │     se     │
    // │            │            │
    // │            │            │
    // └────────────┴────────────x
    //                     se(x,y)

    float minx = node->self_nw.x;
    float miny = node->self_nw.y;

    float maxx = node->self_se.x;
    float maxy = node->self_se.y;

    float ctrx = minx + ((maxx - minx) / 2);
    float ctry = miny + ((maxy - miny) / 2);

    /*                         nw                     se              */
    _set_bounds(nw, (vec2){minx, miny}, (vec2){ctrx, ctry});
    _set_bounds(ne, (vec2){ctrx, miny}, (vec2){maxx, ctry});
    _set_bounds(se, (vec2){ctrx, ctry}, (vec2){maxx, maxy});
    _set_bounds(sw, (vec2){minx, ctry}, (vec2){ctrx, maxy});

    node->nw = nw;
    node->ne = ne;
    node->sw = sw;
    node->se = se;

    _node_clear_data(node);
    return _node_insert(tree, node, data, pos, mass); // inserts into one of the children
}

/**
 * Find the smallest qnode (leaf) who cony a given position
 */
static QNode *_node_find_nearest(QTree *tree, QNode *node, vec2 pos) {
    if (!tree || !node) {
        return NULL;
    }

    if (qnode_isleaf(node)) {
        return node;
    }

    if (qnode_ispointer(node)) {
        QNode *child = _node_quadrant(node, pos);
        if (!child) {
            return NULL;
        }
        return _node_find_nearest(tree, child, pos);
    }

    return NULL;
}

/**
 * Find a node for a given position
 */
static QNode *_node_find(QTree *tree, QNode *node, vec2 pos) {
    if (!tree || !node) {
        return NULL;
    }

    if (qnode_isleaf(node)) {
        if (node->pos.x == pos.x && node->pos.y == pos.y) {
            return node;
        }
    }

    if (qnode_ispointer(node)) {
        QNode *child = _node_quadrant(node, pos);
        if (!child) {
            return NULL;
        }
        return _node_find(tree, child, pos);
    }

    return NULL;
}

static void _node_collect(QNode *node, QList *list) {
    if (!node || !list) {
        return;
    }

    if (node->nw) {
        _node_collect(node->nw, list);
    }
    if (node->ne) {
        _node_collect(node->nw, list);
    }
    if (node->se) {
        _node_collect(node->nw, list);
    }
    if (node->sw) {
        _node_collect(node->nw, list);
    }
}

static void _node_find_in_area(QNode *node, vec2 nw, vec2 se, QList *list) {
    if (!node || !list) {
        return;
    }
    // printf(" --- node->nw: {%f, %f}, node->se: {%f, %f}, nw: {%f, %f}, se: {%f, %f}\n", node->self_nw.x, node->self_nw.y, node->self_se.x, node->self_se.y, nw.x, nw.y, se.x, se.y);

    // this node does not interesect with the search boundary
    // stop searching this branch
    if (!qnode_overlaps_area(node, nw, se)) {
        return;
    }

    // this is a data node (and thus without children)
    if (qnode_isleaf(node)) {
        if (_vec2_within(node->pos, nw, se)) {
            qlist_append(list, node);
        }
        return;
    }

    if (node->nw) {
        _node_find_in_area(node->nw, nw, se, list);
    }
    if (node->ne) {
        _node_find_in_area(node->ne, nw, se, list);
    }
    if (node->se) {
        _node_find_in_area(node->se, nw, se, list);
    }
    if (node->sw) {
        _node_find_in_area(node->sw, nw, se, list);
    }
}

// --- public

QNode *qnode_create(QNode *parent) {
    QNode *node = malloc(sizeof(QNode));
    if (!node) {
        LOG_ERROR("failed to allaocate memory for QNode");
        return NULL;
    }

    node->parent = parent;

    node->ne = NULL;
    node->nw = NULL;
    node->se = NULL;
    node->sw = NULL;

    node->self_nw = (vec2){0};
    node->self_se = (vec2){0};

    _node_clear_data(node);
    return node;
}

void qnode_destroy(QNode *node) {
    if (!node) {
        return;
    }
    if (node->nw) {
        qnode_destroy(node->nw);
    }
    if (node->ne) {
        qnode_destroy(node->ne);
    }
    if (node->sw) {
        qnode_destroy(node->sw);
    }
    if (node->se) {
        qnode_destroy(node->se);
    }

    // We  don not manage the memory of the data item
    _node_clear_data(node);

    freez(node);
}

int qnode_isleaf(QNode *node) {
    return node->data != NULL;
}

// TODO rename
int qnode_ispointer(QNode *node) {
    return node->nw != NULL && node->ne != NULL && node->sw != NULL && node->se != NULL && !qnode_isleaf(node);
}

int qnode_isempty(QNode *node) {
    return node->nw == NULL && node->ne == NULL && node->sw == NULL && node->se == NULL && !qnode_isleaf(node);
}

/**
 * checks if the area of a qnode is fully enclosed by a given area
 */
int qnode_within_area(QNode *node, vec2 nw, vec2 se) {
    return node != NULL && node->self_nw.x >= nw.x && node->self_se.x <= se.x && node->self_nw.y >= nw.y && node->self_se.y <= se.y;
}

/**
 * checks if the area of a qnode is fully overlaps a given area
 */
int qnode_overlaps_area(QNode *node, vec2 nw, vec2 se) {
    return node != NULL && node->self_nw.x < se.x && node->self_se.x >= nw.x && node->self_nw.y < se.y && node->self_se.y >= nw.y;
}

/**
 * recusively walk trough a quad node's children and apply on before (recusing) and on after call backs
 */
void qnode_walk(QNode *node, void (*descent)(QNode *node), void (*ascent)(QNode *node)) {
    if (descent) {
        (*descent)(node);
    }
    if (node->nw != NULL) {
        qnode_walk(node->nw, descent, ascent);
    }
    if (node->ne != NULL) {
        qnode_walk(node->ne, descent, ascent);
    }
    if (node->sw != NULL) {
        qnode_walk(node->sw, descent, ascent);
    }
    if (node->se != NULL) {
        qnode_walk(node->se, descent, ascent);
    }

    if (ascent) {
        (*ascent)(node);
    }
}

////
// QTree
////

QTree *qtree_create(vec2 window_nw, vec2 window_se) {
    assert(window_nw.x < window_se.x);
    assert(window_nw.y < window_se.y);

    QTree *tree = malloc(sizeof(QTree));
    if (!tree) {
        return NULL;
    }

    tree->root = qnode_create(NULL);
    if (!tree->root) {
        return NULL;
    }

    _set_bounds(tree->root, window_nw, window_se);
    tree->length = 0;

    return tree;
}

void qtree_destroy(QTree *tree) {
    if (!tree) {
        return;
    }
    qnode_destroy(tree->root);
    freez(tree);
}

int qtree_insert(QTree *tree, void *data, vec2 pos, float mass) {
    if (!tree || !data) {
        return QUAD_FAILED;
    }

    // check if pos is in tree bounds
    if (!_node_contains(tree->root, pos)) {
        return QUAD_FAILED;
    }

    int status = _node_insert(tree, tree->root, data, pos, mass);
    if (status == QUAD_INSERTED) {
        tree->length++;
    }

    return status;
}

/**
 * Find a qnode who matches exact a given position
 */
QNode *qtree_find(QTree *tree, vec2 pos) {
    if (!tree) {
        return NULL;
    }
    return _node_find(tree, tree->root, pos);
}

/**
 * Find the smallest qnode (leaf) who contains a given position
 */
QNode *qtree_find_nearest(QTree *tree, vec2 pos) {
    if (!tree) {
        return NULL;
    }
    return _node_find_nearest(tree, tree->root, pos);
}

QList *qtree_find_in_area(QTree *tree, vec2 pos, float radius, QList *list) {
    if (!tree || !list) {
        return NULL;
    }

    vec2 nw = {pos.x - radius, pos.y - radius};
    vec2 se = {pos.x + radius, pos.y + radius};
    _node_find_in_area(tree->root, nw, se, list);

    return list;
}

////
// debug
////

FILE *_out = NULL;

static void _print_desc(QNode *node) {
    qnode_print(_out, node);
    fprintf(_out, "\n");
}

static void _print_asc(QNode *node) {}

// --- public

void qtree_print(FILE *fp, QTree *tree) {
    if (!tree) {
        fprintf(fp, "<NULL>");
        return;
    }

    _out = fp;
    qnode_walk(tree->root, _print_asc, _print_desc);
    _out = NULL;
}

void qnode_print(FILE *fp, QNode *node) {
    if (!node) {
        fprintf(fp, "<NULL>");
        return;
    }

    fprintf(fp, "{self_nw: {%f, %f}, self_se: {%f, %f}, ", node->self_nw.x, node->self_nw.y, node->self_se.x, node->self_se.y);
    fprintf(fp, "parent: '%c', ", (node->parent) ? 'y' : '-');
    fprintf(fp, "nw: '%c', sw: '%c', se: '%c', nw: '%c', ", (node->nw) ? 'y' : '-', (node->sw) ? 'y' : '-', (node->se) ? 'y' : '-', (node->ne) ? 'y' : '-');
    fprintf(fp, "mass: %f, com: {%f, %f}, ", node->mass, node->com.x, node->com.y);

    if (node->pos.x < INFINITY) {
        fprintf(fp, "pos: {%f, %f}, ", node->pos.x, node->pos.y);
    } else {
        fprintf(fp, "pos: {INF..,INF..}}");
    }

    if (node->data) {
        fprintf(fp, "data: %p}", node->data);
    } else {
        fprintf(fp, "data: '-'}");
    }
}

////
// QList
////

QList *qlist_create(size_t max) {
    QList *list = malloc(sizeof(QList));
    if (!list) {
        LOG_ERROR("error allocating memory for quadlist");
        return NULL;
    }

    list->len = 0;
    list->grow = max;
    list->max = max;

    list->nodes = calloc(sizeof(QNode*), max);
    if (!list->nodes) {
        LOG_ERROR("error re-allocating memory for quadlist nodes");
        freez(list);
        return NULL;
    }
    return list;
}

QList *qlist_append(QList *list, QNode *node) {
    if (!list || !node) {
        return NULL;
    }

    if (list->len >= list->max) {
        list->max += list->grow;
        list->nodes = realloc(list->nodes, list->max * sizeof(QNode*));
        if (!list->nodes) {
            LOG_ERROR("error re-allocating memory for quadlist nodes");
            freez(list);
            return NULL;
        }
    }
    list->nodes[list->len] = node;
    list->len++;

    return list;
}

void qlist_fill(QList *list, QNode *root) {
    if (!list || !root) {
        return;
    }

    qlist_append(list, root);

    qlist_append(list, root->nw);
    qlist_append(list, root->ne);
    qlist_append(list, root->sw);
    qlist_append(list, root->se);

}

void qlist_reset(QList *list) {
    if (!list) {
        return;
    }
    for (size_t i = 0; i < list->len; i++) {
        list->nodes[i] = NULL; // leave allocated mem
    }
    list->len = 0;
}

void qlist_destroy(QList *list) {
    if (!list) {
        return;
    }
    freez(list->nodes); // free the list, not the nodes!
    freez(list);
}

void qlist_print(FILE *fp, QList *list) {
    if (!fp) {
        return;
    }
    if (!list) {
        fprintf(fp, "<NULL>\n");
        return;
    }

    printf(
        "{\n"
        "  len: %ld\n"
        "  max: %ld\n"
        "  grow: %ld\n"
        "  nodes: [",
        list->len,
        list->max,
        list->grow);
    if (list->nodes) {
        for (size_t i = 0; i < list->len; i++) {
            if (list->nodes[i]) {
                printf("{pos:{%f, %f}}", list->nodes[i]->pos.x, list->nodes[i]->pos.y);
            }
            printf("%s", (i < list->len - 1) ? ", " : "");
        }
    }
    printf("]\n}\n");
}
