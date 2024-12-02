// https://learnopengl.com/Advanced-OpenGL/Instancing
// https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/

// “Instancing” means that we have a base mesh (in our case, a simple quad of 2 triangles), but many instances of this quad.



////
// rm -f ./versuch && clear && gcc ./versuch.c ./glad/src/glad.c -lglfw -lGL -lm -I. -I./glad/include -o ./versuch && ./versuch
////


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "utils.h"

// settings
const unsigned int WORLD_WIDTH = 800;
const unsigned int WORLD_HEIGHT = 600;

#define POP_MAX 100

typedef struct { float x, y, z, w; } vec4;
typedef struct { float r, g, b, a; } rgba;


static void _update_positions(vec4 positions[], size_t len) {

    for (unsigned int i = 0; i < len; i++) {

        float x = 0.02f * i * cos(90*M_PI* i / len);
        float y = 0.02f * i * sin(90*M_PI* i / len);
        positions[i].x = x;
        positions[i].y = y;
        positions[i].z = 0.f;
        positions[i].w = 1.f; // size
        printf("%d %f %f\n", i, positions[1].x, positions[1].y);
    }
    // printf("------------\n");
}

static void _update_colors(rgba colors[], size_t len) {
    for (unsigned int i = 0; i < len; i++) {
        colors[i].r = 1.f;
        colors[i].g = 0.f;
        colors[i].b = 0.f;
        colors[i].a = 1.f;
    }
}

static unsigned int _ogl_compile_shader(const char *path, int type) {
    char *src = load_file_alloc(path);
    if (!src) {
        LOG_ERROR_F("Loading shader file failed: '%s'\n", path);
        return 0;
    }

    unsigned int shader = glCreateShader(type);
    const char *src_ = (const char*) src;
    glShaderSource(shader, 1, &src_, NULL);
    glCompileShader(shader);

    int success;
    char msg[512];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        freez(src);
        glGetShaderInfoLog(shader, 512, NULL, msg);
        LOG_ERROR_F("Creating shader '%s' failed: '%s'\n", path, msg);
        return 0;
    }

    freez(src);
    return shader;
}

// TPDP glDetachShader
static unsigned int _ogl_shader_program(unsigned int vertexShader, unsigned int fragmentShader, unsigned int geometryShader) {
    unsigned int program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    if (geometryShader) {
       glAttachShader(program, geometryShader);
    }

    int success;
    char msg[512];

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success){
        glGetShaderInfoLog(program, 512, NULL, msg);
        LOG_ERROR_F("Compiling and linking of shader program failed: '%s'\n", msg);
        return 0;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryShader) {
       glDeleteShader(geometryShader);
    }

    return program;
}

enum {
    BUF_VERTEX,
    BUF_POSITIONS,
    BUF_COLORS,
    BUF_NUM,
} buffers;

static void _shader_vertexes(int *vao, int *vbo) {
    GLfloat vertices[] = {
        // x    y     z
        -0.2f, -0.2f, 0.0f,
         0.2f, -0.2f, 0.0f,
         0.0f, 0.2f,  0.0f
    };

    glGenVertexArrays(1, vao);
    glGenBuffers(BUF_NUM, vbo);

    // 1. bind the vao
    glBindVertexArray(*vao);

    // 2. bind and set vertex buffer(s)
    glBindBuffer(GL_ARRAY_BUFFER, vbo[BUF_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[BUF_POSITIONS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * POP_MAX, NULL, GL_STREAM_DRAW);    //NULL (empty) buffer

    glBindBuffer(GL_ARRAY_BUFFER, vbo[BUF_COLORS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rgba) * POP_MAX, NULL, GL_STREAM_DRAW);    //NULL (empty) buffer

    // cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/**
 * prepares drawing to window
 */
static void _ogl_pre_draw_2D(unsigned int program) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
}

/**
 * prepares drawing to window
 */
static void _ogl_draw_2D(unsigned int program, GLuint *vao, GLuint *vbo, vec4 positions[], rgba colors[], size_t len) {
    glBindVertexArray(*vao);

    glVertexAttribDivisor(0, 0); // vertex
    glVertexAttribDivisor(1, 1); // positions
    glVertexAttribDivisor(2, 1); // colors

    // vertex
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[BUF_VERTEX]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0); // 3 points, float data, no rgba

    // positions
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[BUF_POSITIONS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * len, &positions[0], GL_STREAM_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // colors
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER,  vbo[BUF_COLORS]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rgba) * len, &colors[0], GL_STREAM_DRAW);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);


    //glDrawElementsInstanced(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0, len);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 3, len);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindVertexArray(0);
}

// Is called whenever a key is pressed/released via GLFW
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

/**
 *  glfw: whenever the window size changed (by OS or user resize) this callback function executes
 *      make sure the viewport matches the new window dimensions; note that width and
 *      height will be significantly larger than specified on retina displays.
 */
static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

static void error_callback(int err, const char* message) {
    LOG_ERROR_F("GLFW Error (%d): %s", err, message);
}

int main() {
    GLuint VAO;
    GLuint VBO[BUF_NUM];

    glfwSetErrorCallback(error_callback);

    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // window
    GLFWwindow* window = glfwCreateWindow(WORLD_WIDTH, WORLD_HEIGHT, "LearnOpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    //callbacks
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD");
        glfwTerminate();
        return -1;
    }

    // viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // shaders
    GLuint vert_sh = _ogl_compile_shader("shaders/borticle.vert", GL_VERTEX_SHADER);
    GLuint frag_sh = _ogl_compile_shader("shaders/borticle.frag", GL_FRAGMENT_SHADER);
    GLuint program = _ogl_shader_program(vert_sh, frag_sh, 0);
    _shader_vertexes(&VAO, VBO);

    // data
    vec4 positions[POP_MAX];
    rgba colors[POP_MAX];

    // Game loop
    while (!glfwWindowShouldClose(window)) {

        _update_positions(positions, POP_MAX);
        _update_colors(colors, POP_MAX);

        _ogl_pre_draw_2D(program);
        _ogl_draw_2D(program, &VAO, VBO, positions, colors, POP_MAX);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(BUF_NUM, VBO);
    glfwTerminate();

    return 0;
}
