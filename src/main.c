////
// clear && make clean && make && ./bin/borticles
// clear && make clean && make && make test && ./bin/test
////

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h> // getopt

#include <math.h>
#include <time.h>

#include "raylib.h"
#include "rlgl.h"

#include "qtree/qtree.h"

#include "log.h"
#include "utils.h"

#include "state.h"
#include "borticle.h"

#include "ui.h"

#define MATH_3D_IMPLEMENTATION
#include "external/math_3d.h"

#define RAYGUI_IMPLEMENTATION
#include "external/raygui.h"

static void _configure(State *state, int argc, char **argv) {

    int opt, ival;
    float fval;
    unsigned int pop_len = POP_MAX;

    // default
    state->algorithms |= ALGO_BARNES_HUT;
    // state->algorithms |= ALGO_NOMADIC;
    // state->algorithms = ALGO_NONE;

    char usage[] = "usage: %s [-h] [-f fps] [-g gravity constant] [-p particles:number] [-a algorithms <int,int, ...>] [-P paused]\n";
    while ((opt = getopt(argc, argv, "f:g:p:a:Ph")) != -1) {
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
                if (!fval < 0.f) {
                    fprintf(stderr, "invalid 'g' option value\n");
                    exit(1);
                }

                state->grav_g = fval;
            break;

            case 'a': {
                state->algorithms = 0; // reset
                ival = -1;
                char *pt;
                pt = strtok (optarg, ",");
                while (pt != NULL) {
                    int ival = atoi(pt);

                    if (ival < 0 || ival >= ALGO_LEN) {
                        fprintf(stderr, "invalid 'a' option value\n");
                        exit(1);
                    }
                    state->algorithms |= (1 << ival);
                    pt = strtok (NULL, ",");
                }
            }
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
    // state_print(stdout, state);
}

int main(int argc, char **argv) {
    // set random seed
    srand(time(NULL));

    State *state = state_create();
    _configure(state, argc, argv);

    // window
    InitWindow(state->width, state->height, "Borticles");
    ui_init(state);

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
    bort_init(state, 0, state->pop_len);

    // fps calc
    SetTargetFPS(state->fps);
    state_print(stdout, state);

    Vector2 mpos = {0.f};

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_SPACE)) {
            state->paused = !state->paused;
        }

        if (state->paused) {

            BeginDrawing();
            ui_draw(state);
            EndDrawing();

            continue;
        }

        BeginDrawing();

        ClearBackground(state->bg_color);

        // update
        state->tree = qtree_create((vec2){0.f, 0.f}, (vec2){(float) state->width, (float) state->height});
        bort_update(state);
        ui_update(state);

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)){
            mpos = GetMousePosition();
            if (state->selected) {
                state->selected = NULL;
            } else {
                QNode *nearest = qtree_find_nearest(state->tree, (vec2) {mpos.x, mpos.y});

                if (nearest) {
                    state->selected = (Borticle*) nearest->data;
                }
            }
        }

        // state_print(stdout, state);
        // qtree_print(stdout, state->tree);exit(1);

        // draw
        bort_draw_2D(&bort, state);
        qtree_draw_2D(&qt, state);
        ui_draw(state);

        // finalize
        qtree_destroy(state->tree);
        state->tree = NULL;

        EndDrawing();
    }

    // cleanup

    bort_cleanup_shaders(&bort);
    qtree_cleanup_shaders(&qt);
    state_destroy(state);

    CloseWindow();        // Close window and OpenGL context

    return 0;
}
