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

private:
  std::string name; // �摜�t�@�C����.
  GLuint id = 0;    // �I�u�W�F�N�gID.
};

/// Texture�|�C���^�^.
using TexturePtr = std::shared_ptr<Texture>;

/**
* 2D�e�N�X�`�����쐬����.
*
* @param filename �摜�t�@�C����.
*
* @return �쐬�����e�N�X�`��.
*/
inline TexturePtr CreateTexture2D(const char* filename)
{
  return std::make_shared<Texture>(filename);
}

#endif // TEXTURE_H_INCLUDED