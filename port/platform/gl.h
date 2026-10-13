// The OpenGL 2.1 subset the renderer uses, loaded at runtime through the platform layer
// (SDL_GL_GetProcAddress), so the build needs neither GL headers nor libGL.
#pragma once
#include <stddef.h>

typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLboolean;
typedef unsigned char GLubyte;
typedef float GLfloat;
typedef unsigned int GLbitfield;
typedef char GLchar;

#define GL_FALSE 0
#define GL_TRUE 1
#define GL_UNSIGNED_BYTE 0x1401
#define GL_FLOAT 0x1406
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_FAN 0x0006
#define GL_BLEND 0x0BE2
#define GL_DEPTH_TEST 0x0B71
#define GL_CULL_FACE 0x0B44
#define GL_SCISSOR_TEST 0x0C11
#define GL_ONE 1
#define GL_ZERO 0
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE0 0x84C0
#define GL_RGBA 0x1908
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_REPEAT 0x2901
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_VERSION 0x1F02
#define GL_RENDERER 0x1F01

// X(return type, name, parameter list)
#define PORT_GL_FUNCTIONS(X)                                                                         \
    X(void, glViewport, (GLint x, GLint y, GLsizei w, GLsizei h))                                    \
    X(void, glScissor, (GLint x, GLint y, GLsizei w, GLsizei h))                                     \
    X(void, glEnable, (GLenum cap))                                                                  \
    X(void, glDisable, (GLenum cap))                                                                 \
    X(void, glClearColor, (GLfloat r, GLfloat g, GLfloat b, GLfloat a))                              \
    X(void, glClear, (GLbitfield mask))                                                              \
    X(void, glBlendFunc, (GLenum src, GLenum dst))                                                   \
    X(void, glGenTextures, (GLsizei n, GLuint * textures))                                           \
    X(void, glDeleteTextures, (GLsizei n, const GLuint* textures))                                   \
    X(void, glBindTexture, (GLenum target, GLuint texture))                                          \
    X(void, glActiveTexture, (GLenum texture))                                                       \
    X(void, glTexImage2D, (GLenum target, GLint level, GLint internal, GLsizei w, GLsizei h, GLint border, \
                           GLenum format, GLenum type, const void* pixels))                          \
    X(void, glTexParameteri, (GLenum target, GLenum name, GLint param))                              \
    X(void, glPixelStorei, (GLenum name, GLint param))                                               \
    X(GLuint, glCreateShader, (GLenum type))                                                         \
    X(void, glShaderSource, (GLuint shader, GLsizei count, const GLchar* const* src, const GLint* len)) \
    X(void, glCompileShader, (GLuint shader))                                                        \
    X(void, glGetShaderiv, (GLuint shader, GLenum name, GLint * params))                             \
    X(void, glGetShaderInfoLog, (GLuint shader, GLsizei max, GLsizei * len, GLchar * log))           \
    X(GLuint, glCreateProgram, (void))                                                               \
    X(void, glAttachShader, (GLuint program, GLuint shader))                                         \
    X(void, glBindAttribLocation, (GLuint program, GLuint index, const GLchar* name))                \
    X(void, glLinkProgram, (GLuint program))                                                         \
    X(void, glGetProgramiv, (GLuint program, GLenum name, GLint * params))                           \
    X(void, glGetProgramInfoLog, (GLuint program, GLsizei max, GLsizei * len, GLchar * log))         \
    X(void, glUseProgram, (GLuint program))                                                          \
    X(GLint, glGetUniformLocation, (GLuint program, const GLchar* name))                             \
    X(void, glUniform1i, (GLint location, GLint v))                                                  \
    X(void, glUniform2f, (GLint location, GLfloat a, GLfloat b))                                     \
    X(void, glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized,     \
                                    GLsizei stride, const void* pointer))                            \
    X(void, glEnableVertexAttribArray, (GLuint index))                                               \
    X(void, glDrawArrays, (GLenum mode, GLint first, GLsizei count))                                 \
    X(const GLubyte*, glGetString, (GLenum name))                                                    \
    X(void, glReadPixels, (GLint x, GLint y, GLsizei w, GLsizei h, GLenum format, GLenum type, void* data))

#define PORT_GL_DECLARE(ret, name, params) \
    typedef ret(*PFN_##name) params;      \
    extern PFN_##name name;
PORT_GL_FUNCTIONS(PORT_GL_DECLARE)
#undef PORT_GL_DECLARE

// Loads every function above; false (with the missing name printed) if one is unavailable
bool port_gl_load();
