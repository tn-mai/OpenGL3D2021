/**
* @file VertexArrayObject.cpp
*/
#include "VertexArrayObject.h"

/**
* �R���X�g���N�^
*/
VertexArrayObject::VertexArrayObject()
{
  glCreateVertexArrays(1, &vao);
}

/**
* �f�X�g���N�^
*/
VertexArrayObject::~VertexArrayObject()
{
  glDeleteVertexArrays(1, &vao);
}

/**
* ���_�A�g���r���[�g��ݒ肷��
*
* @param bindingPoint   �A�g���r���[�g�ԍ������蓖�Ă�o�C���f�B���O�|�C���g
* @param index          �ݒ肷��A�g���r���[�g�ԍ�
* @param size           �f�[�^�̗v�f��
* @param type           �v�f�̌^
* @param normalized     ���K���̗L��
* @param relativeOffset �f�[�^�P�ʂ̐擪����̑��΃I�t�Z�b�g
*/
void VertexArrayObject::SetAttribute(GLuint bindingPoint, GLuint index,
  GLint size, GLenum type, GLboolean normalized, GLuint relativeOffset) const
{
  glEnableVertexArrayAttrib(vao, index);
  glVertexArrayAttribBinding(vao, index, bindingPoint);
  glVertexArrayAttribFormat(vao, index, size, type, normalized, relativeOffset);
}

/**
* VBO���o�C���f�B���O�|�C���g�Ɋ��蓖�Ă�
*
* @param bindingPoint VBO�����蓖�Ă�o�C���f�B���O�|�C���g
* @param vbo          �o�C���f�B���O�|�C���g�Ɋ��蓖�Ă�VBO
* @param offset       VBO���̍ŏ��̗v�f�܂ł̃I�t�Z�b�g
* @param stride       ����v�f�̐擪���玟�̗v�f�̐擪�܂ł̃o�C�g��
*/
void VertexArrayObject::SetVBO(GLuint bindingPoint, GLuint vbo,
  GLintptr offset, GLsizei stride) const
{
  glVertexArrayVertexBuffer(vao, bindingPoint, vbo, offset, stride);
}

/**
* IBO�����蓖�Ă�
*
* @param ibo  VAO�Ɋ��蓖�Ă�IBO
*/
void VertexArrayObject::SetIBO(GLuint ibo)
{
  glVertexArrayElementBuffer(vao, ibo);
}

/**
* VAO���O���t�B�b�N�X�p�C�v���C���Ɋ��蓖�Ă�
*/
void VertexArrayObject::Bind() const
{
  glBindVertexArray(vao);
}

/**
* VAO�̃O���t�B�b�N�X�p�C�v���C���ւ̊��蓖�Ă���������
*/
void VertexArrayObject::Unbind() const
{
  glBindVertexArray(0);
}

