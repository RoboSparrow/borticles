#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "test.h"
#include "quadtree.h"


static unsigned int capacity = QNODE_CAPACITY;

static void test_tree() {
    DESCRIBE("node");
    QNode *root;

    root = qnode_create((rect) {0.f, 0.f, 600.f, 400.f});
    assert(root->parent == NULL);

    assert(root->nw == NULL);
    assert(root->ne == NULL);
    assert(root->se == NULL);
    assert(root->sw == NULL);

    qnode_destroy(root);
    DONE();
}

QNode *test_split() {
    DESCRIBE("node capacity");
    QNode *root;
    QNode *curr;
    QMember mem;
    unsigned int i;

    root = qnode_create((rect) {0.f, 0.f, 600.f, 400.f});
    assert(root->parent == NULL);

    assert(root->nw == NULL);
    assert(root->ne == NULL);
    assert(root->se == NULL);
    assert(root->sw == NULL);

    for (i = 0; i < capacity; i++) {
        curr = qnode_insert(root, (vec2) {i * 2.f, i * 2.f}, i);
        assert(curr == root);
        assert(root->sz == i + 1);
    }

    // capacity overflow: split the n
    curr = qnode_insert(root, (vec2) {301.f, 201.f}, 999);

    assert(curr != root);
    assert(curr != NULL);

    // root is a branch with no members
    assert(root->sz == 0);

    assert(root->nw != NULL);
    assert(root->ne != NULL);
    assert(root->se != NULL);
    assert(root->sw != NULL);

    // root->nw should contain the first 4 nodes
    assert(root->nw->sz == capacity);
    assert(root->ne->sz == 0);
    assert(root->se->sz == 1);
    assert(root->sw->sz == 0);

    // the first 4 members are in nw
    for (i = 0; i < capacity; i++) {
        mem = root->nw->members[i];
        assert(mem.id == i);
    }

    // the last member was insrted into se
    mem = root->se->members[0];
    assert(mem.id == 999);

    DONE();
    return root;
}

static void test_contains(QNode *root) {
    // see QNode *test_split()
    DESCRIBE("node contains area");

    QNode *src, *res;
    src = root->se; // {301.f, 201.f}

    // outside
    res = qnode_contains(src, (rect) {0.f, 0.f, 1.f, 1.f});
    assert(res == NULL);

    // partial overlap
    res = qnode_contains(src, (rect) {
        src->area.x      - 1.f,
        src->area.y      - 1.f,
        src->area.width,
        src->area.height
    });
    assert(res == NULL);

    // exact overlap
    res = qnode_contains(src, src->area);
    assert(res == src);

    // inside
    res = qnode_contains(src, (rect) {
        src->area.x      + 1.f,
        src->area.y      + 1.f,
        src->area.width  - 1.f,
        src->area.height - 1.f
    });
    assert(res == src);

    DONE();
}

static void test_duplicate_pos() {
    DESCRIBE("node capacity");
    QNode *root;
    QNode *curr;
    unsigned int i;

    root = qnode_create((rect) {0.f, 0.f, 600.f, 400.f});
    assert(root->parent == NULL);

    assert(root->nw == NULL);
    assert(root->ne == NULL);
    assert(root->se == NULL);
    assert(root->sw == NULL);

    curr = qnode_insert(root, (vec2) {0.f}, 0);
    assert(curr == root);
    assert(root->sz == 1);

    // duplicate pos will be ignored
    curr = qnode_insert(root, (vec2) {0.f}, 1);
    assert(curr == NULL);
    assert(root->sz == 1);

    qnode_destroy(root);
    DONE();
}

void test_quadtree(int argc, char **argv) {
    QNode *root;

    test_tree();
    test_duplicate_pos();

    root = test_split();
    test_contains(root);
}
