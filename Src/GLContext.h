/**
* @file GLContext.h
**/
#ifndef GLCONTEXT_H_INCLUDED
#define GLCONTEXT_H_INCLUDED
#include <glad/glad.h>
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

/**
* 2Dテクスチャ.
*/
class Image2D
{
public:
  Image2D() = default;
  explicit Image2D(GLuint id);
  ~Image2D();
  Image2D(const Image2D&) = delete;
  Image2D& operator=(const Image2D&) = delete;

  void Bind(GLuint unit) const;
  void Unbind() const;
  GLsizei Width() const;
  GLsizei Height() const;

private:
  GLuint id = 0;
  GLsizei width = 0;
  GLsizei height = 0;
};
using Image2DPtr = std::shared_ptr<Image2D>;
Image2DPtr CreateImage2D(GLsizei width, GLsizei height, const void* data, GLenum format, GLenum type);
Image2DPtr CreateImage2D(const char* filename);
void UnbindAllTextures();

GLuint CreateSampler();

} // namespace GLContext

#endif // GLCONTEXT_H_INCLUDED

