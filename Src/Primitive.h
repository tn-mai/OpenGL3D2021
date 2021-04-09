/**
* @file Primitive.h
**/
#ifndef PRIMITIVE_H_INCLUDED
#define PRIMITIVE_H_INCLUDED
#include <glad/glad.h>

/**
* �v���~�e�B�u�f�[�^.
*/
class Primitive
{
public:
  Primitive() = default;
  Primitive(GLenum m, GLsizei c, size_t o, GLint b) :
    mode(m), count(c), indices(reinterpret_cast<GLvoid*>(o)), baseVertex(b)
  {}
  ~Primitive() = default;

  void Draw() const;

private:
  GLenum mode = GL_TRIANGLES; ///< �v���~�e�B�u�̎��.
  GLsizei count = 0; ///< �`�悷��C���f�b�N�X��.
  const GLvoid* indices = 0; ///< �`��J�n�C���f�b�N�X�̃o�C�g�I�t�Z�b�g.
  GLint baseVertex = 0; ///< �C���f�b�N�X0�ԂƂ݂Ȃ���钸�_�z����̈ʒu.
};

#endif // PRIMITIVE_H_INCLUDED
