#include <math.h>
#include <stdlib.h>

#include "external/math_3d.h"

#include "shader.h"
#include "borticle.h"
#include "state.h"

static void _update_size(ShaderInfo *shader, State *state, Borticle *bort, size_t index) {}

static void _update_position(ShaderInfo *shader, State *state, Borticle *bort, size_t index) {
    float dirx = (bort->vel.x > 0) ? 1 : -1;
    float diry = (bort->vel.y > 0) ? 1 : -1;

    if (bort->pos.x < 0 || bort->pos.x > state->width) {
        dirx = -dirx;
    }
    if (bort->pos.y < 0 || bort->pos.y > state->height) {
        diry = -diry;
    }

    bort->vel.x = (dirx * bort->acc.x);
    bort->vel.y = (diry * bort->acc.y);

    bort->pos.x += bort->vel.x;
    bort->pos.y += bort->vel.y;
}

static void _update_color(ShaderInfo *shader, State *state, Borticle *bort, size_t index) {}

float _magnitude(Borticle *src, Borticle *targ) {
    if (!src || !targ) {
        return 0.f;
    }

    if (src->id == targ->id) {
        return 0.f;
    }

    vec3_t sub = v3_sub(src->pos, targ->pos);
    return v3_length(sub);
}

static void _apply_attraction(Borticle *bort, QNode *region, float max_dist, State *state) {
    if (!region) {
        return;
    }

    float mag, force, delta;
    vec3_t pos;
    Borticle *targ;

    for (unsigned int i = 0; i < region->sz; i++) {
        targ = state_get_borticle(state, region->members[i].id);
        mag = _magnitude(bort, targ);

        if (fabsf(mag) > max_dist) {
            continue;
        }

        // F = (G * mass1 * mass2) / radius^2
        force = (state->grav_g * bort->size * targ->size) / (mag * mag);
        delta = force / bort->size;

        pos = vec3(bort->pos.x, bort->pos.y, bort->pos.z);
        pos = v3_divs(pos, mag); // normalize
        pos = v3_muls(pos, delta);

        if (bort->pos.x < 0.f || bort->pos.x > state->width) {
            pos.x = -pos.x;
        }
        if (bort->pos.y < 0.f || bort->pos.y > state->height) {
            pos.y = -pos.y;
        }

        bort->pos = v3_add(bort->pos, pos);
        // printf(" -- force: %.2f, delta %.2f - src (%d) {%.2f,%.2f} <=> targ (%d) {%.2f,%.2f}\n", force, delta, bort->id, bort->pos.x, bort->pos.y, targ->id, targ->pos.x, targ->pos.y);
    }

    if(region->nw != NULL) _apply_attraction(bort, region->nw, max_dist, state);
    if(region->ne != NULL) _apply_attraction(bort, region->ne, max_dist, state);
    if(region->sw != NULL) _apply_attraction(bort, region->sw, max_dist, state);
    if(region->se != NULL) _apply_attraction(bort, region->se, max_dist, state);
}

void bort_init_attraction(ShaderInfo *shader, State *state, Borticle *bort, size_t index) {
    // shader and state are required and wont be tested here
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

void bort_update_attraction(ShaderInfo *shader, State *state, Borticle *bort, size_t index) {
    if(!bort) {
        return;
    }

    _update_size(shader, state, bort, index);
    //_update_position(shader, state, bort, index);
    _update_color(shader, state, bort, index);

    float max_dist = 200.f; // dev TODO
    float r = max_dist/2;

    rect area;
    area.x = bort->pos.x - r;
    area.y = bort->pos.y - r;
    area.width = max_dist;
    area.height = max_dist;

    // find nearby
    QNode *region = qnode_contains(state->tree, area);
    _apply_attraction(bort, region, max_dist, state);
}
