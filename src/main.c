////
// clear && make clean && make && ./bin/borticles
////

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // getopt

#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "utils.h"
#include "log.h"

#include "borticle.h"
#include "quadtree.h"
#include "state.h"

#define MATH_3D_IMPLEMENTATION
#include "external/math_3d.h"

// globals
double then;


// Is called whenever a key is pressed/released via GLFW
static void _key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

/**
 *  glfw: whenever the window size changed (by OS or user resize) this callback function executes
 *      make sure the viewport matches the new window dimensions; note that width and
 *      height will be significantly larger than specified on retina displays.
 */
static void _framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    State *state = (State*)glfwGetWindowUserPointer(window);
    state->width = width;
    state->height = height;
    glViewport(0, 0, state->width, state->height);
}

static void _error_callback(int err, const char* message) {
    LOG_ERROR_F("GLFW Error (%d): %s", err, message);
}

/**
 * key input callback
 * @see https://www.glfw.org/docs/latest/input_guide.html#input_key
 * @see https://www.glfw.org/docs/latest/group__keys.html
 */
static void _gl_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {

    State *state = (State*)glfwGetWindowUserPointer(window);

    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            LOG_INFO("Closing app");
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_SPACE:
            LOG_INFO("toggling pause");
            state->paused = !state->paused;
            break;
        }
    }
}

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

    state_set_len(state, pop_len);
    state->algorithms = ALGO_NONE;

    // dev algorithm
    state->algorithms |= ALGO_ATTRACTION;

    // state_print(stdout, state);
}

int main(int argc, char **argv) {

    time_t seed = time(NULL);
    srand(seed); // set random seed

    State *state = state_create();
    _configure(state, argc, argv);

    glfwSetErrorCallback(_error_callback);

    // glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // window
    GLFWwindow* window = glfwCreateWindow(state->width, state->height, "Borticles", NULL, NULL);
    glfwMakeContextCurrent(window);

    // store up for callback updates
    glfwSetWindowUserPointer(window, state);

    // glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD");
        glfwTerminate();
        return -1;
    }

    // viewport
    glfwGetFramebufferSize(window, &state->width, &state->height);
    glViewport(0, 0, state->width, state->height);

    // glfw events
    glfwSetFramebufferSizeCallback(window, _framebuffer_size_callback);
    glfwSetKeyCallback(window, _gl_key_callback);

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
    double now, delta;
    double max = 1.0 / state->fps;
    then = glfwGetTime();

    state_print(stdout, state);

    while (!glfwWindowShouldClose(window)) {
        // init
        now = glfwGetTime();
        if (now - then < max) {
            continue;
        }

        if (state->paused) {
            glfwPollEvents();
            continue;
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // update
        state->tree = qnode_create((rect){0.f, 0.f, (float)state->width, (float)state->height});
        bort_update(&bort, state);
        // state_print(stdout, state);

        // draw
        bort_draw_2D(&bort, state);
        qtree_draw_2D(&qt, state);

        draw_quad(50.f, 50.f, 250.f, 250.f);

        // finalize
        then = now;
        qnode_destroy(state->tree);
        state->tree = NULL;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup

    bort_cleanup_shaders(&bort);
    qtree_cleanup_shaders(&qt);

    state_destroy(state);
    glfwTerminate();

    return 0;
}
