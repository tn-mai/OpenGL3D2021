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
  Texture(const char* name, const char** fileList, size_t count); // 15b�Ŏ���. 15�͖�����.
  Texture(const char* name, GLsizei width, GLsizei height,
    const void* data, GLenum pixelFormat, GLenum type);
  ~Texture();

  // �I�u�W�F�N�g�̗L�����𔻒肷��
  bool IsValid() const;

  // �o�C���h�Ǘ�
  void Bind(GLuint unit) const;
  void Unbind(GLuint unit) const;

  // �e�N�X�`��ID���擾
  GLuint GetId() const { return id; }
  // 19b�Ŏ���. 19�͖�����.
  void* GetIdByPtr() const
  {
    return reinterpret_cast<void*>(static_cast<uintptr_t>(id));
  }

  // 15b�Ŏ���. 15�͖�����.
  // �e�N�X�`�������擾
  const std::string& GetName() const { return name; }

  // 15b�Ŏ���. 15�͖�����
  void Write(GLint x, GLint y, GLsizei width, GLsizei height,
    const void* data, GLenum pixelFormat, GLenum type);

  // �e�N�X�`���̕��A�������擾
  GLint GetWidth() const { return width; }
  GLint GetHeight() const { return height; }

private:
  std::string name; // �摜�t�@�C����.
  GLuint id = 0;    // �I�u�W�F�N�gID.

  // 19�Ŏ���. 19b�͖�����.
  GLsizei width = 0;
  GLsizei height = 0;
};

// �e�L�X�g������
void UnbindTextures(GLuint start, GLsizei count);

#endif // TEXTURE_H_INCLUDED