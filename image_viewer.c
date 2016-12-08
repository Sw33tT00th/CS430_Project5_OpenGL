#include <stdlib.h>
#include <stdio.h>
#include <OpenGL/gl.h>
#include <GLFW/glfw3.h>

#include "linmath.h"

#include "ppm/header.h"
#include "ppm/p6.h"

#include <assert.h>

#define PI 3.14159265359

typedef struct {
  float Position[2];
  float TexCoord[2];
} Vertex;

// (-1, 1)  (1, 1)
// (-1, -1) (1, -1)

Vertex vertexes[] = {
  {{-1, 1}, {0, 0}},
  {{1, 1}, {0.99999, 0}},

  {{-1, -1}, {0, 0.99999}},
  {{1, 1}, {0.99999, 0}},

  {{1, -1}, {0.99999, 0.99999}},
  {{-1, -1}, {0, 0.99999}}
};

float x_coord             = 0.0;
float y_coord             = 0.0;
float scale_value         = 1.0;
float scale_increment     = 1.0;
float translate_x         = 0.0;
float translate_y         = 0.0;
float translate_increment = 2.0;
float rotation_value      = 0.0;
float rotation_increment  = 3.0;
float shear_value         = 0.0;
float shear_increment     = 2.0;

static const char* vertex_shader_text =
"uniform mat4 MVP;\n"
"attribute vec2 TexCoordIn;\n"
"attribute vec2 vPos;\n"
"varying vec2 TexCoordOut;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    TexCoordOut = TexCoordIn;\n"
"}\n";

static const char* fragment_shader_text =
"varying vec2 TexCoordOut;\n"
"uniform sampler2D Texture;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    // MORE KEYBOARD INPUTS
    // Up key for tanslate up
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        translate_y += translate_increment;

    // Down key for translate down
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        translate_y -= translate_increment;

    // Right key for translate right
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        translate_x += translate_increment;

    // Left key for translate left
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
        translate_x -= translate_increment;

    // W key for scale up
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
        scale_value += scale_increment;

    // S key for scale down
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
        scale_value -= scale_increment;

    // A key for rotate counter-clockwise
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
        rotation_value += PI/rotation_increment;

    // D key for rotate clockwise
    if (key == GLFW_KEY_D && action == GLFW_PRESS)
        rotation_value -= PI/rotation_increment;

    // E key for shear right
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
        shear_value += shear_increment;

    // Q key for shear left
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        shear_value -= shear_increment;
}

static inline void matrix_shear(mat4x4 input, float x, float y, mat4x4 output) {
    vec4 xy = {{x}, {y}, {1}, {1}};

    mat4x4 shear_mat = {
        {1.f, y,    1.f,  0.f},
        {x,   1.f,  1.f,  0.f},
        {1.f, 1.f,  1.f,  0.f},
        {0.f, 0.f,  0.f,  1.f}
    };

    mat4x4_mul(output, input, shear_mat);
}

void glCompileShaderOrDie(GLuint shader) {
  GLint compiled;
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
    char* info = malloc(infoLen+1);
    GLint done;
    glGetShaderInfoLog(shader, infoLen, &done, info);
    printf("Unable to compile shader: %s\n", info);
    exit(1);
  }
}

int main(int argc, char *argv[])
{
  char* file_name = malloc(sizeof(char) * 80);
  if(argc == 2) {
    file_name = argv[1];
  }
  else {
    file_name = "out.ppm";
  }
  
  FILE *image_file = fopen(file_name, "r");
  Header image_header;
  if (read_header(image_file, &image_header)) { exit(1); }


  int image_size = image_header.height * image_header.width;
  Pixel *body_content = malloc(sizeof(Pixel) * image_size);

  if (read_p6(file_name, image_file, image_header, body_content)) {
    exit(1);
  }

  unsigned char image[image_size * 3];

  Pixel *current_pixel = body_content;
  int i;
  int j = 0;

  for(i = 0; i < image_size * 3; i += 3) {
    image[i] = (char) (current_pixel[j].r * 255);
    image[i + 1] = (char) (current_pixel[j].g * 255);
    image[i + 2] = (char) (current_pixel[j].b * 255);
    j++;
  }
  
    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    // gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShaderOrDie(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShaderOrDie(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    // more error checking! glLinkProgramOrDie!

    mvp_location = glGetUniformLocation(program, "MVP");
    assert(mvp_location != -1);

    vpos_location = glGetAttribLocation(program, "vPos");
    assert(vpos_location != -1);

    GLint texcoord_location = glGetAttribLocation(program, "TexCoordIn");
    assert(texcoord_location != -1);

    GLint tex_location = glGetUniformLocation(program, "Texture");
    assert(tex_location != -1);

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) 0);

    glEnableVertexAttribArray(texcoord_location);
    glVertexAttribPointer(texcoord_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (sizeof(float) * 2));
    
    int image_width = image_header.width;
    int image_height = image_header.height;

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(tex_location, 0);

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 m, p, mvp, translate_matrix, rotate_matrix, scale_matrix, shear_matrix;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4_identity(m);
        mat4x4_identity(translate_matrix);
        mat4x4_identity(rotate_matrix);
        mat4x4_identity(scale_matrix);
        mat4x4_identity(shear_matrix);

        // scale
        mat4x4_scale_aniso(scale_matrix, scale_matrix, scale_value, scale_value, scale_value);
        mat4x4_add(m, scale_matrix, m);

        //shear
        matrix_shear(shear_matrix, shear_value, shear_value, shear_matrix);
        mat4x4_add(m, shear_matrix, m);

        // translate
        mat4x4_translate(translate_matrix, translate_x, translate_y, 0);
        mat4x4_add(m, translate_matrix, m);

        // rotate
        mat4x4_rotate_Z(rotate_matrix, rotate_matrix, rotation_value);
        mat4x4_add(m, rotate_matrix, m);

        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]
