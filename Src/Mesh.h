/**
* @file Mesh.h
**/
#ifndef MESH_H_INCLUDED
#define MESH_H_INCLUDED
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <vector>

namespace Mesh {

// 先行宣言.
class PrimitiveBuffer;

/**
* プリミティブデータ.
*/
class Primitive
{
public:
  Primitive() = default;
  Primitive(GLenum m, GLsizei c, GLsizeiptr o, GLint b, const PrimitiveBuffer* pb) :
    mode(m), count(c), indices(reinterpret_cast<GLvoid*>(o)), baseVertex(b), primitiveBuffer(pb)
  {}
  ~Primitive() = default;

  void Draw(const Primitive* morphTarget = nullptr) const;

private:
  GLenum mode = GL_TRIANGLES; ///< プリミティブの種類.
  GLsizei count = 0; ///< 描画するインデックス数.
  const GLvoid* indices = 0; ///< 描画開始インデックスのバイトオフセット.
  GLint baseVertex = 0; ///< インデックス0番とみなされる頂点配列内の位置.
  const PrimitiveBuffer* primitiveBuffer = nullptr;
};

/**
* プリミティブバッファ.
*/
class PrimitiveBuffer
{
public:
  PrimitiveBuffer() = default;
  ~PrimitiveBuffer();
  PrimitiveBuffer(const PrimitiveBuffer&) = delete;
  PrimitiveBuffer& operator=(const PrimitiveBuffer&) = delete;

  // メモリ管理.
  bool Allocate(GLsizei maxVertexCount, GLsizei maxIndexCount);
  void Free();

  // プリミティブの追加と参照.
  bool Add(size_t vertexCount, const glm::vec3* pPosition, const glm::vec4* pColor,
    const glm::vec2* pTexcoord, const glm::vec3* pNormal, size_t indexCount, const GLushort* pIndex);
  bool AddFromObjFile(const char* filename);
  const Primitive& Get(size_t n) const;

  // VAOバインド管理.
  void BindVertexArray() const;
  void UnbindVertexArray() const;

  void SetMorphBaseMesh(GLuint offset) const;
  void SetMorphTargetMesh(GLuint offset) const;

private:
  std::vector<Primitive> primitives;

  GLuint vboPosition = 0;
  GLuint vboColor = 0;
  GLuint vboTexcoord = 0;
  GLuint vboNormal = 0;
  GLuint ibo = 0;
  GLuint vao = 0;

  GLsizei maxVertexCount = 0; // 格納できる最大頂点数.
  GLsizei curVertexCount = 0; // 格納済み頂点数.
  GLsizei maxIndexCount = 0; // 格納できる最大インデックス数.
  GLsizei curIndexCount = 0; // 格納済みインデックス数.
};

} // namespace Mesh


#endif // MESH_H_INCLUDED
