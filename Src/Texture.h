/**
* @file Texture.h
*/
#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED
#include <glad/glad.h>
#include "GLContext.h"
#include <string>

/**
* �e�N�X�`�����Ǘ�����N���X.
*/
class Texture
{
public:
  Texture(const char* filename);
  ~Texture();

  // �I�u�W�F�N�g�̗L�����𔻒肷��
  bool IsValid() const;

  // �o�C���h�Ǘ�
  void Bind(GLuint unit) const;
  void Unbind(GLuint unit) const;

  GLuint GetId() const { return id; }
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
  std::string name; // �摜�t�@�C����.
  GLuint id = 0;    // �I�u�W�F�N�gID.
};

#endif // TEXTURE_H_INCLUDED