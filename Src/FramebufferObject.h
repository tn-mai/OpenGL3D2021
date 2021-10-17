/**
* @file FramebufferObject.h
*/
#ifndef FRAMEBUFFEROBJECT_H_INCLUDED
#define FRAMEBUFFEROBJECT_H_INCLUDED
#include "glad/glad.h"
#include "Texture.h"
#include <memory>

/**
* FBO�̎��
*/
enum class FboType {
  color = 1, // �J���[�e�N�X�`������
  depth = 2, // �[�x�e�N�X�`������
  colorDepth = color | depth, // �J���[�e�N�X�`���Ɛ[�x�e�N�X�`��
};

/**
* �t���[���o�b�t�@�I�u�W�F�N�g
*/
class FramebufferObject
{
public:
  FramebufferObject(int w, int h, FboType type = FboType::colorDepth);
  ~FramebufferObject();
  FramebufferObject(const FramebufferObject&) = delete;
  FramebufferObject& operator=(const FramebufferObject&) = delete;

  void Bind() const;
  void Unbind() const;
  void BindColorTexture(GLuint) const;
  void UnbindColorTexture(GLuint) const;
  void BindDepthTexture(GLuint) const;
  void UnbindDepthTexture(GLuint) const;

  // �t���[���o�b�t�@�I�u�W�F�N�gID���擾����
  GLuint GetId() const { return fbo; }

private:
  GLuint fbo = 0; // �t���[���o�b�t�@�I�u�W�F�N�g��ID
  std::shared_ptr<Texture> texColor; // �J���[�e�N�X�`��
  std::shared_ptr<Texture> texDepth; // �[�x�e�N�X�`��
  int width = 0; // �t���[���o�b�t�@�̕�(�s�N�Z��)
  int height = 0; // �t���[���o�b�t�@�̍���(�s�N�Z��)
};

#endif // FRAMEBUFFEROBJECT_H_INCLUDED
