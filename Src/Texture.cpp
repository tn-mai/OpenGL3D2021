/**
* @file Texture.cpp
*/
#include "Texture.h"
#include <vector>
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
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH, &width);
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT, &height);
    std::cout << "[情報]" << __func__ << "テクスチャ" << name << "を作成.\n";
  }
}

/**
* 配列テクスチャを作成するコンストラクタ
*/
Texture::Texture(const char* name, const char** fileList, size_t count)
{
  // 画像ファイルを仮テクスチャとして読み込む
  std::vector<GLuint> texList(count, 0);
  for (int i = 0; i < count; ++i) {
    texList[i] = GLContext::CreateImage2D(fileList[i]);
  }

  // テクスチャのピクセル形式、幅、高さを取得
  GLint internalFormat;
  glGetTextureLevelParameteriv(texList[0], 0, GL_TEXTURE_INTERNAL_FORMAT,
    &internalFormat);
  glGetTextureLevelParameteriv(texList[0], 0, GL_TEXTURE_WIDTH, &width);
  glGetTextureLevelParameteriv(texList[0], 0, GL_TEXTURE_HEIGHT, &height);

  // 配列テクスチャを作成
  glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &id);
  glTextureStorage3D(id, 1, internalFormat, width, height, static_cast<GLsizei>(count));
  this->name = name;

  // 配列テクスチャに画像データを設定
  for (int i = 0; i < count; ++i) {
    glCopyImageSubData(
      texList[i], GL_TEXTURE_2D, 0, 0, 0, 0,
      id, GL_TEXTURE_2D_ARRAY, 0, 0, 0, i,
      width, height, 1);
  }

  // 画像読み込みに使った仮テクスチャを削除
  glDeleteTextures(static_cast<GLsizei>(count), texList.data());

  const GLenum result = glGetError();
  if (result != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << "配列テクスチャ" << name << "の作成に失敗\n";
  } else {
    std::cout << "[情報]" << __func__ << "配列テクスチャ" << name << "を作成\n";
  }
}

/**
* コンストラクタ.
*/
Texture::Texture(const char* name, GLsizei width, GLsizei height,
  const void* data, GLenum pixelFormat, GLenum type)
{
  id = GLContext::CreateImage2D(width, height, data, pixelFormat, type);
  if (id) {
    this->name = name;
    this->width = width;
    this->height = height;
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
    glBindTextures(unit, 1, nullptr);
  }
}

/**
* テクスチャにデータを書き込む
*/
void Texture::Write(GLint x, GLint y, GLsizei width, GLsizei height,
  const void* data, GLenum pixelFormat, GLenum type)
{
  if (id) {
    GLint alignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTextureSubImage2D(id, 0, 0, 0, width, height, pixelFormat, type, data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
  }
}

// テキスト未実装
/**
* テクスチャのバインドを解除する
*/
void UnbindTextures(GLuint start, GLsizei count)
{
  const size_t end = std::min(size_t(start + count), std::size(textureBindingArray));
  for (GLuint i = start; i < end; ++i) {
    textureBindingArray[i] = 0;
  }
  glBindTextures(start, count, nullptr);
}

