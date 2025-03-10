#include "borticle.h"
#include "qtree/qtree.h"
#include "state.h"

// Threshold for using coenter of mass approximation vs  direct summation
static const float THETA = 1.f; // TODO to state


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
 * Function MainApp::CalcForce
 *   for all particles
 *     force = RootNode.CalculateForceFromTree(particle)
 *   end for
 * end
 *
 * Function force = TreeNode::CalculateForce(targetParticle)
 *   force = 0
 *
 *   if number of particle equals 1
 *     force = Gravitational force between targetParticle and particle
 *   else
 *     r = distance from nodes center of mass to targetParticle
 *     d = height of the node
 *     if (d/r < θ)
 *       force = Gravitational force between targetParticle and node
 *     else
 *       for all child nodes n
 *         force += n.CalculateForce(particle)
 *       end for
 *     end if
 *   end
 * end
 */
static float _calculate_force(Borticle *bort, QNode *node, float theta, float grav_g, int *count) {
    float force = 0.f;

    if(!bort || !node) {
        return force;
    }

    *count += 1;

    vec2 sub = (vec2) {bort->pos.x - node->com.x, bort->pos.y - node->com.y};
    float radius = sqrtf(sub.x * sub.x + sub.y * sub.y);

    if (qnode_isleaf(node)) {
        if (!node->data) {
            return force;
        }
        // leaf node, direct comparsion (node->mass == bort->size);
        Borticle *targ = (Borticle*) node->data;
        return _calculate_gravitational_force(bort->size, targ->size, radius, grav_g) ;
    }

    float height = node->self_se.y - node->self_nw.y;
    float res = (radius == 0.f) ? 0.f : (height / radius); // using node height

    /// TODO add another filter

    if (res < theta) {
        // sufficiently far away: use center of mass ansd skip child nodes
        return _calculate_gravitational_force(bort->size, node->mass, radius, grav_g) ;
    }

    force += _calculate_force(bort, node->ne, theta, grav_g, count);
    force += _calculate_force(bort, node->nw, theta, grav_g, count);
    force += _calculate_force(bort, node->sw, theta, grav_g, count);
    force += _calculate_force(bort, node->se, theta, grav_g, count);

    return force;
}

static void _update_size(State *state, Borticle *bort, size_t index) {}

static void _update_position(State *state, Borticle *bort, size_t index) {
    if(!bort || !state->tree) {
        return;
    }

    int count = 0;
    float force = _calculate_force(bort, state->tree->root, THETA, state->grav_g, &count);
    printf(" (%d), {%f, %f}\n", bort->id, bort->pos.x, bort->pos.y);
    bort->pos.x += force;
    bort->pos.y += force;

    printf("(%d), {%f, %f} => %f (%d)\n", bort->id, bort->pos.x, bort->pos.y, force, count);
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
