/**
* @file Texture.h
*/
#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include <glad/glad.h>

/**
* 2Dテクスチャ.
*/
class Image2D
{
public:
  Image2D() = default;
  explicit Image2D(const char* filename);
  Image2D(GLsizei width, GLsizei height, const void* data, GLenum format, GLenum type);
  ~Image2D();
  Image2D(const Image2D&) = delete;
  Image2D& operator=(const Image2D&) = delete;

  explicit operator bool() const { return id; }

  void Bind(GLuint unit) const;
  void Unbind() const;
  GLsizei Width() const;
  GLsizei Height() const;

private:
  GLuint id = 0;
  GLsizei width = 0;
  GLsizei height = 0;
};
void UnbindAllTextures();

#endif // TEXTURE_H_INCLUDED
