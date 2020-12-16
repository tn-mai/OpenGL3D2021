/**
* @file Texture.cpp
*/
#include "Texture.h"
#include "GLContext.h"
#include <iostream>

// インテルグラフィックスドライバのバグによりglBindTextureUnit(i, 0)が機能しないことへの対応.
#define AVOID_INTEL_DRIVER_BUG_FOR_GLBINDTEXTUREUNIT

/**
* テクスチャ関連の機能を格納する名前空間.
*/
namespace Texture {

namespace /* unnamed */ {

/**
* テクスチャのバインド状態を追跡するための配列.
*
* テクスチャイメージユニットにバインドされたテクスチャIDを保持する.
*/
GLuint textureBindingState[16] = {};

/**
* サンプラのバインド状態を追跡するための配列.
*/
GLuint samplerBindingState[16] = {};

} // unnamed namespace

/**
* 全てのテクスチャのバインドを解除する.
*/
void UnbindAllTextures()
{
  for (GLuint i = 0; i < std::size(textureBindingState); ++i) {
    textureBindingState[i] = 0;
  }
  glBindTextures(0, static_cast<GLsizei>(std::size(textureBindingState)), textureBindingState);
}

/**
* 全てのサンプラのバインドを解除する.
*/
void UnbindAllSamplers()
{
  for (GLuint i = 0; i < std::size(samplerBindingState); ++i) {
    samplerBindingState[i] = 0;
  }
  glBindSamplers(0, static_cast<GLsizei>(std::size(samplerBindingState)), samplerBindingState);
}

/**
* コンストラクタ.
*
* @param filename 2Dテクスチャとして読み込むファイル名.
*/
Image2D::Image2D(const char* filename) : name(filename),
  id(GLContext::CreateImage2D(filename))
{
  if (id) {
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH, &width);
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT, &height);
  }
}

/**
* コンストラクタ.
*
* @param name    テクスチャを識別するための名前.
* @param width   画像の幅(ピクセル数).
* @param height  画像の高さ(ピクセル数).
* @param data    画像データへのポインタ.
* @param pixelFormat  画像のピクセル形式(GL_BGRAなど).
* @param type    画像データの型.
*/
Image2D::Image2D(const char* name, GLsizei width, GLsizei height, const void* data,
  GLenum format, GLenum type) :
  name(name),
  id(GLContext::CreateImage2D(width, height, data, format, type))
{
  if (id) {
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH, &width);
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT, &height);
  }
}

/**
* デストラクタ.
*/
Image2D::~Image2D()
{
  Unbind();
  glDeleteTextures(1, &id);
}

/**
* テクスチャをテクスチャイメージユニットにバインドする.
*
* @param unit バインド先のユニット番号.
*/
void Image2D::Bind(GLuint unit) const
{
  if (unit >= std::size(textureBindingState)) {
    std::cerr << "[エラー]" << __func__ << ": ユニット番号が大きすぎます(unit=" << unit << ")\n";
    return;
  }
// インテルグラフィックスドライバのバグによりglBindTextureUnit(i, 0)が機能しないことへの対応.
#ifndef AVOID_INTEL_DRIVER_BUG_FOR_GLBINDTEXTUREUNIT 
  glBindTextureUnit(unit, id);
#else
  glBindTextures(unit, 1, &id);
#endif
  textureBindingState[unit] = id;
}

/**
* テクスチャのバインドを解除する.
*/
void Image2D::Unbind() const
{
  for (GLuint i = 0; i < std::size(textureBindingState); ++i) {
    if (textureBindingState[i] == id) {
      textureBindingState[i] = 0;
// インテルグラフィックスドライバのバグによりglBindTextureUnit(i, 0)が機能しないことへの対応.
#ifndef AVOID_INTEL_DRIVER_BUG_FOR_GLBINDTEXTUREUNIT 
      glBindTextureUnit(i, 0);
#else
      const GLuint id = 0;
      glBindTextures(i, 1, &id);
#endif
    }
  }
}

/**
* テクスチャの幅を取得する.
*
* @return テクスチャの幅(ピクセル数).
*/
GLsizei Image2D::Width() const
{
  return width;
}

/**
* テクスチャの高さを取得する.
*
* @return テクスチャの高さ(ピクセル数).
*/
GLsizei Image2D::Height() const
{
  return height;
}

/**
* コンストラクタ.
*/
Sampler::Sampler() : id(GLContext::CreateSampler())
{
}

/**
* デストラクタ.
*/
Sampler::~Sampler()
{
  Unbind();
  glDeleteSamplers(1, &id);
}

/**
* ラップモードを指定する.
*
* @param mode ラップモード.
*/
void Sampler::SetWrapMode(GLenum mode)
{
  if (!id) {
    return;
  }
  glSamplerParameteri(id, GL_TEXTURE_WRAP_S, mode);
  glSamplerParameteri(id, GL_TEXTURE_WRAP_T, mode);
}

/**
* フィルタを設定する.
*
* @param filter 設定するフィルタの種類(GL_LINEARかGL_NEAREST).
*/
void Sampler::SetFilter(GLenum filter)
{
  if (!id) {
    return;
  }
  GLenum minFilter = GL_NEAREST_MIPMAP_LINEAR;
  if (filter == GL_NEAREST) {
    minFilter = GL_NEAREST_MIPMAP_NEAREST;
  }
  glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, minFilter);
  glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, filter);
}

/**
* サンプラをテクスチャイメージユニットにバインドする.
*
* @param unit バインド先のユニット番号.
*/
void Sampler::Bind(GLuint unit) const
{
  if (unit >= std::size(textureBindingState)) {
    std::cerr << "[エラー]" << __func__ << ": ユニット番号が大きすぎます(unit=" << unit << ")\n";
    return;
  }
  glBindSampler(unit, id);
  samplerBindingState[unit] = id;
}

/**
* サンプラのバインドを解除する.
*/
void Sampler::Unbind() const
{
  for (GLuint i = 0; i < std::size(samplerBindingState); ++i) {
    if (samplerBindingState[i] == id) {
      samplerBindingState[i] = 0;
      glBindSampler(i, 0);
    }
  }
}

} // namespace Texture

