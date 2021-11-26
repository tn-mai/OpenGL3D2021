/**
* @file FramebufferObject.h
*/
#ifndef FRAMEBUFFEROBJECT_H_INCLUDED
#define FRAMEBUFFEROBJECT_H_INCLUDED
#include "glad/glad.h"
#include "Texture.h"
#include <memory>

// 影のゆらぎを抑制するコードを有効化する(テキスト未実装)
#define SUPRESS_SHADOW_JITTERING

/**
* FBOの種類
*/
enum class FboType {
  color = 1, // カラーテクスチャだけ
  depth = 2, // 深度テクスチャだけ
  colorDepth = color | depth, // カラーテクスチャと深度テクスチャ
};

/**
* フレームバッファオブジェクト
*/
class FramebufferObject
{
public:
  FramebufferObject(int w, int h, FboType type = FboType::colorDepth);
  ~FramebufferObject();
  FramebufferObject(const FramebufferObject&) = delete;
  FramebufferObject& operator=(const FramebufferObject&) = delete;

  void Bind() const;
  void Unbind() const;
  void BindColorTexture(GLuint) const;
  void UnbindColorTexture(GLuint) const;
  void BindDepthTexture(GLuint) const;
  void UnbindDepthTexture(GLuint) const;

#ifdef SUPRESS_SHADOW_JITTERING
  GLint GetWidth() const {
    if (texColor) {
      return texColor->GetWidth();
    } else if (texDepth) {
      return texDepth->GetWidth();
    }
    return 0;
  }

  GLint GetHeight() const {
    if (texColor) {
      return texColor->GetHeight();
    } else if (texDepth) {
      return texDepth->GetHeight();
    }
    return 0;
  }
#endif // SUPRESS_SHADOW_JITTERING

  // フレームバッファオブジェクトIDを取得する
  GLuint GetId() const { return fbo; }

private:
  GLuint fbo = 0; // フレームバッファオブジェクトのID
  std::shared_ptr<Texture> texColor; // カラーテクスチャ
  std::shared_ptr<Texture> texDepth; // 深度テクスチャ
  int width = 0; // フレームバッファの幅(ピクセル)
  int height = 0; // フレームバッファの高さ(ピクセル)
};

#endif // FRAMEBUFFEROBJECT_H_INCLUDED
