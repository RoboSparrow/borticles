#include "borticle.h"
#include "qtree/qtree.h"
#include "state.h"

/**
 * Applies Newton's law of universal gravitation
 *
 * @see https://en.wikipedia.org/wiki/Newton%27s_law_of_universal_gravitation
 */
static float _calculate_gravitational_force(float mass1, float mass2, float radius, float grav_const) {
    if (fabsf(radius) == 0.f) {
        return 0.f;
    }
    return grav_const * ((mass1 * mass2) / (radius * radius));
}

/**
 * Compute the forces excerted on the particles, using the Barnes-Hut Approximation
 * @see https://www.cs.princeton.edu/courses/archive/fall03/cs126/assignments/barnes-hut.html
 */
static void _calculate_force(Borticle *bort, QNode *node, vec2 *delta, float theta, float grav_g, int *count) {
    float force = 0.f;

    if(!bort || !node) {
        return;
    }

    Borticle *targ = (Borticle*) node->data;
    if (targ && bort->id == targ->id) {
        return;
    }

    // calculate distance
    vec2 sub = (vec2) {bort->pos.x - node->com.x, bort->pos.y - node->com.y};
    float radius = sqrtf(sub.x * sub.x + sub.y * sub.y);

    // leaf nodes: direct comparsion (node->mass == bort->size);
    if (targ) {
        force = _calculate_gravitational_force(bort->size, targ->size, radius, grav_g);
        delta->x += (bort->pos.x < targ->pos.x) ? force : -force;
        delta->y += (bort->pos.y < targ->pos.y) ? force : -force;
       return;
    }

    // check if we can use the node mass for far away regions
    float height = node->self_se.y - node->self_nw.y;
    float res = (radius == 0.f) ? 0.f : (height / radius); // using node height

    // far away nodes: use center of mass and skip child nodes
    if (res < theta) {
        force = _calculate_gravitational_force(bort->size, node->mass, radius, grav_g) ;
        delta->x += (bort->pos.x < node->com.x) ? force : -force;
        delta->y += (bort->pos.y < node->com.y) ? force : -force;
        return;
    }

    // nearby nodes: traverse into child nodes
    _calculate_force(bort, node->ne, delta, theta, grav_g, count);
    _calculate_force(bort, node->nw, delta, theta, grav_g, count);
    _calculate_force(bort, node->sw, delta, theta, grav_g, count);
    _calculate_force(bort, node->se, delta, theta, grav_g, count);
}

static void _update_size(State *state, Borticle *bort, size_t index) {}

static void _update_position(State *state, Borticle *bort, size_t index) {
    if(!bort || !state->tree) {
        return;
    }

    int count = 0;
    vec2 delta = {0.f};
    _calculate_force(bort, state->tree->root, &delta, state->bh_theta, state->grav_g, &count);

    bort->pos.x += delta.x;
    bort->pos.y += delta.y;
    // printf("(%d), {%f, %f} => {%f, %f} (%d)\n", bort->id, bort->pos.x, bort->pos.y, delta.x, delta.y, count);
}

static void _update_color(State *state, Borticle *bort, size_t index) {}

void bort_init_barnes_hut(State *state, Borticle *bort, size_t index) {
    // state is required and wont be tested here
    if (!bort) {
        return;
    }

    bort->color = (rgba) {
        rand_range_f(0.f, 1.f),
        rand_range_f(0.f, 1.f),
        rand_range_f(0.f, 1.f),
        1.f
    };

    bort->pos = (vec3_t) {
        rand_range_f(0.f, (float)state->width),
        rand_range_f(0.f, (float)state->height),
        0.f
    };

    bort->vel = (vec3_t) {
        rand_range_f(-1.f, 1.f),
        rand_range_f(-1.f, 1.f),
        0.f
    };

    bort->acc = (vec3_t) {
        rand_range_f(0.1f, 1.f),
        rand_range_f(0.1f, 1.f),
        0.f
    };

    bort->size = rand_range_f(0.1f, 6.f);
}

void bort_update_barnes_hut(State *state, Borticle *bort, size_t index) {
    // sstate is required and wont be tested here
    if(!bort) {
        return;
    }
    _update_size(state, bort, index);
    _update_position(state, bort, index);
    _update_color(state, bort, index);
}
