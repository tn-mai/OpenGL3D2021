/**
* @file GLContext.cpp
*/
#include "GLContext.h"
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

/**
* OpenGL�R���e�L�X�g�Ɋւ���@�\���i�[���閼�O���.
*/
namespace GLContext {

/**
* �o�b�t�@�I�u�W�F�N�g���쐬����.
*
* @param size �f�[�^�̃T�C�Y.
* @param data �f�[�^�ւ̃|�C���^.
*
* @return �쐬�����o�b�t�@�I�u�W�F�N�g.
*/
GLuint CreateBuffer(GLsizeiptr size, const GLvoid* data)
{
  GLuint id = 0;
  glCreateBuffers(1, &id);
  glNamedBufferStorage(id, size, data, 0);
  return id;
}

/**
* Vertex Array Object���쐬����.
*
* @param vboPosition VAO�Ɋ֘A�t��������W�f�[�^.
* @param vboColor    VAO�Ɋ֘A�t������J���[�f�[�^.
* @param vboTexcoord VAO�Ɋ֘A�t������e�N�X�`�����W�f�[�^.
* @param ibo         VAO�Ɋ֘A�t������C���f�b�N�X�f�[�^.
*
* @return �쐬����VAO.
*/
GLuint CreateVertexArray(GLuint vboPosition, GLuint vboColor,
  GLuint vboTexcoord, GLuint ibo)
{
  if (!vboPosition || !vboColor || !vboTexcoord || !ibo) {
    std::cerr << "[�G���[]" << __func__ << ":�o�b�t�@�I�u�W�F�N�g��0�ł��B\n";
    return 0;
  }

  GLuint id = 0;
  glCreateVertexArrays(1, &id);

  const GLuint positionIndex = 0;
  const GLuint positionBindingIndex = 0;
  glEnableVertexArrayAttrib(id, positionIndex);
  glVertexArrayAttribFormat(id, positionIndex, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(id, positionIndex, positionBindingIndex);
  glVertexArrayVertexBuffer(
    id, positionBindingIndex, vboPosition, 0, sizeof(Position));

  const GLuint colorIndex = 1;
  const GLuint colorBindingIndex = 1;
  glEnableVertexArrayAttrib(id, colorIndex);
  glVertexArrayAttribFormat(id, colorIndex, 4, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(id, colorIndex, colorBindingIndex);
  glVertexArrayVertexBuffer(id, colorBindingIndex, vboColor, 0, sizeof(Color));

  const GLuint texcoordIndex = 2;
  const GLuint texcoordBindingIndex = 2;
  glEnableVertexArrayAttrib(id, texcoordIndex);
  glVertexArrayAttribFormat(id, texcoordIndex, 2, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(id,texcoordIndex, texcoordBindingIndex);
  glVertexArrayVertexBuffer(
    id, texcoordBindingIndex, vboTexcoord, 0, sizeof(glm::vec2));

  glVertexArrayElementBuffer(id, ibo);

  return id;
}

/**
* �V�F�[�_�[�E�v���O�������r���h����.
*
* @param type �V�F�[�_�[�̎��.
* @param code �V�F�[�_�[�E�v���O�����ւ̃|�C���^.
*
* @retval 0���傫�� �쐬�����v���O�����E�I�u�W�F�N�g.
* @retval 0          �v���O�����E�I�u�W�F�N�g�̍쐬�Ɏ��s.
*/
GLuint CreateProgram(GLenum type, const GLchar* code)
{
  GLuint program = glCreateShaderProgramv(type, 1, &code);

  GLint status = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint infoLen = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen) {
      std::vector<char> buf;
      buf.resize(infoLen);
      if ((int)buf.size() >= infoLen) {
        glGetProgramInfoLog(program, infoLen, nullptr, buf.data());
        std::cerr << "[�G���[]" << __func__ <<
          ":�V�F�[�_�[�̃r���h�Ɏ��s.\n" << buf.data() << "\n";
      }
    }
    glDeleteProgram(program);
    return 0;
  }
  return program;
}

/**
* �p�C�v���C���E�I�u�W�F�N�g���쐬����.
*
* @param vp  ���_�V�F�[�_�[�E�v���O����.
* @param fp  �t���O�����g�V�F�[�_�[�E�v���O����.
*
* @retval 0���傫�� �쐬�����p�C�v���C���E�I�u�W�F�N�g.
* @retval 0         �p�C�v���C���E�I�u�W�F�N�g�̍쐬�Ɏ��s.
*/
GLuint CreatePipeline(GLuint vp, GLuint fp)
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  GLuint id;
  glCreateProgramPipelines(1, &id);
  glUseProgramStages(id, GL_VERTEX_SHADER_BIT, vp);
  glUseProgramStages(id, GL_FRAGMENT_SHADER_BIT, fp);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":�v���O�����p�C�v���C���̍쐬�Ɏ��s.\n";
    glDeleteProgramPipelines(1, &id);
    return 0;
  }

  GLint testVp = 0;
  glGetProgramPipelineiv(id, GL_VERTEX_SHADER, &testVp);
  if (testVp != vp) {
    std::cerr << "[�G���[]" << __func__ << ":���_�V�F�[�_�̐ݒ�Ɏ��s.\n";
    glDeleteProgramPipelines(1, &id);
    return 0;
  }
  GLint testFp = 0;
  glGetProgramPipelineiv(id, GL_FRAGMENT_SHADER, &testFp);
  if (testFp != fp) {
    std::cerr << "[�G���[]" << __func__ << ":�t���O�����g�V�F�[�_�̐ݒ�Ɏ��s.\n";
    glDeleteProgramPipelines(1, &id);
    return 0;
  }
  return id;
}

/**
* �T���v���E�I�u�W�F�N�g���쐬����.
*
* @param wrapMode  ���b�v�E���[�h.
*
* @retval 0���傫�� �쐬�����T���v���E�I�u�W�F�N�g.
* @retval 0          �T���v���E�I�u�W�F�N�g�̍쐬�Ɏ��s.
*/
GLuint CreateSampler(GLenum wrapMode)
{
  GLuint id;
  glCreateSamplers(1, &id);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":���b�v���[�h�ł͂Ȃ��l���w�肳�ꂽ.\n";
    glDeleteSamplers(1, &id);
    return 0;
  }

  // ���b�v�E���[�h��ݒ肷��.
  glSamplerParameteri(id, GL_TEXTURE_WRAP_S, wrapMode);
  glSamplerParameteri(id, GL_TEXTURE_WRAP_T, wrapMode);
  if (glGetError() != GL_NO_ERROR) {
    glDeleteSamplers(1, &id);
    return 0;
  }

  // �t�B���^��ݒ肷��.
  glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":�t�B���^�ł͂Ȃ��l���w�肳�ꂽ.\n";
    glDeleteSamplers(1, &id);
    return 0;
  }

  return id;
}

/**
* 2D�e�N�X�`�����쐬����.
*
* @param width   �摜�̕�(�s�N�Z����).
* @param height  �摜�̍���(�s�N�Z����).
* @param data    �摜�f�[�^�̃A�h���X.
*
* @retval 0�ȊO  �쐬�����e�N�X�`���E�I�u�W�F�N�g��ID.
* @retval 0      �e�N�X�`���̍쐬�Ɏ��s.
*/
GLuint CreateImage2D(GLsizei width, GLsizei height, const void* data)
{
  GLuint id;

  // �e�N�X�`���E�I�u�W�F�N�g���쐬���AGPU���������m�ۂ���.
  glCreateTextures(GL_TEXTURE_2D, 1, &id);
  glTextureStorage2D(id, 1, GL_RGBA8, width, height);

  // GPU�������Ƀf�[�^��]������.
  glTextureSubImage2D(id, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
  const GLenum result = glGetError();
  if (result != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << "�e�N�X�`���̍쐬�Ɏ��s\n";
    glDeleteTextures(1, &id);
    return 0;
  }

  return id;
}

} // namespace GLContext