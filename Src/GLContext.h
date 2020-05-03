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

#if 0
/**
*
*/
class ShaderProgram
{
public:
  ShaderProgram() = default;
  explicit ShaderProgram(GLuint id);
  ~ShaderProgram();
  ShaderProgram(const ShaderProgram&) = delete;
  ShaderProgram& operator=(const ShaderProgram&) = delete;

  void SetMVP(const glm::mat4&);

private:
  GLuint id = 0;
  GLint locMatMVP = -1;
};
using ShaderProgramPtr = std::shared_ptr<ShaderProgram>;

/**
*
*/
class ShaderPipeline
{
public:
  ShaderPipeline() = default;
  explicit ShaderPipeline(GLuint id);
  ~ShaderPipeline();
  ShaderPipeline(const ShaderPipeline&) = delete;
  ShaderPipeline& operator=(const ShaderPipeline&) = delete;

private:
  GLuint id = 0;
  ShaderProgramPtr vs;
  ShaderProgramPtr fs;
};
using ShaderPipelinePtr = std::shared_ptr<ShaderPipeline>;
#endif

} // namespace GLContext

#endif // GLCONTEXT_H_INCLUDED

