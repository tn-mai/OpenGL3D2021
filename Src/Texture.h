/**
* @file Texture.h
*/
#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include <glad/glad.h>
#include "GLContext.h"
#include <string>
#include <memory>

/**
* テクスチャを管理するクラス.
*/
class Texture
{
public:
  Texture(const char* filename);
  ~Texture();

  // オブジェクトの有効性を判定する
  bool IsValid() const;

  // バインド管理
  void Bind(GLuint unit) const;
  void Unbind(GLuint unit) const;

private:
  std::string name; // 画像ファイル名.
  GLuint id = 0;    // オブジェクトID.
};

/// Textureポインタ型.
using TexturePtr = std::shared_ptr<Texture>;

/**
* 2Dテクスチャを作成する.
*
* @param filename 画像ファイル名.
*
* @return 作成したテクスチャ.
*/
inline TexturePtr CreateTexture2D(const char* filename)
{
  return std::make_shared<Texture>(filename);
}

#endif // TEXTURE_H_INCLUDED