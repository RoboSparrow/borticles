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

void bort_init_nomadic(ShaderInfo *shader, State *state, Borticle *bort, size_t index) {
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

void bort_update_nomadic(ShaderInfo *shader, State *state, Borticle *bort, size_t index) {
    if(!bort) {
        return;
    }
    _update_size(shader, state, bort, index);
    _update_position(shader, state, bort, index);
    _update_color(shader, state, bort, index);
}
