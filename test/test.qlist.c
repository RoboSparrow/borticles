#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "test.h"
#include "qtree/qtree.h"

typedef struct TestItem {
    int id;
    vec2 pos; // control data, will not be queried within qtree.h
    float mass;
} TestItem;


static int _in_list(int id, QList *list) {
    for (size_t i = 0; i < list->len; i++) {
        if (list->nodes[i] && list->nodes[i]->data) {
            TestItem *item = (TestItem*) list->nodes[i]->data;
            if (item->id == id) {
                return 1;
            }
        }
    }
    return 0;
}

static void test_qlist_core() {
    {
        DESCRIBE("qlist_create()");

        size_t sz = 2;
        QList *list = qlist_create(sz);
        // qnode_print(tree->root);Vec2

        assert(list->len == 0);
        assert(list->max == sz);
        assert(list->grow == sz);
        assert(list->nodes != NULL);

        qlist_destroy(list);
        DONE();
    } {
        DESCRIBE("qlist_append()");

        QNode q1 = {
            .pos={1.f, 1.f}
        };
        QNode q2 = {
            .pos={2.f, 2.f}
        };

        QList *list = qlist_create(1);
        assert(list->len == 0);
        assert(list->max == 1);
        assert(list->grow == 1);

        qlist_append(list, &q1);
        assert(list->len == 1);
        assert(list->max == 1);
        assert(list->grow == 1);

        qlist_append(list, &q2);
        assert(list->len == 2);
        assert(list->max == 2);
        assert(list->grow == 1);

        qlist_destroy(list);
        DONE();
    } {
        DESCRIBE("qlist_reset()");

        QNode q1 = {
            .pos={1.f, 1.f}
        };
        QNode q2 = {
            .pos={2.f, 2.f}
        };

        QList *list = qlist_create(1);
        qlist_append(list, &q1);
        qlist_append(list, &q2);

        assert(list->len == 2);
        assert(list->max == 2);
        assert(list->grow == 1);

        qlist_reset(list);

        assert(list->len == 0);
        assert(list->max == 2);
        assert(list->grow == 1);

        assert(list->nodes[0] == NULL);
        assert(list->nodes[1] == NULL);

        qlist_destroy(list);
        DONE();
    } {
        DESCRIBE("qlist_destroy()");

        qlist_destroy(NULL);

        DONE();
    }
}

static void test_qnode_within_area() {
    DESCRIBE("node covered by area");

    int res;
    QNode *node = qnode_create(NULL);
    node->self_nw = (vec2) {2.f, 2.f};
    node->self_se = (vec2) {5.f, 5.f};

    // outside area
    res = qnode_within_area(node, (vec2) {0.f, 0.f}, (vec2) {1.f, 1.f});
    assert(res == 0);

    // overlaps area
    res = qnode_within_area(node, (vec2) {0.f, 0.f}, (vec2) {3.f, 3.f});
    assert(res == 0);

    // exact coverage
    res = qnode_within_area(node, (vec2){2.f, 2.f}, (vec2) {5.f, 5.f});
    assert(res == 1);

    // inside area
    res = qnode_within_area(node, (vec2){1.f, 1.f}, (vec2) {6.f, 6.f});
    assert(res == 1);

    qnode_destroy(node);
    DONE();
}

static void test_qnode_overlaps_area() {
    DESCRIBE("node overlaps area");

    int res;
    QNode *node = qnode_create(NULL);
    node->self_nw = (vec2) {2.f, 2.f};
    node->self_se = (vec2) {5.f, 5.f};

    // outside area
    res = qnode_overlaps_area(node, (vec2) {0.f, 0.f}, (vec2) {1.f, 1.f});
    assert(res == 0);

    // overlaps area
    res = qnode_overlaps_area(node, (vec2) {0.f, 0.f}, (vec2) {3.f, 3.f});
    assert(res == 1);

    // exact coverage
    res = qnode_overlaps_area(node, (vec2){2.f, 2.f}, (vec2) {5.f, 5.f});
    assert(res == 1);

    // inside area
    res = qnode_overlaps_area(node, (vec2){1.f, 1.f}, (vec2) {6.f, 6.f});
    assert(res == 1);

    qnode_destroy(node);
    DONE();
}

static void test_find_in_area() {
    DESCRIBE("area");
    QTree *tree = qtree_create((vec2){1.f, 1.f}, (vec2) {10.f, 10.f});

    float radius = 2.f;
    vec2 pos = (vec2) {4.f, 4.f};

    TestItem ref = {0, pos, 1.f};

    vec2 nw = {ref.pos.x - radius, ref.pos.y - radius};
    vec2 se = {ref.pos.x + radius, ref.pos.y + radius};
    // printf("ref: {%f, %f}, nw: {%f, %f}, se: {%f, %f}\n", ref.pos.x, ref.pos.y, nw.x, nw.y, se.x, se.y);

    // inside
    TestItem itm1 = {
        .id=1,
        .pos=(vec2) {pos.x + (radius / 2.f), pos.y + (radius / 2.f)},
        .mass=1.f
    };

    TestItem itm2 = {
        .id=2,
        .pos=(vec2) {pos.x + radius, pos.y + radius},
        .mass=1.f
    };

    TestItem itm3 = {
        .id=3,
        .pos=(vec2) {pos.x - radius, pos.y - radius},
        .mass=1.f
    };

    // outside
    TestItem itm4 = {
        .id=4,
        .pos=(vec2) {1.f, 1.f},
        .mass=1.f
    };

    TestItem itm5 = {
        .id=5,
        .pos=(vec2) {pos.x, pos.y + radius + 0.1},
        .mass=1.f
    };

    TestItem itm6 = {
        .id=6,
        .pos=(vec2) {pos.x - radius - 0.1, pos.y},
        .mass=1.f
    };

    TestItem *inside[3]  = {&itm1, &itm2, &itm3};
    TestItem *outside[3] = {&itm4, &itm5, &itm6};

    QList *list = qlist_create(1);
    int res, i;

    res = qtree_insert(tree, &ref, ref.pos, ref.mass);
    assert(res == QUAD_INSERTED);

    // insert nodes
    for (i = 0; i < 3; i++) {
        res = qtree_insert(tree, inside[i], inside[i]->pos, inside[i]->mass);
        // printf("(%d): {%f, %f}, nw: {%f, %f}, se: {%f, %f}\n", inside[i]->id, inside[i]->pos.x, inside[i]->pos.y, nw.x, nw.y, se.x, se.y);
        assert(res == QUAD_INSERTED);
    }
    for (i = 0; i < 3; i++) {
        res = qtree_insert(tree, outside[i], outside[i]->pos, outside[i]->mass);
        // printf("(%d): {%f, %f}, nw: {%f, %f}, se: {%f, %f}\n", outside[i]->id, outside[i]->pos.x, outside[i]->pos.y, nw.x, nw.y, se.x, se.y);
        assert(res == QUAD_INSERTED);
    }
    assert(tree->length == 7); // 2 * 3 + 1

    // qtree_print(stdout, tree); exit(0);
    qtree_find_in_area(tree, pos, radius, list);

    assert(list != NULL);
    assert(list->len == 4);

    assert(_in_list(0, list));
    assert(_in_list(1, list));
    assert(_in_list(2, list));
    assert(_in_list(3, list));

    qlist_destroy(list);
    qtree_destroy(tree);
    DONE();
}

void test_qlist(int argc, char **argv) {
    test_qlist_core();
    test_qnode_within_area();
    test_qnode_overlaps_area();

    test_find_in_area();
}
