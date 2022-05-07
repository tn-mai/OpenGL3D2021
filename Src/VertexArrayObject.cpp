/**
* @file VertexArrayObject.cpp
*/
#include "VertexArrayObject.h"

/**
* コンストラクタ
*/
VertexArrayObject::VertexArrayObject()
{
  glCreateVertexArrays(1, &vao);
}

/**
* デストラクタ
*/
VertexArrayObject::~VertexArrayObject()
{
  glDeleteVertexArrays(1, &vao);
}

/**
* 頂点アトリビュートを設定する
*
* @param bindingPoint   アトリビュート番号を割り当てるバインディングポイント
* @param index          設定するアトリビュート番号
* @param size           データの要素数
* @param type           要素の型
* @param normalized     正規化の有無
* @param relativeOffset データ単位の先頭からの相対オフセット
*/
void VertexArrayObject::SetAttribute(GLuint bindingPoint, GLuint index,
  GLint size, GLenum type, GLboolean normalized, GLuint relativeOffset) const
{
  glEnableVertexArrayAttrib(vao, index);
  glVertexArrayAttribBinding(vao, index, bindingPoint);
  glVertexArrayAttribFormat(vao, index, size, type, normalized, relativeOffset);
}

/**
* VBOをバインディングポイントに割り当てる
*
* @param bindingPoint VBOを割り当てるバインディングポイント
* @param vbo          バインディングポイントに割り当てるVBO
* @param offset       VBO内の最初の要素までのオフセット
* @param stride       ある要素の先頭から次の要素の先頭までのバイト数
*/
void VertexArrayObject::SetVBO(GLuint bindingPoint, GLuint vbo,
  GLintptr offset, GLsizei stride) const
{
  glVertexArrayVertexBuffer(vao, bindingPoint, vbo, offset, stride);
}

/**
* IBOを割り当てる
*
* @param ibo  VAOに割り当てるIBO
*/
void VertexArrayObject::SetIBO(GLuint ibo)
{
  glVertexArrayElementBuffer(vao, ibo);
}

/**
* VAOをグラフィックスパイプラインに割り当てる
*/
void VertexArrayObject::Bind() const
{
  glBindVertexArray(vao);
}

/**
* VAOのグラフィックスパイプラインへの割り当てを解除する
*/
void VertexArrayObject::Unbind() const
{
  glBindVertexArray(0);
}

