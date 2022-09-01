/**
* @file ProgramPipeline.cpp
*/
#include "ProgramPipeline.h"
#include "GLContext.h"
#include <iostream>

//#define USE_PROGRAM_OBJECT

/**
* �R���X�g���N�^.
*
* @param vsCode  ���_�V�F�[�_�t�@�C����.
* @param fsCode  �t���O�����g�V�F�[�_�t�@�C����.
*/
ProgramPipeline::ProgramPipeline(const char* vsCode, const char* fsCode)
{
#ifdef USE_PROGRAM_OBJECT
  vp = fp = GLContext::CreateProgramFromFile(vsCode, fsCode);
  if (vp) {
    std::cout << "[���] �v���O�����p�C�v���C�����쐬(id=" << pipeline <<
      ", vp=" << vsCode << ", fp=" << fsCode << ")\n";
  }
#else
  vp = GLContext::CreateProgramFromFile(GL_VERTEX_SHADER, vsCode);
  fp = GLContext::CreateProgramFromFile(GL_FRAGMENT_SHADER, fsCode);
  pipeline = GLContext::CreatePipeline(vp, fp);
  if (!pipeline) {
    std::cout << "[���] �v���O�����p�C�v���C�����쐬(id=" << pipeline <<
      ", vp=" << vsCode << ", fp=" << fsCode << ")\n";
  }
#endif
}

/**
* �f�X�g���N�^.
*/
ProgramPipeline::~ProgramPipeline()
{
#ifdef USE_PROGRAM_OBJECT
  if (vp) {
    std::cout << "[���]�v���O�����p�C�v���C�����폜(id=" << pipeline << ")\n";
  }
  glDeleteProgram(vp);
#else
  if (pipeline) {
    std::cout << "[���]�v���O�����p�C�v���C�����폜(id=" << pipeline << ")\n";
  }
  glDeleteProgramPipelines(1, &pipeline);
  glDeleteProgram(fp);
  glDeleteProgram(vp);
#endif
}

/**
* �I�u�W�F�N�g���g�����Ԃ��ǂ����𒲂ׂ�.
*
* @retval true  �g����.
* @retval false �g���Ȃ�(�������Ɏ��s���Ă���).
*/
bool ProgramPipeline::IsValid() const
{
#ifdef USE_PROGRAM_OBJECT
  return vp;
#else
  return pipeline;
#endif
}

/**
* ���j�t�H�[���ϐ��Ƀf�[�^���R�s�[����.
*
* @param location ���j�t�H�[���ϐ��̈ʒu.
* @param data     ���j�t�H�[���ϐ��ɃR�s�[����f�[�^.
*
* @retval true  �R�s�[����.
* @retval false �R�s�[���s.
*/
bool ProgramPipeline::SetUniform(GLint location, const glm::mat4& data) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  // ���P�[�V�����ԍ��ɂ���ăR�s�[���ύX����
  // - 0�`99: ���_�V�F�[�_
  // - 100�`: �t���O�����g�V�F�[�_
  GLuint program = vp;
  if (location >= 100) {
    program = fp;
  }

  glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, &data[0][0]);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":���j�t�H�[���ϐ��̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* ���j�t�H�[���ϐ��Ƀf�[�^���R�s�[����
*/
bool ProgramPipeline::SetUniform(GLint location, const glm::vec3& data) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  const GLuint program = GetProgram(location);
  glProgramUniform3fv(program, location, 1, &data.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":���j�t�H�[���ϐ��̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* ���j�t�H�[���ϐ��Ƀf�[�^���R�s�[����
*/
bool ProgramPipeline::SetUniform(GLint location, const glm::vec4& data) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g.

  const GLuint program = GetProgram(location);
  glProgramUniform4fv(program, location, 1, &data.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":���j�t�H�[���ϐ��̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

// TODO: �e�L�X�g���ǉ�
/**
* ���j�t�H�[���ϐ��Ƀf�[�^���R�s�[����
*/
bool ProgramPipeline::SetUniform(
  GLint location, const glm::uint* data, size_t size) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g

  const GLuint program = GetProgram(location);
  glProgramUniform1uiv(program, location, static_cast<GLsizei>(size), data);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":���j�t�H�[���ϐ��̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* ���j�t�H�[���ϐ��Ƀf�[�^���R�s�[����
*/
bool ProgramPipeline::SetUniform(
  GLint location, const glm::vec4* data, size_t size) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g

  const GLuint program = GetProgram(location);
  glProgramUniform4fv(program, location, static_cast<GLsizei>(size), &data->x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":���j�t�H�[���ϐ��̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* ���j�t�H�[���ϐ��Ƀf�[�^���R�s�[����
*/
bool ProgramPipeline::SetUniform(GLint location, const glm::mat4* data,
  size_t count) const
{
  glGetError(); // �G���[��Ԃ����Z�b�g

  const GLuint program = GetProgram(location);
  glProgramUniformMatrix4fv(program, location, static_cast<GLsizei>(count),
    GL_FALSE, &data[0][0][0]);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ":���j�t�H�[���ϐ��̐ݒ�Ɏ��s.\n";
    return false;
  }
  return true;
}

/**
* �v���O�����p�C�v���C�����o�C���h����.
*/
void ProgramPipeline::Bind() const
{
#ifdef USE_PROGRAM_OBJECT
  glUseProgram(vp);
#else
  glBindProgramPipeline(pipeline);
#endif
}

/**
* �v���O�����p�C�v���C���̃o�C���h����������.
*/
void ProgramPipeline::Unbind() const
{
#ifdef USE_PROGRAM_OBJECT
  glUseProgram(0);
#else
  glBindProgramPipeline(0);
#endif
}

/**
* ���P�[�V�����ԍ��ɑΉ�����v���O����ID���擾����
*/
GLuint ProgramPipeline::GetProgram(GLint location) const
{
  // ���P�[�V�����ԍ��ɂ���ăR�s�[���ύX����
  // - 0�`99: ���_�V�F�[�_
  // - 100�`: �t���O�����g�V�F�[�_
  if (location >= 100) {
    return fp;
  }
  return vp;
}

