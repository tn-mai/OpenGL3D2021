/**
* @file Texture.h
*/
#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include <glad/glad.h>
#include "GLContext.h"
#include <string>

/**
* テクスチャを管理するクラス.
*/
class Texture
{
public:
  Texture(const char* filename);
  Texture(const char* name, const char** fileList, size_t count); // 15bで実装. 15は未実装.
  Texture(const char* name, GLsizei width, GLsizei height,
    const void* data, GLenum pixelFormat, GLenum type);
  ~Texture();

  // オブジェクトの有効性を判定する
  bool IsValid() const;

  // バインド管理
  void Bind(GLuint unit) const;
  void Unbind(GLuint unit) const;

  // テクスチャIDを取得
  GLuint GetId() const { return id; }
  // 19bで実装. 19は未実装.
  void* GetIdByPtr() const
  {
    return reinterpret_cast<void*>(static_cast<uintptr_t>(id));
  }

  // 15bで実装. 15は未実装.
  // テクスチャ名を取得
  const std::string& GetName() const { return name; }

  // 15bで実装. 15は未実装
  void Write(GLint x, GLint y, GLsizei width, GLsizei height,
    const void* data, GLenum pixelFormat, GLenum type);

  // テクスチャの幅、高さを取得
  GLint GetWidth() const { return width; }
  GLint GetHeight() const { return height; }

private:
  std::string name; // 画像ファイル名.
  GLuint id = 0;    // オブジェクトID.

  // 19で実装. 19bは未実装.
  GLsizei width = 0;
  GLsizei height = 0;
};

// テキスト未実装
void UnbindTextures(GLuint start, GLsizei count);

#endif // TEXTURE_H_INCLUDED