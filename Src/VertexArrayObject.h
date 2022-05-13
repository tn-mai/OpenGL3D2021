/**
* @file VertexArrayObject.h
*/
#ifndef VERTEXARRAYOBJECT_H_INCLUDED
#define VERTEXARRAYOBJECT_H_INCLUDED
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <memory>

/**
* 頂点属性配列オブジェクト(VAO)
*/
class VertexArrayObject
{
public:
  VertexArrayObject();
  ~VertexArrayObject();

  void SetAttribute(GLuint bindingPoint, GLuint index,
    GLint size, GLenum type, GLboolean normalized, GLuint offset) const;
  void SetVBO(GLuint bindingPoint, GLuint vbo,
    GLintptr offset, GLsizei stride) const;
  void SetIBO(GLuint ibo);

  void Bind() const;
  void Unbind() const;

private:
  // コピーを禁止
  VertexArrayObject(const VertexArrayObject&) = delete;
  VertexArrayObject& operator=(const VertexArrayObject&) = delete;

  GLuint vao = 0;
};
using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;

#endif // VERTEXARRAYOBJECT_H_INCLUDED
