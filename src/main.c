////
// clear && make clean && make && ./bin/borticles
////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // getopt

#include <math.h>
#include <time.h>

#include "raylib.h"
#include "rlgl.h"

#include "quadtree/qnode.h"

#include "utils.h"
#include "log.h"

#include "state.h"
#include "borticle.h"
#include "algorithms.h"

#define MATH_3D_IMPLEMENTATION
#include "external/math_3d.h"

static void _configure(State *state, int argc, char **argv) {

    int opt, ival;
    float fval;
    unsigned int pop_len = POP_MAX;

    char usage[] = "usage: %s [-h] [-p particles:number] [-f fps] [-g fgravity constant] [-P paused]\n";
    while ((opt = getopt(argc, argv, "f:g:p:Ph")) != -1) {
        switch (opt) {
        case 'p':
            ival = atoi(optarg);
            if (!ival || ival < 0) {
                fprintf(stderr, "invalid '%c' option value\n", opt);
                exit(1);
            }

            pop_len = ival;
            break;

        case 'f':
            ival = atoi(optarg);
            if (!ival || ival < 0) {
                fprintf(stderr, "invalid 'f' option value\n");
                exit(1);
            }

            state->fps = ival;
            break;

        case 'g':
            fval = atof(optarg);
            if (!fval || fval < 0.f) {
                fprintf(stderr, "invalid 'g' option value\n");
                exit(1);
            }

            state->grav_g = fval;
            break;

        case 'P':
            state->paused = 1;
            break;

        case 'h':

        case '?':
            fprintf(stderr, usage, argv[0]);
            exit(0);
            break;
        }
    }

    state_set_pop_len(state, pop_len);
    state->algorithms = ALGO_NONE;

    // dev algorithm TODO param & default
    state->algorithms |= ALGO_ATTRACTION;

    // state_print(stdout, state);
}

int main(int argc, char **argv) {
    // set random seed
    srand(time(NULL));

    State *state = state_create();
    _configure(state, argc, argv);

    // window
    InitWindow(state->width, state->height, "Borticles");

    // we will render point sizes
    glEnable(GL_PROGRAM_POINT_SIZE);

    // matrices
    mat4_t model = m4_identity();
    mat4_t view = m4_identity();
    mat4_t projection = m4_ortho(0.f, (float) state->width, (float) state->height, 0.f, 0.f, 1.f);

    // borticle shaders
    ShaderInfo bort = {0};
    bort_init_shaders(&bort);
    bort_init_matrices(&bort, model.m, view.m, projection.m);
    bort_init_shaders_data(&bort, state);

    // qtree shaders
    ShaderInfo qt = {0};
    qtree_init_shaders(&qt);
    bort_init_matrices(&qt, model.m, view.m, projection.m);// TODO make common funcname name

    // init population
    bort_init(&bort, state);

    // fps calc
    SetTargetFPS(state->fps);

    state_print(stdout, state);

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_SPACE)) {
            state->paused = !state->paused;
        }

        if (state->paused) {;
            continue;
        }

        BeginDrawing();

        ClearBackground(state->bg_color);

        // update
        state->tree = qnode_create((rect){0.f, 0.f, (float)state->width, (float)state->height});
        bort_update(&bort, state);
        // state_print(stdout, state);

        // draw
        bort_draw_2D(&bort, state);
        qtree_draw_2D(&qt, state);

        // finalize
        qnode_destroy(state->tree);
        state->tree = NULL;

        //debug render
        DrawFPS(10, 10);

        EndDrawing();
    }

    // cleanup

    bort_cleanup_shaders(&bort);
    qtree_cleanup_shaders(&qt);
    state_destroy(state);

    CloseWindow();

    return 0;
}
