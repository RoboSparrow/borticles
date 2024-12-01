// https://learnopengl.com/Advanced-OpenGL/Instancing
// https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/

// “Instancing” means that we have a base mesh (in our case, a simple quad of 2 triangles), but many instances of this quad.



////
// rm -f ./versuch && clear && gcc ./versuch.c ./glad/src/glad.c -lglfw -lGL -lm -I. -I./glad/include -o ./versuch && ./versuch
////


#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "utils.h"

// settings
const unsigned int WORLD_WIDTH = 800;
const unsigned int WORLD_HEIGHT = 600;

#define POP_MAX 100

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


static void _shader_vertexes(int *vao, int *vbo) {
    GLfloat vertices[] = {
        // x    y     z
        -0.8f, -0.8f, 0.0f,
         0.8f, -0.8f, 0.0f,
         0.0f, 0.8f,  0.0f
    };

    glGenVertexArrays(1, vao);
    glGenBuffers(1, vbo);

    // 1. bind the vao
    glBindVertexArray(*vao);

    // 2. bind and set vertex buffer(s)
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 3. bind and set attribute pointer(s).
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0); // 3 points, float data, no rgba
    glEnableVertexAttribArray(0);

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
static void _ogl_draw_2D(unsigned int program, unsigned int vao, unsigned int vbo) {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3); // We set the count to 6 since we're drawing 6 vertices now (2 triangles); not 3!
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
    GLuint VBO, VAO;

    glfwSetErrorCallback(error_callback);

    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(WORLD_WIDTH, WORLD_HEIGHT, "LearnOpenGL", NULL, NULL);
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG_ERROR("Failed to initialize GLAD");
        glfwTerminate();
        return -1;
    }

    // Define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    GLuint vert_sh = _ogl_compile_shader("shaders/borticle.vert", GL_VERTEX_SHADER);
    GLuint frag_sh = _ogl_compile_shader("shaders/borticle.frag", GL_FRAGMENT_SHADER);
    GLuint program = _ogl_shader_program(vert_sh, frag_sh, 0);

    _shader_vertexes(&VAO, &VBO);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        _ogl_pre_draw_2D(program);
        _ogl_draw_2D(program, VAO, VBO);
        //// Render
        //// Clear the colorbuffer
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);
        //
        //// Draw our first triangle
        //glUseProgram(program);
        //glBindVertexArray(VAO);
        //glDrawArrays(GL_TRIANGLES, 0, 6); // We set the count to 6 since we're drawing 6 vertices now (2 triangles); not 3!
        //glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}
