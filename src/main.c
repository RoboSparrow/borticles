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

#define MATH_3D_IMPLEMENTATION
#include "external/math_3d.h"

// settings
const unsigned int WORLD_WIDTH = 800;
const unsigned int WORLD_HEIGHT = 600;

double then;

#define POP_MAX 1000

// globals
int width, height;
unsigned int pop_len = POP_MAX;
unsigned int fps = 32;
unsigned int paused = 0;

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
static void _framebuffer_size_callback(GLFWwindow* window, int w, int h) {
    width = w;
    height = h;
    glViewport(0, 0, width, height);
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
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            LOG_INFO("Closing app");
            glfwSetWindowShouldClose(window, GL_TRUE);
            break;
        case GLFW_KEY_SPACE:
            LOG_INFO("toggling pause");
            paused = !paused;
            break;
        }
    }
}

static void _configure(int argc, char **argv) {

    int opt;
    int ival;

    char usage[] = "usage: %s [-h] [-p particles:number] [-f fps] [-P paused]\n";
    while ((opt = getopt(argc, argv, "f:p:Ph")) != -1) {
        switch (opt) {
        case 'p':
            ival = atoi(optarg);
            if (!ival || ival < 0) {
                fprintf(stderr, "invalid '%c' option value\n", opt);
                exit(1);
            }
            if (ival > pop_len) {
                fprintf(stderr, "invalid '%c' option value: pop > max (%d > %d)\n", opt, ival, pop_len);
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
            fps = ival;
            break;

        case 'P':
            paused = 1;
            break;

        case 'h':
        case '?':
            fprintf(stderr, usage, argv[0]);
            exit(0);
            break;
        }
    }
}

int main(int argc, char **argv) {

    time_t seed = time(NULL);
    srand(seed); // set random seed

    _configure(argc, argv);
    glfwSetErrorCallback(_error_callback);

    // glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // window
    GLFWwindow* window = glfwCreateWindow(WORLD_WIDTH, WORLD_HEIGHT, "Borticles", NULL, NULL);
    glfwMakeContextCurrent(window);

    // glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD");
        glfwTerminate();
        return -1;
    }

    // viewport
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // glfw events
    glfwSetFramebufferSizeCallback(window, _framebuffer_size_callback);
    glfwSetKeyCallback(window, _gl_key_callback);

    // we will render point sizes
    glEnable(GL_PROGRAM_POINT_SIZE);

    // matrices
    mat4_t model = m4_identity();
    mat4_t view = m4_identity();
    mat4_t projection = m4_ortho(0.f, (float) width, (float) height, 0.f, 0.f, 1.f);

    // borticle shaders
    ShaderState bort = {0};
    bort.vp_width = width;
    bort.vp_height = height;

    bort_init_shaders(&bort);
    bort_init_matrices(&bort, model.m, view.m, projection.m);
    bort_init_shaders_data(&bort, pop_len);

    // borticle data
    vec4 positions[POP_MAX];
    rgba colors[POP_MAX];
    Borticle pop[POP_MAX];

    float hw = (float) width / 2;
    float hh = (float) height / 2;

    for (size_t i = 0; i < pop_len; i++) {
        pop[i].id = i;
        pop[i].pos = (vec3_t) {hw, hh, 0.f};
        pop[i].color = (rgba) {
            rand_range_f(0.f, 1.f),
            rand_range_f(0.f, 1.f),
            rand_range_f(0.f, 1.f),
            1.f
        };
        pop[i].vel = (vec3_t) {
            rand_range_f(-10.f, 10.f),
            rand_range_f(-10.f, 10.f),
            0.f
        };
        pop[i].acc = (vec3_t) {
            rand_range_f(0.1f, 5.f),
            rand_range_f(0.1f, 5.f),
            0.f
        };
        pop[i].size = rand_range_f(0.1f, 6.f);
    }

    // qtree shaders
    ShaderState qt = {0};
    qt.vp_width = width;
    qt.vp_height = height;

    qtree_init_shaders(&qt);
    bort_init_matrices(&qt, model.m, view.m, projection.m);// TODO make common funcname name

    // fps calc
    double now, delta;
    double max = 1.0 / fps;
    then = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        // init
        now = glfwGetTime();
        if (now - then < max) {
            continue;
        }

        if (paused) {
            glfwPollEvents();
            continue;
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // update
        bort.vp_width = width;
        bort.vp_height = height;
        QNode *tree = qnode_create((rect){0.f, 0.f, (float)width, (float)height});
        bort_update(&bort, tree, pop, positions, colors, pop_len);

        // draw
        bort_draw_2D(&bort, tree, pop, positions, colors, pop_len);
        qtree_draw_2D(tree, &qt);

        // finalize
        then = now;
        qnode_destroy(tree);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    bort_cleanup_shaders(&bort);
    qtree_cleanup_shaders(&qt);
    glfwTerminate();

    return 0;
}
