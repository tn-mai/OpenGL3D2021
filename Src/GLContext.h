/**
* @file GLContext.h
**/
#ifndef GLCONTEXT_H_INCLUDED
#define GLCONTEXT_H_INCLUDED
#include <glad/glad.h>
#include <string>
#include <memory>

/// 三次元座標型.
struct Position
{
  float x, y, z;
};

/// RGBAカラー型.
struct Color
{
  float r, g, b, a;
};

namespace GLContext {

GLuint CreateBuffer(GLsizeiptr size, const GLvoid* data);
GLuint CreateVertexArray(GLuint vboPosition, GLuint vboColor, GLuint vboTexcoord, GLuint ibo);
GLuint CreateProgram(GLenum type, const GLchar* code);
GLuint CreatePipeline(GLuint vp, GLuint fp);
GLuint CreateImage2D(GLsizei width, GLsizei height, const void* data, GLenum format, GLenum type);
GLuint CreateImage2D(const char* filename);
GLuint CreateSampler();

} // namespace GLContext

#endif // GLCONTEXT_H_INCLUDED

