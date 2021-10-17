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
  Texture(const char* name, const char** fileList, size_t count);
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

  void Write(GLint x, GLint y, GLsizei width, GLsizei height,
    const void* data, GLenum pixelFormat, GLenum type);

  // TODO: テキスト未追加
  const std::string& GetName() const { return name; } // 15bで実装. 15は未実装.
  GLint GetWidth() const {
    GLint w = 0;
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH, &w);
    return w;
  }
  GLint GetHeight() const {
    GLint h = 0;
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT, &h);
    return h;
  }

private:
  std::string name; // 画像ファイル名.
  GLuint id = 0;    // オブジェクトID.
};

#endif // TEXTURE_H_INCLUDED