/**
* @file ShaderStorageBuffer.cpp
*/
#include "ShaderStorageBuffer.h"
#include "GLContext.h"
#include <iostream>

/**
* �R���X�g���N�^
*
* @param size �o�b�t�@�I�u�W�F�N�g�Ɋi�[�\�ȃo�C�g��
*/
ShaderStorageBuffer::ShaderStorageBuffer(size_t size) :
  size(size)
{
  for (auto& e : buffer) {
    glCreateBuffers(1, &e.id);
    glNamedBufferStorage(e.id, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
  }
}

/**
* �f�X�g���N�^
*/
ShaderStorageBuffer::~ShaderStorageBuffer()
{
  for (auto& e : buffer) {
    glDeleteBuffers(1, &e.id);
    glDeleteSync(e.sync);
  }
}

/**
* SSBO��GL�R���e�L�X�g�Ɋ��蓖�Ă�
*
* @param index  ���蓖�Đ�̃o�C���f�B���O�|�C���g�ԍ�
* @param offset ���蓖�ĊJ�n�I�t�Z�b�g(256�o�C�g���E�ɍ��킹�邱��)
* @param size   ���蓖�Ă�o�C�g��(0���w�肷��ƑS�̂��w�肵�����ƂɂȂ�)
*/
void ShaderStorageBuffer::Bind(GLuint index, GLintptr offset, GLsizeiptr size)
{
  const Buffer& e = buffer[renderingBufferIndex];
  if (e.id) {
    if (size == 0) {
      size = GetSize();
    }
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, e.id, offset, size);
  }
}

/**
* SSBO�̊��蓖�Ă���������
*
* @param index  ���蓖�Đ�̃o�C���f�B���O�|�C���g�ԍ�
*/
void ShaderStorageBuffer::Unbind(GLuint index)
{
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, 0, 0, 0);
}

/**
* GPU�������Ƀf�[�^���R�s�[����
*
* @param offset �R�s�[��̐擪�I�t�Z�b�g(256�o�C�g���E�ɍ��킹�邱��) 
* @param size   �R�s�[����o�C�g��
* @param data   �R�s�[����f�[�^�̃A�h���X
*/
void ShaderStorageBuffer::BufferSubData(GLintptr offset, GLsizeiptr size, const void* data)
{
  // �T�C�Y��0�̏ꍇ�͉������Ȃ�
  if (size <= 0) {
    return;
  }

  const size_t updatingBufferIndex =
    (renderingBufferIndex + std::size(buffer) - 1) % std::size(buffer);
  Buffer& e = buffer[updatingBufferIndex];
  if (e.id) {
    // �����I�u�W�F�N�g�����݂���ꍇ�A�����̊�����҂�
    if (e.sync) {
      const GLenum result = glClientWaitSync(e.sync, 0, 0);
      switch (result) {
      case GL_ALREADY_SIGNALED:
        // ���Ɋ������Ă���(����)
        break;
      case GL_TIMEOUT_EXPIRED:
        std::cerr << "[�x��]" << __func__ << ":�`��Ɏ��Ԃ��������Ă��܂�(sync=" << e.sync <<")\n";
        glClientWaitSync(e.sync, 0, 1'000'000); // �ő�1�b�ԑ҂�
        break;
      default:
        std::cerr << "[�G���[]" << __func__ << ":�����Ɏ��s(" << result << ")\n";
        break;
      }
      // �����I�u�W�F�N�g���폜
      glDeleteSync(e.sync);
      e.sync = 0;
    }
    glNamedBufferSubData(e.id, offset, size, data);
  }
}

/**
* �_�u���o�b�t�@��؂�ւ���
*/
void ShaderStorageBuffer::SwapBuffers()
{
  renderingBufferIndex = (renderingBufferIndex + std::size(buffer) - 1) % std::size(buffer);
}

/**
* �����I�u�W�F�N�g���쐬����
*
* SSBO���g���`��֐������s��������ɂ��̊֐����Ăяo�����ƁB
*/
void ShaderStorageBuffer::FenceSync()
{
  GLsync& sync = buffer[renderingBufferIndex].sync;
  glDeleteSync(sync);
  sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

