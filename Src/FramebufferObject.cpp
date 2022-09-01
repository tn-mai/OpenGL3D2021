/**
* @file FramebufferObject.cpp
*/
#include "FramebufferObject.h"
#include <iostream>

/**
* �R���X�g���N�^
*
* @param w    �t���[���o�b�t�@�̕�(�s�N�Z����)
* @param h    �t���[���o�b�t�@�̍���(�s�N�Z����)
* @param type FBO�̎��
*/
FramebufferObject::FramebufferObject(int w, int h, FboType type)
{
  // �J���[�e�N�X�`�����쐬
  if (static_cast<int>(type) & 1) {
    texColor.reset(new Texture("FBO(Color)", w, h, nullptr, GL_RGBA, GL_HALF_FLOAT));
    if (!texColor || !texColor->GetId()) {
      std::cerr << "[�G���[]" << __func__ <<
        ":�t���[���o�b�t�@�p�J���[�e�N�X�`���̍쐬�Ɏ��s.\n";
      texColor.reset(); // �J���[�e�N�X�`����j��
      return;
    }
  }

  // �[�x�e�N�X�`�����쐬
  if (static_cast<int>(type) & 2) {
    texDepth.reset(new Texture("FBO(Depth)", w, h, nullptr, GL_DEPTH_COMPONENT32F, GL_FLOAT));
    if (!texDepth || !texDepth->GetId()) {
      std::cerr << "[�G���[]" << __func__ <<
        ":�t���[���o�b�t�@�p�[�x�e�N�X�`���̍쐬�Ɏ��s.\n";
      texColor.reset(); // �J���[�e�N�X�`����j��
      texDepth.reset(); // �[�x�e�N�X�`����j��
      return;
    }
  }

  // �t���[���o�b�t�@�I�u�W�F�N�g���쐬
  glCreateFramebuffers(1, &fbo);
  if (static_cast<int>(type) & 1) {
    glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, texColor->GetId(), 0);
  } else {
    glNamedFramebufferDrawBuffer(fbo, GL_NONE);
  }
  if (static_cast<int>(type) & 2) {
    glNamedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, texDepth->GetId(), 0);
  }

  // �t���[���o�b�t�@�I�u�W�F�N�g���쐬�ł������`�F�b�N
  if (glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "[�G���[]" << __func__ << ":�I�t�X�N���[���o�b�t�@�̍쐬�Ɏ��s.\n";
    glDeleteFramebuffers(1, &fbo);
    fbo = 0;
    texColor.reset();
    texDepth.reset();
    return;
  }
  width = w;
  height = h;
}

/**
* �f�X�g���N�^
*/
FramebufferObject::~FramebufferObject()
{
  glDeleteFramebuffers(1, &fbo);
}

/**
* �O���t�B�b�N�X�p�C�v���C���̕`����FBO�����蓖�Ă�
*/
void FramebufferObject::Bind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glViewport(0, 0, width, height);
}

/**
* �O���t�B�b�N�X�p�C�v���C���̕`�����f�t�H���g�̃t���[���o�b�t�@�ɖ߂�
*/
void FramebufferObject::Unbind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
* �J���[�e�N�X�`�����O���t�B�b�N�X�p�C�v���C���Ɋ��蓖�Ă�
*
* @param unit ���蓖�Đ�̃e�N�X�`�����j�b�g�ԍ�
*/
void FramebufferObject::BindColorTexture(GLuint unit) const
{
  texColor->Bind(unit);
}

/**
* �J���[�e�N�X�`�����O���t�B�b�N�X�p�C�v���C��������O��
*
* @param unit ���蓖�Đ�̃e�N�X�`�����j�b�g�ԍ�
*/
void FramebufferObject::UnbindColorTexture(GLuint unit) const
{
  texColor->Unbind(unit);
}

/**
* �[�x�X�e���V���e�N�X�`�����O���t�B�b�N�X�p�C�v���C���Ɋ��蓖�Ă�
*
* @param unit ���蓖�Đ�̃e�N�X�`�����j�b�g�ԍ�
*/
void FramebufferObject::BindDepthTexture(GLuint unit) const
{
  texDepth->Bind(unit);
}

/**
* �[�x�X�e���V���e�N�X�`�����O���t�B�b�N�X�p�C�v���C��������O��
*
* @param unit ���蓖�Đ�̃e�N�X�`�����j�b�g�ԍ�
*/
void FramebufferObject::UnbindDepthTexture(GLuint unit) const
{
  texDepth->Unbind(unit);
}

