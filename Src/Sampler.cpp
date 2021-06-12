/**
* @file Sampler.cpp
*/
#include "Sampler.h"
#include "GLContext.h"
#include <iostream>

/**
* �R���X�g���N�^.
*/
Sampler::Sampler(GLenum wrapMode)
{
  id = GLContext::CreateSampler(wrapMode);
  if (id) {
    std::cout << "[���]" << __func__ << "�T���v��" << id << "���쐬.\n";
  }
}

/**
* �f�X�g���N�^.
*/
Sampler::~Sampler()
{
  if (id) {
    std::cout << "[���]" << __func__ << "�T���v��" << id << "���폜.\n";
  }
  glDeleteSamplers(1, &id);
}

/**
* �I�u�W�F�N�g���g�����Ԃ��ǂ����𒲂ׂ�.
*
* @retval true  �g����.
* @retval false �g���Ȃ�(�������Ɏ��s���Ă���).
*/
bool Sampler::IsValid() const
{
   return id;
}

/**
* �T���v�����O���t�B�b�N�X�p�C�v���C���Ɋ��蓖�Ă�.
*
* @param unit ���蓖�Ă�e�N�X�`���C���[�W���j�b�g�̔ԍ�.
*/
void Sampler::Bind(GLuint unit) const
{
  glBindSampler(unit, id);
}

/**
* �T���v���̊��蓖�Ă���������.
*
* @param unit ���蓖�ĉ�������e�N�X�`���C���[�W���j�b�g�̔ԍ�.
*/
void Sampler::Unbind(GLuint unit) const
{
  glBindSampler(unit, 0);
}

