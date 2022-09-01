/**
* @file ProgramPipeline.h
*/
#ifndef PROGRAMPIPELINE_H_INCLUDED
#define PROGRAMPIPELINE_H_INCLUDED
#include <glad/glad.h>
#include <glm/glm.hpp>

/**
* �V�F�[�_�[�v���O�������Ǘ�����N���X.
*/
class ProgramPipeline
{
public:
  ProgramPipeline(const char* vsCode, const char* fsCode);
  ~ProgramPipeline();

  // �I�u�W�F�N�g�̗L�����𔻒肷��
  bool IsValid() const;

  // ���j�t�H�[���ϐ��̐ݒ�
  bool SetUniform(GLint, const glm::mat4&) const;
  bool SetUniform(GLint, const glm::vec3&) const;
  bool SetUniform(GLint, const glm::vec4&) const;

  // TODO: �e�L�X�g���ǉ�
  bool SetUniform(GLint, const glm::vec4*, size_t) const;
  bool SetUniform(GLint, const glm::uint*, size_t) const;
  bool SetUniform(GLint, const glm::mat4*, size_t) const;

  // �o�C���h�Ǘ�
  void Bind() const;
  void Unbind() const;

private:
  GLuint GetProgram(GLint location) const;

  GLuint vp = 0;       // �v���O�����E�I�u�W�F�N�g(���_�V�F�[�_)
  GLuint fp = 0;       // �v���O�����E�I�u�W�F�N�g(�t���O�����g�V�F�[�_)
  GLuint pipeline = 0; // �v���O�����E�p�C�v���C���E�I�u�W�F�N�g
};

#endif // PROGRAMPIPELINE_H_INCLUDED

