/**
* @file Texture.cpp
*/
#include "Texture.h"
#include <iostream>

GLuint textureBindingArray[16];

/**
* コンストラクタ.
*
* @param filename 画像ファイル名.
*/
Texture::Texture(const char* filename)
{
  id = GLContext::CreateImage2D(filename);
  if (id) {
    name = filename;
    std::cout << "[情報]" << __func__ << "テクスチャ" << name << "を作成.\n";
  }
}

/**
* デストラクタ.
*/
Texture::~Texture()
{
  if (id) {
    std::cout << "[情報]" << __func__ << "テクスチャ" << name << "を削除.\n";
  }
  glDeleteTextures(1, &id);
}

/**
* オブジェクトが使える状態かどうかを調べる.
*
* @retval true  使える.
* @retval false 使えない(初期化に失敗している).
*/
bool Texture::IsValid() const
{
   return id;
}


/**
* テクスチャをグラフィックスパイプラインに割り当てる.
*
* @param unit 割り当てるテクスチャイメージユニットの番号.
*/
void Texture::Bind(GLuint unit) const
{
  // ユニット番号が管理範囲外の場合はバインドできない.
  if (unit >= std::size(textureBindingArray)) {
    return;
  }

  // 自分と違うオブジェクトIDが割り当てられていたらバインドする.
  if (textureBindingArray[unit] != id) {
    textureBindingArray[unit] = id;
    glBindTextureUnit(unit, id);
  }
}

/**
* テクスチャの割り当てを解除する.
*
* @param unit 割り当て解除するテクスチャイメージユニットの番号.
*/
void Texture::Unbind(GLuint unit) const
{
  // ユニット番号が管理範囲外の場合はバインドできない.
  if (unit >= std::size(textureBindingArray)) {
    return;
  }

  // 自分のオブジェクトIDが割り当てられていたらバインド解除する.
  if (textureBindingArray[unit] == id) {
    textureBindingArray[unit] = 0;
    glBindTextureUnit(unit, 0);
  }
}


