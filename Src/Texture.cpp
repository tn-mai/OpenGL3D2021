/**
* @file Texture.cpp
*/
#include "Texture.h"
#include "GLContext.h"
#include <iostream>

namespace /* unnamed */ {

/**
* テクスチャのバインド状態を追跡するための配列.
*
* テクスチャイメージユニットにバインドされたテクスチャIDを保持する.
*/
GLuint textureBindingState[16] = {};

} // unnamed namespace

/**
* 全てのテクスチャのバインドを解除する.
*/
void UnbindAllTextures()
{
  for (GLuint i = 0; i < std::size(textureBindingState); ++i) {
    glBindTextureUnit(i, 0);
    textureBindingState[i] = 0;
  }
}

/**
* コンストラクタ.
*
* @param filename 2Dテクスチャとして読み込むファイル名.
*/
Image2D::Image2D(const char* filename) : id(GLContext::CreateImage2D(filename))
{
  if (id) {
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH, &width);
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT, &height);
  }
}

/**
* コンストラクタ.
*
* @param width   画像の幅(ピクセル数).
* @param height  画像の高さ(ピクセル数).
* @param data    画像データへのポインタ.
* @param pixelFormat  画像のピクセル形式(GL_BGRAなど).
* @param type    画像データの型.
*/
Image2D::Image2D(GLsizei width, GLsizei height, const void* data, GLenum format, GLenum type) :
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
  glBindTextureUnit(unit, id);
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
      glBindTextureUnit(i, 0);
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
