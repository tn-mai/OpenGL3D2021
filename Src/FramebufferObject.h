/**
* @file FramebufferObject.h
*/
#ifndef FRAMEBUFFEROBJECT_H_INCLUDED
#define FRAMEBUFFEROBJECT_H_INCLUDED
#include "glad/glad.h"
#include "Texture.h"
#include <memory>

/**
* FBOの種類.
*/
enum class FboType {
  Color, // カラーバッファのみ.
  ColorDepthStencil // カラー、深度、ステンシルバッファ.
};

/**
* フレームバッファオブジェクト.
*/
class FramebufferObject
{
public:
  FramebufferObject(int w, int h, FboType type);
  ~FramebufferObject();
  FramebufferObject(const FramebufferObject&) = delete;
  FramebufferObject& operator=(const FramebufferObject&) = delete;

  explicit operator bool() const { return fbo; }

  void Bind() const;
  void Unbind() const;
  void BindColorTexture(GLuint) const;
  void UnbindColorTexture() const;
  void BindDepthStencilTexture(GLuint) const;
  void UnbindDepthStencilTexture() const;

  // フレームバッファオブジェクトIDを取得する.
  GLuint GetId() const { return fbo; }

private:
  GLuint fbo = 0;
  std::shared_ptr<Texture::Image2D> texColor;
  std::shared_ptr<Texture::Image2D> texDepthStencil;
  GLuint depthStencil = 0;
  int width = 0;
  int height = 0;
};

#endif // FRAMEBUFFEROBJECT_H_INCLUDED
