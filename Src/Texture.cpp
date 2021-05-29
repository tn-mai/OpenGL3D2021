/**
* @file Texture.cpp
*/
#include "Texture.h"
#include <iostream>

GLuint textureBindingArray[16];

/**
* �R���X�g���N�^.
*
* @param filename �摜�t�@�C����.
*/
Texture::Texture(const char* filename)
{
  id = GLContext::CreateImage2D(filename);
  if (id) {
    name = filename;
    std::cout << "[���]" << __func__ << "�e�N�X�`��" << name << "���쐬.\n";
  }
}

/**
* �f�X�g���N�^.
*/
Texture::~Texture()
{
  if (id) {
    std::cout << "[���]" << __func__ << "�e�N�X�`��" << name << "���폜.\n";
  }
  glDeleteTextures(1, &id);
}

/**
* �I�u�W�F�N�g���g�����Ԃ��ǂ����𒲂ׂ�.
*
* @retval true  �g����.
* @retval false �g���Ȃ�(�������Ɏ��s���Ă���).
*/
bool Texture::IsValid() const
{
   return id;
}


/**
* �e�N�X�`�����O���t�B�b�N�X�p�C�v���C���Ɋ��蓖�Ă�.
*
* @param unit ���蓖�Ă�e�N�X�`���C���[�W���j�b�g�̔ԍ�.
*/
void Texture::Bind(GLuint unit) const
{
  // ���j�b�g�ԍ����Ǘ��͈͊O�̏ꍇ�̓o�C���h�ł��Ȃ�.
  if (unit >= std::size(textureBindingArray)) {
    return;
  }

  // �����ƈႤ�I�u�W�F�N�gID�����蓖�Ă��Ă�����o�C���h����.
  if (textureBindingArray[unit] != id) {
    textureBindingArray[unit] = id;
    glBindTextureUnit(unit, id);
  }
}

/**
* �e�N�X�`���̊��蓖�Ă���������.
*
* @param unit ���蓖�ĉ�������e�N�X�`���C���[�W���j�b�g�̔ԍ�.
*/
void Texture::Unbind(GLuint unit) const
{
  // ���j�b�g�ԍ����Ǘ��͈͊O�̏ꍇ�̓o�C���h�ł��Ȃ�.
  if (unit >= std::size(textureBindingArray)) {
    return;
  }

  // �����̃I�u�W�F�N�gID�����蓖�Ă��Ă�����o�C���h��������.
  if (textureBindingArray[unit] == id) {
    textureBindingArray[unit] = 0;
    glBindTextureUnit(unit, 0);
  }
}


