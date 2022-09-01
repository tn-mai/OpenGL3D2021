/**
* @file Texture.cpp
*/
#include "Texture.h"
#include <vector>
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
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_WIDTH, &width);
    glGetTextureLevelParameteriv(id, 0, GL_TEXTURE_HEIGHT, &height);
    std::cout << "[���]" << __func__ << "�e�N�X�`��" << name << "���쐬.\n";
  }
}

/**
* �z��e�N�X�`�����쐬����R���X�g���N�^
*/
Texture::Texture(const char* name, const char** fileList, size_t count)
{
  // �摜�t�@�C�������e�N�X�`���Ƃ��ēǂݍ���
  std::vector<GLuint> texList(count, 0);
  for (int i = 0; i < count; ++i) {
    texList[i] = GLContext::CreateImage2D(fileList[i]);
  }

  // �e�N�X�`���̃s�N�Z���`���A���A�������擾
  GLint internalFormat;
  glGetTextureLevelParameteriv(texList[0], 0, GL_TEXTURE_INTERNAL_FORMAT,
    &internalFormat);
  glGetTextureLevelParameteriv(texList[0], 0, GL_TEXTURE_WIDTH, &width);
  glGetTextureLevelParameteriv(texList[0], 0, GL_TEXTURE_HEIGHT, &height);

  // �z��e�N�X�`�����쐬
  glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &id);
  glTextureStorage3D(id, 1, internalFormat, width, height, static_cast<GLsizei>(count));
  this->name = name;

  // �z��e�N�X�`���ɉ摜�f�[�^��ݒ�
  for (int i = 0; i < count; ++i) {
    glCopyImageSubData(
      texList[i], GL_TEXTURE_2D, 0, 0, 0, 0,
      id, GL_TEXTURE_2D_ARRAY, 0, 0, 0, i,
      width, height, 1);
  }

  // �摜�ǂݍ��݂Ɏg�������e�N�X�`�����폜
  glDeleteTextures(static_cast<GLsizei>(count), texList.data());

  const GLenum result = glGetError();
  if (result != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << "�z��e�N�X�`��" << name << "�̍쐬�Ɏ��s\n";
  } else {
    std::cout << "[���]" << __func__ << "�z��e�N�X�`��" << name << "���쐬\n";
  }
}

/**
* �R���X�g���N�^.
*/
Texture::Texture(const char* name, GLsizei width, GLsizei height,
  const void* data, GLenum pixelFormat, GLenum type)
{
  id = GLContext::CreateImage2D(width, height, data, pixelFormat, type);
  if (id) {
    this->name = name;
    this->width = width;
    this->height = height;
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
    glBindTextures(unit, 1, nullptr);
  }
}

/**
* �e�N�X�`���Ƀf�[�^����������
*/
void Texture::Write(GLint x, GLint y, GLsizei width, GLsizei height,
  const void* data, GLenum pixelFormat, GLenum type)
{
  if (id) {
    GLint alignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTextureSubImage2D(id, 0, 0, 0, width, height, pixelFormat, type, data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
  }
}

// �e�L�X�g������
/**
* �e�N�X�`���̃o�C���h����������
*/
void UnbindTextures(GLuint start, GLsizei count)
{
  const size_t end = std::min(size_t(start + count), std::size(textureBindingArray));
  for (GLuint i = start; i < end; ++i) {
    textureBindingArray[i] = 0;
  }
  glBindTextures(start, count, nullptr);
}

