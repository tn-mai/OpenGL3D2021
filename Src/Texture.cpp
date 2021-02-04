/**
* @file Texture.cpp
*/
#include "Texture.h"
#include "GLContext.h"
#include <iostream>

// インテルグラフィックスドライバのバグによりglBindTextureUnit(i, 0)が機能しないことへの対応.
#define AVOID_INTEL_DRIVER_BUG_FOR_GLBINDTEXTUREUNIT

// AMDドライバのバグによりglBindTextures, glBindTextureUnitを使うとバインド解除が遅延する問題への対応.
// glBindTextures, glBindTextureUnitによってテクスチャをバインドしたとき、
// バインド解除にglBindTextures, glBindTextureUnitを使うと直後の描画ではバインドが解除されないことがある.
// 同じテクスチャを「バインド->アンバインド」する場合は問題ないが、「バインド*n->アンバインド」とすると遅延が発生する.
// 参照カウントを疑ったが「バインド*n->アンバインド*n」でも遅延するので、そうでない可能性がある.
// しかし、アンバインドにglActiveTexture&glBindTextureを使うと遅延せず、直後の描画コマンドでも正しくアンバインドされている.
// また、バインドにglActiveTexture&glBindTextureを使うと、どの方法でも1回で正しくアンバインドされる.
// バインドに`glActiveTexture&glBindTexture`を使っておけばアンバインドはどの方法でも機能するように見えるが、
// 正直なところ、バインドとアンバインドで異なる関数を使った結果、またバグを踏むかもしれず不安は残る.
// 結局、"AMDドライバではglBindTextures, glBindTextureUnitを使うな"ということになる.
// なおAMDは既にOpenGLドライバの更新を終了してVulkanに移行している. この問題が改善される日は来ないだろう.
#define AVOID_AMD_DRIVER_BUG_FOR_GLBINDTEXTURES

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
* テクスチャのバインドを解除する.
*
* @param unit 解除するバインディングポイント.
*/
void UnbindTexture(GLuint unit)
{
#ifndef AVOID_INTEL_DRIVER_BUG_FOR_GLBINDTEXTUREUNIT 
  glBindTextureUnit(unit, 0);
#else
# ifndef AVOID_AMD_DRIVER_BUG_FOR_GLBINDTEXTURES
  glBindTextures(unit, 1, nullptr);
# else
  static const GLenum targets[] = {
    GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D,
    GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D_ARRAY,
    GL_TEXTURE_RECTANGLE,
    GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_ARRAY,
    GL_TEXTURE_BUFFER,
    GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_2D_MULTISAMPLE_ARRAY
  };
  glActiveTexture(GL_TEXTURE0 + unit);
  for (auto e : targets) {
    glBindTexture(e, 0);
  }
  glActiveTexture(GL_TEXTURE0);
# endif
#endif
  textureBindingState[unit] = 0;
}

/**
* 全てのテクスチャのバインドを解除する.
*/
void UnbindAllTextures()
{
#ifndef AVOID_AMD_DRIVER_BUG_FOR_GLBINDTEXTURES
  for (GLuint i = 0; i < std::size(textureBindingState); ++i) {
    textureBindingState[i] = 0;
  }
  glBindTextures(0, static_cast<GLsizei>(std::size(textureBindingState)), textureBindingState);
#else
  for (GLuint i = 0; i < std::size(textureBindingState); ++i) {
    UnbindTexture(i);
  }
#endif
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
#ifndef AVOID_INTEL_DRIVER_BUG_FOR_GLBINDTEXTUREUNIT 
  glBindTextureUnit(unit, id);
#else
# ifndef AVOID_AMD_DRIVER_BUG_FOR_GLBINDTEXTURES
  glBindTextures(unit, 1, &id);
# else
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, id);
  glActiveTexture(GL_TEXTURE0);
# endif
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
      UnbindTexture(i);
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

