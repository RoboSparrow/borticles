#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <assert.h>

#include "test.h"
#include "qtree/qtree.h"

typedef struct TestItem {
    int id;
    vec2 pos; // control data, will not be queried within qtree.h
    float mass;
} TestItem;

static void test_tree() {
    DESCRIBE("tree");
    QTree *tree;
    QNode *root;

    tree = qtree_create((vec2) {0}, (vec2) {600.f, 400.f});
    assert(tree->length == 0);

    root = tree->root;
    assert(!qnode_isleaf(root));
    assert(qnode_isempty(root));
    assert(!qnode_ispointer(root));

    assert(root->self_nw.x == 0);
    assert(root->self_nw.y == 0);
    assert(root->self_se.x == 600.0);
    assert(root->self_se.y == 400.0);

    qtree_destroy(tree);
    DONE();
}

static void test_node() {
    DESCRIBE("node");
    QNode *node;

    node = qnode_create(NULL);

    assert(!qnode_isleaf(node));
    assert(qnode_isempty(node));
    assert(!qnode_ispointer(node));
    assert(node->parent == NULL);

    qnode_destroy(node);
    DONE();
}

static void test_tree_insert() {
    QTree *tree =  qtree_create((vec2) {1.f, 1.f}, (vec2) {10.f, 10.f});
    assert(tree != NULL);
    assert(tree->root->self_nw.x == 1.0);
    assert(tree->root->self_nw.y == 1.0);
    assert(tree->root->self_se.x == 10.0);
    assert(tree->root->self_se.y == 10.0);

    TestItem itm1 = {111, {8.f, 2.f}, 1.f};
    TestItem itm2 = {222, {1.f, 1.f}, 1.f};

    {
        DESCRIBE("test_qtree_insert(first node)");
        int res = qtree_insert(tree, &itm1, itm1.pos, itm1.mass);
        // qnode_print(tree->root);

        assert(tree->root->data != NULL);
        assert(res == QUAD_INSERTED);
        assert(tree->length == 1);

        assert(tree->root->nw == NULL);
        assert(tree->root->ne == NULL);
        assert(tree->root->se == NULL);
        assert(tree->root->sw == NULL);

        // verify node idendity
        assert(tree->root->data != NULL);
        assert(tree->root->pos.x == itm1.pos.x);
        assert(tree->root->pos.y == itm1.pos.y);

        // verify barnes-hut params
        assert(tree->root->mass == itm1.mass);
        assert(tree->root->com.x == itm1.pos.x);
        assert(tree->root->com.y == itm1.pos.y);

        TestItem *item = (TestItem*) tree->root->data;
        assert(item->id == itm1.id);
        DONE();
    } {
        DESCRIBE("test_qtree_insert(second node)");
        int res = qtree_insert(tree, &itm2, itm2.pos, itm2.mass);
        // qnode_print(tree->root);

        assert(tree->root->data == NULL); // 111 has been moved
        assert(res == QUAD_INSERTED);
        assert(tree->length == 2);

        // splitting
        assert(tree->root->nw != NULL);
        assert(tree->root->ne != NULL);
        assert(tree->root->se != NULL);
        assert(tree->root->sw != NULL);

        // verify node idendity
        assert(tree->root->ne->data != NULL);
        assert(tree->root->ne->pos.x == itm1.pos.x);
        assert(tree->root->ne->pos.y == itm1.pos.y);

        TestItem *item = (TestItem*) tree->root->ne->data;
        assert(item->id == itm1.id);
        DONE();
    }

    qtree_destroy(tree);
}

static void test_tree_insert_outside() {
    DESCRIBE("pos outside of tree");
    QTree *tree = qtree_create((vec2) {1.f, 1.f}, (vec2) {10.f, 10.f});

    TestItem itm = {111, {0.f, 0.f}, 1.f};

    int res;
    res = qtree_insert(tree, &itm, itm.pos, itm.mass);

    assert(tree->root->data == NULL);
    assert(tree->root->pos.x == 0.f);
    assert(tree->root->pos.y == 0.f);
    assert(res == QUAD_FAILED);

    qtree_destroy(tree);
    DONE();
}

static void test_tree_insert_replace() {
    DESCRIBE("replace if (n2.pos == n1.pos)");
    QTree *tree = qtree_create((vec2) {1.f, 1.f}, (vec2) {10.f, 10.f});

    TestItem itm1 = {111, {8.f, 2.f}, 1.f};
    TestItem itm2 = {222, {8.f, 2.f}, 1.f};

    int res;
    TestItem *item;

    {
        // first node
        res = qtree_insert(tree, &itm1, itm1.pos, itm1.mass);

        assert(res == QUAD_INSERTED);
        assert(tree->length == 1);

        assert(tree->root->data != NULL);
        assert(tree->root->pos.x == itm1.pos.x);
        assert(tree->root->pos.y == itm1.pos.y);

        item = (TestItem*) tree->root->data;
        assert(item->id == itm1.id);
    } {
        // second node replaces first
        res = qtree_insert(tree, &itm2, itm2.pos, itm2.mass);

        assert(res == QUAD_REPLACED);
        assert(tree->length == 1);

        assert(tree->root->data != NULL);
        assert(tree->root->pos.x == itm2.pos.x);
        assert(tree->root->pos.y == itm2.pos.y);

        item = (TestItem*) tree->root->data;
        assert(item->id == itm2.id);

        // splitting
        {
            assert(tree->root->nw == NULL);
            assert(tree->root->ne == NULL);
            assert(tree->root->se == NULL);
            assert(tree->root->sw == NULL);
        }
    }

    qtree_destroy(tree);
    DONE();
}

static void test_tree_find() {
    DESCRIBE("find");
    QTree *tree = qtree_create((vec2) {1.f, 1.f}, (vec2) {10.f, 10.f});

    TestItem itm1 = {111, {8.f, 2.f}, 1.f};
    TestItem itm2 = {222, {1.f, 1.f}, 1.f};

    QNode *node;
    TestItem *item;
    int res;
    vec2 search;

    {
        // insert nodes
        res = qtree_insert(tree, &itm1, itm1.pos, itm1.mass);
        assert(res == QUAD_INSERTED);

        res = qtree_insert(tree, &itm2, itm2.pos, itm2.mass);
        assert(res == QUAD_INSERTED);

        assert(tree->length == 2);
    } {
        // find second non-existing node;
        search = (vec2){0};
        node = qtree_find(tree, search);
        assert(node == NULL);
    } {
        // find second item;
        search = itm2.pos;
        node = qtree_find(tree, search);
        assert(node != NULL);

        item = (TestItem*) node->data;
        assert(item->id == itm2.id);
    } {
       //find first item
        search = itm1.pos;
        node = qtree_find(tree, search);

        item = (TestItem*) node->data;
        assert(item->id == itm1.id);
    }

    qtree_destroy(tree);
    DONE();
}

static void test_node_parent() {
    DESCRIBE("parent");
    QTree *tree = qtree_create((vec2) {1.f, 1.f}, (vec2) {10.f, 10.f});

    // a nested tree with three levels
    TestItem itm1 = {111, {8.f, 2.f}, 1.f};
    TestItem itm2 = {222, {9.f, 1.f}, 1.f};

    QNode *node, *parent;
    TestItem *item;
    int res;
    vec2 search;

    {
        // insert nodes
        res = qtree_insert(tree, &itm1, itm1.pos, itm1.mass);
        assert(res == QUAD_INSERTED);

        res = qtree_insert(tree, &itm2, itm2.pos, itm2.mass);
        assert(res == QUAD_INSERTED);

        assert(tree->length == 2);
    } {
        // find first parent
        parent = tree->root;
        node = tree->root->ne;

        assert(node != NULL);
        assert(node->parent != NULL);

        assert(node->parent->self_nw.x == parent->self_nw.x);
        assert(node->parent->self_se.y == parent->self_se.y);
    } {
        // find second parent
        parent = tree->root->ne;
        node   = tree->root->ne->ne;

        assert(node != NULL);
        assert(node->parent != NULL);

        assert(node->parent->self_nw.x == parent->self_nw.x);
        assert(node->parent->self_se.y == parent->self_se.y);

        // qnode_print(stderr, node);
    } {
        // find first leaf
        parent = tree->root->ne->ne;
        node   = tree->root->ne->ne->nw;

        assert(node != NULL);
        assert(node->parent != NULL);

        assert(node->parent->self_nw.x == parent->self_nw.x);
        assert(node->parent->self_se.y == parent->self_se.y);

        // data
        assert(node->data != NULL);

        item = (TestItem*) node->data;
        assert(item->id == itm1.id);

        // qnode_print(stderr, node);
    } {
        // find second leaf
        parent = tree->root->ne->ne;
        node   = tree->root->ne->ne->ne;

        assert(node != NULL);
        assert(node->parent != NULL);

        assert(node->parent->self_nw.x == parent->self_nw.x);
        assert(node->parent->self_se.y == parent->self_se.y);

        // data
        assert(node->data != NULL);

        item = (TestItem*) node->data;
        assert(item->id == itm2.id);

        // qnode_print(stderr, node);
    }

    qtree_destroy(tree);
    DONE();
}

/**
 * @see: https://www.omnicalculator.com/math/center-of-mass
 */
static void test_node_mass() {
    QTree *tree =  qtree_create((vec2) {1.f, 1.f}, (vec2) {10.f, 10.f});

    TestItem itm1 = {111, {8.f, 2.f}, 1.f};
    TestItem itm2 = {222, {1.f, 1.f}, 2.f};

    {
        DESCRIBE("test_qtree_insert(first node)");
        int res = qtree_insert(tree, &itm1, itm1.pos, itm1.mass);

        // verify distribution
        assert(tree->root->data != NULL);
        assert(tree->length == 1);

        // verify barnes-hut params
        assert(tree->root->mass == itm1.mass);
        assert(tree->root->com.x == itm1.pos.x);
        assert(tree->root->com.y == itm1.pos.y);

        // verify node data
        TestItem *item = (TestItem*) tree->root->data;
        assert(item->id == itm1.id);
        DONE();
    } {
        DESCRIBE("test_qtree_insert(second node)");
        int res = qtree_insert(tree, &itm2, itm2.pos, itm2.mass);
        qtree_print(stderr, tree);

        // verify distribution
        assert(tree->root->data == NULL);
        assert(tree->root->ne->data != NULL);
        assert(tree->length == 2);

        // verify node idendity
        assert(tree->root->mass == itm1.mass + itm2.mass);
        ASSERT_FLOAT(tree->root->com.x, 3.333f, 0.001);
        ASSERT_FLOAT(tree->root->com.y, 1.333f, 0.001);

        TestItem *item;
        item = (TestItem*) tree->root->nw->data;
        assert(item->id == itm2.id);
        item = (TestItem*) tree->root->ne->data;
        assert(item->id == itm1.id);

        DONE();
    }

    qtree_destroy(tree);
}

void test_qtree(int argc, char **argv) {
    test_tree();
    test_node();
    test_tree_insert();
    test_tree_insert_outside();
    test_tree_insert_replace();
    test_tree_find();
    test_node_parent();
    test_node_mass();
}
