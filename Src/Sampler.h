/**
* @file Sampler.h
*/
#ifndef SAMPLER_H_INCLUDED
#define SAMPLER_H_INCLUDED
#include <glad/glad.h>

/**
* �T���v�����Ǘ�����N���X.
*/
class Sampler
{
public:
  Sampler(GLenum wrapMode);
  ~Sampler();

  // �I�u�W�F�N�g�̗L�����𔻒肷��
  bool IsValid() const;

  // �o�C���h�Ǘ�
  void Bind(GLuint unit) const;
  void Unbind(GLuint unit) const;

private:
  GLuint id = 0; // �I�u�W�F�N�gID
};

#endif // SAMPLER_H_INCLUDED
