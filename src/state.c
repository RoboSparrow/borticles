#include <stdlib.h>

#include "state.h"
#include "borticle.h"
#include "quadtree.h"
#include "vec.h"

#include "utils.h"
#include "log.h"

State *state_create() {
    State *state = malloc(sizeof(State));
    EXIT_IF(state == NULL, "failed to allocate for State");

    state->width = WORLD_WIDTH;
    state->height = WORLD_HEIGHT;

    state->fps = 32;
    state->paused = 0;

    state->pop_max = POP_MAX;
    state->pop_len = 0;
    return state;
}

void state_set_len(State *state, unsigned int len) {
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
        state->pop_max,
        state->pop_len,
        (state->population) ? "[...]" : "<NULL>",
        (state->positions) ? "[...]" : "<NULL>",
        (state->colors) ? "[...]" : "<NULL>"
    );
}
