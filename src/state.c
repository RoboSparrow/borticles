#include <stdlib.h>

#include "quadtree/qnode.h"

#include "vec.h"
#include "state.h"
#include "borticle.h"

#include "utils.h"
#include "log.h"

State *state_create() {
    State *state = malloc(sizeof(State));
    EXIT_IF(state == NULL, "failed to allocate for State");

    state->width = WORLD_WIDTH;
    state->height = WORLD_HEIGHT;

    state->fps = 32;
    state->paused = 0;

    state->bg_color = (Color) {51, 77, 77, 255};
    state->fg_color = (Color) {255, 255, 255, 255};

    state->grav_g = 9.81f;

    state->pop_max = POP_MAX;
    state->pop_len = 0;

    state->population = NULL;
    state->tree = NULL;

    state->positions = NULL;
    state->colors = NULL;

    return state;
}

void state_set_pop_len(State *state, unsigned int len) {
    if (!state) {
        return;
    };

    if (len == 0) {
        freez(state->population);
        state->population = NULL;
        freez(state->positions);
        state->positions = NULL;
        freez(state->colors);
        state->colors = NULL;
    }

    if (len > state->pop_max) {
        LOG_ERROR_F("State: length '%d' exceeds map pop_max, value capped to '%d'", len, state->pop_max);
        len = state->pop_max;
    }
    state->pop_len = len;

    state->population = realloc(state->population, len * sizeof(Borticle));
    EXIT_IF(state->population == NULL, "failed to (re)allocate for State->population");

    state->positions = realloc(state->positions, len * sizeof(vec4));
    EXIT_IF(state->positions == NULL, "failed to (re)allocate for State->positions");

    state->colors = realloc(state->colors, len * sizeof(rgba));
    EXIT_IF(state->colors == NULL, "failed to (re)allocate for State->colors");

    // also destroy the actual qtree
    qnode_destroy(state->tree);
    state->tree= NULL;

}

void state_destroy(State *state) {
    if (!state) {
        return;
    };

    freez(state->population);
    freez(state->positions);
    freez(state->colors);
    qnode_destroy(state->tree);

    freez(state);
}

void state_print(FILE *fp, State *state) {
    if(!fp) {
        return;
    }

    if (!state) {
        fprintf(fp, "<NULL>\n");
    };

    fprintf(fp,
        "{\n"
        "  width: %d\n"
        "  height: %d\n"
        "  fps: %d\n"
        "  paused: %d\n"
        "  fg_color: {%d,%d,%d,%d}\n"
        "  bg_color: {%d,%d,%d,%d}\n"
        "  algorithms: %d\n"
        "  grav_g: %.2f\n"
        "  pop_max: %d\n"
        "  pop_len: %d\n"
        "  population: %s\n"
        "  positions: %s\n"
        "  colors: %s\n"
        "}\n",

        state->width,
        state->height,
        state->fps,
        state->paused,
        state->fg_color.r, state->fg_color.g, state->fg_color.b, state->fg_color.a,
        state->bg_color.r, state->bg_color.g, state->bg_color.b, state->bg_color.a,
        state->algorithms,
        state->grav_g,
        state->pop_max,
        state->pop_len,
        (state->population) ? "[...]" : "<NULL>",
        (state->positions) ? "[...]" : "<NULL>",
        (state->colors) ? "[...]" : "<NULL>"
    );
}

Borticle *state_get_borticle(State *state, int index) {
    if (!state || !state->population) {
        return NULL;
    }

    if (index >= 0 && index < state->pop_len) {
        return &state->population[index];
    }

    return NULL;
}
