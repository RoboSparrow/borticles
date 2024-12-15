// https://learnopengl.com/Advanced-OpenGL/Instancing
// https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/

// “Instancing” means that we have a base mesh (in our case, a simple quad of 2 triangles), but many instances of this quad.

////
// clear && make clean && make && ./bin/borticles
////


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "utils.h"
#include "borticle.h"
#include "quadtree.h"

#define MATH_3D_IMPLEMENTATION
#include "external/math_3d.h"

// settings
const unsigned int WORLD_WIDTH = 800;
const unsigned int WORLD_HEIGHT = 600;

double then;

#define POP_MAX 1000
#define FPS 12.0

// viewport
int width, height;

// Is called whenever a key is pressed/released via GLFW
static void _key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
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

int main() {
    time_t seed = time(NULL);
    srand(seed); // set random seed

    glfwSetErrorCallback(_error_callback);

    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // window
    GLFWwindow* window = glfwCreateWindow(WORLD_WIDTH, WORLD_HEIGHT, "Borticles", NULL, NULL);
    glfwMakeContextCurrent(window);

    //callbacks
    glfwSetKeyCallback(window, _key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD");
        glfwTerminate();
        return -1;
    }

    // viewport
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, _framebuffer_size_callback);

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
    bort_init_shaders_data(&bort, POP_MAX);

    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // data
    vec4 positions[POP_MAX];
    rgba colors[POP_MAX];

    Borticle pop[POP_MAX];
    float hw = (float) width / 2;
    float hh = (float) height / 2;

    for (size_t i = 0; i < POP_MAX; i++) {
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

    // fps calc
    double now, delta;
    double max = 1.0 / FPS;
    then = glfwGetTime();

    // Game loop
    while (!glfwWindowShouldClose(window)) {
        // init
        now = glfwGetTime();
        if (now - then < max) {
            continue;
        }
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // update
        bort.vp_width = width;
        bort.vp_height = height;
        QNode *tree = qnode_create((rect){0.f, 0.f, (float)width, (float)height});
        bort_update(&bort, tree, pop, positions, colors, POP_MAX);

        // draw
        bort_draw_2D(&bort, tree, pop, positions, colors, POP_MAX);

        // finalize
        then = now;
        qnode_destroy(tree);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glDeleteVertexArrays(1, bort.vao);
    glDeleteBuffers(BUF_NUM, bort.vbo);
    glfwTerminate();

    return 0;
}
