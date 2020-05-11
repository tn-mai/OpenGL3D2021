/**
* @file Mesh.cpp
*/
#include "Mesh.h"
#include "GLContext.h"
#include <glm/glm.hpp>
#include <iostream>

/**
* 図形データに関する名前空間.
*/
namespace Mesh {

/**
* データをバッファオブジェクトにコピーする.
*
* @param id          コピー先となるバッファオブジェクトID.
* @param unitSize    要素のバイト数.
* @param offsetCount コピー先オフセット(要素単位).
* @param count       コピーする要素数.
* @param data        コピーするデータのアドレス.
*
* @retval true  コピー成功.
* @retval false コピー失敗.
*/
bool CopyData(GLuint id, size_t unitSize, GLsizei offsetCount, size_t count, const void* data)
{
  const GLsizei size = static_cast<GLsizei>(count * unitSize);
  const GLuint tmp = GLContext::CreateBuffer(size, data);
  if (!tmp) {
    std::cerr << "[エラー]" << __func__ << ": コピー元バッファの作成に失敗(size=" << size << ").\n";
    return false;
  }
  const GLsizei offset = static_cast<GLsizei>(offsetCount * unitSize);
  glCopyNamedBufferSubData(tmp, id, 0, offset, size);
  glDeleteBuffers(1, &tmp);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ": データのコピーに失敗(size=" << size << ", offset=" << offset << ").\n";
  }
  return true;
}

/**
* プリミティブを描画する.
*/
void Primitive::Draw() const
{
  glDrawElementsBaseVertex(mode, count, GL_UNSIGNED_SHORT, indices, baseVertex);
}

/**
* デストラクタ.
*/
PrimitiveBuffer::~PrimitiveBuffer()
{
  Free();
}

/**
* プリミティブ用のメモリを確保する.
*
* @param maxVertexCount  格納可能な最大頂点数.
* @param maxIndexCount   格納可能な最大インデックス数.
*
* @retval true  確保成功.
* @retval false 確保失敗、または既に確保済み.
*/
bool PrimitiveBuffer::Allocate(GLsizei maxVertexCount, GLsizei maxIndexCount)
{
  if (vao) {
    std::cerr << "[警告]" << __func__ << ": VAOは作成済みです.\n";
    return false;
  }
  vboPosition = GLContext::CreateBuffer(sizeof(glm::vec3) * maxVertexCount, nullptr);
  vboColor = GLContext::CreateBuffer(sizeof(glm::vec4) * maxVertexCount, nullptr);
  vboTexcoord = GLContext::CreateBuffer(sizeof(glm::vec2) * maxVertexCount, nullptr);
  vboNormal = GLContext::CreateBuffer(sizeof(glm::vec3) * maxVertexCount, nullptr);
  ibo = GLContext::CreateBuffer(sizeof(GLushort) * maxIndexCount, nullptr);
  vao = GLContext::CreateVertexArray(vboPosition, vboColor, vboTexcoord, vboNormal, ibo);
  if (!vboPosition || !vboColor || !vboTexcoord || !vboNormal || !ibo || !vao) {
    std::cerr << "[エラー]" << __func__ << ": VAOの作成に失敗.\n";
    Free();
    return false;
  }
  primitives.reserve(100);
  this->maxVertexCount = maxVertexCount;
  this->maxIndexCount = maxIndexCount;
  return true;
}

/**
* プリミティブ用のメモリを開放する.
*/
void PrimitiveBuffer::Free()
{
  primitives.clear();

  glDeleteVertexArrays(1, &vao);
  vao = 0;
  glDeleteBuffers(1, &ibo);
  ibo = 0;
  glDeleteBuffers(1, &vboTexcoord);
  vboTexcoord = 0;
  glDeleteBuffers(1, &vboColor);
  vboColor = 0;
  glDeleteBuffers(1, &vboPosition);
  vboPosition = 0;

  maxVertexCount = 0;
  curVertexCount = 0;
  maxIndexCount = 0;
  curIndexCount = 0;
}

/**
* プリミティブを追加する.
*
* @param vertexCount 追加する頂点データの数.
* @param pPosition   座標データへのポインタ.
* @param pColor      色データへのポインタ.
* @param pTexcoord   テクスチャ座標データへのポインタ.
* @param pNormal     法線データへのポインタ.
* @param indexCount  追加するインデックスデータの数.
* @param pIndex      インデックスデータへのポインタ.
*
* @retval true  追加成功.
* @retval false 追加失敗.
*/
bool PrimitiveBuffer::Add(size_t vertexCount, const glm::vec3* pPosition,
  const glm::vec4* pColor, const glm::vec2* pTexcoord, const glm::vec3* pNormal, size_t indexCount, const GLushort* pIndex)
{
  if (!vao) {
    std::cerr << "[エラー]" << __func__ << ": VAOが作成されていません.\n";
    return false;
  } else if (maxVertexCount < curVertexCount) {
    std::cerr << "[エラー]" << __func__ << ": 頂点カウントに異常があります(max=" << maxVertexCount << ", cur=" << curVertexCount << ")\n";
    return false;
  } else if (maxIndexCount < curIndexCount) {
    std::cerr << "[エラー]" << __func__ << ": インデックスカウントに異常があります(max=" << maxIndexCount << ", cur=" << curIndexCount << ")\n";
    return false;
  } else if (vertexCount > static_cast<size_t>(maxVertexCount) - curVertexCount) {
    std::cerr << "[警告]" << __func__ << ": VBOが満杯です(max=" << maxVertexCount <<
      ", cur=" << curVertexCount << ", add=" << vertexCount << ")\n";
    return false;
  } else if (indexCount > static_cast<size_t>(maxIndexCount) - curIndexCount) {
    std::cerr << "[警告]" << __func__ << ": IBOが満杯です(max=" << maxIndexCount <<
      ", cur=" << curIndexCount << ", add=" << indexCount << ")\n";
    return false;
  }

  if (!CopyData(vboPosition, sizeof(glm::vec3), curVertexCount, vertexCount, pPosition)) {
    return false;
  }
  if (!CopyData(vboColor, sizeof(glm::vec4), curVertexCount, vertexCount, pColor)) {
    return false;
  }
  if (!CopyData(vboTexcoord, sizeof(glm::vec2), curVertexCount, vertexCount, pTexcoord)) {
    return false;
  }
  if (!CopyData(vboNormal, sizeof(glm::vec3), curVertexCount, vertexCount, pNormal)) {
    return false;
  }
  if (!CopyData(ibo, sizeof(GLushort), curIndexCount, indexCount, pIndex)) {
    return false;
  }

  primitives.push_back(Primitive(GL_TRIANGLES, static_cast<GLsizei>(indexCount),
    sizeof(GLushort) * curIndexCount, curVertexCount));

  curVertexCount += static_cast<GLsizei>(vertexCount);
  curIndexCount += static_cast<GLsizei>(indexCount);

  return true;
}

/**
* プリミティブを取得する.
*
* @param n プリミティブのインデックス.
*
* @return nに対応するプリミティブ.
*/
const Primitive& PrimitiveBuffer::Get(size_t n) const
{
  if (n < 0 || n > static_cast<int>(primitives.size())) {
    std::cerr << "[警告]" << __func__ << ":" << n <<
      "は無効なインデックスです(有効範囲0～" << primitives.size() - 1 << ").\n";
    static const Primitive dummy;
    return dummy;
  }
  return primitives[n];
}

/**
* VAOをバインドする.
*/
void PrimitiveBuffer::BindVertexArray() const
{
  glBindVertexArray(vao);
}

/**
* VAOのバインドを解除する.
*/
void PrimitiveBuffer::UnbindVertexArray() const
{
  glBindVertexArray(0);
}

} // namespace Mesh

