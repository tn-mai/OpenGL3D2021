/**
* @file Primitive.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "Primitive.h"
#include "GLContext.h"
#include <fstream>
#include <string>
#include <stdio.h>
#include <iostream>

/**
* データをGPUメモリにコピーする.
*
* @param writeBuffer コピー先のバッファオブジェクト.
* @param unitSize    要素のバイト数.
* @param offsetCount コピー先オフセット(要素単位).
* @param count       コピーする要素数.
* @param data        コピーするデータのアドレス.
*
* @retval true  コピー成功.
* @retval false コピー失敗.
*/
bool CopyData(GLuint writeBuffer, GLsizei unitSize,
  GLsizei offsetCount, size_t count, const void* data)
{
  const GLsizei size = static_cast<GLsizei>(unitSize * count);
  const GLuint readBuffer = GLContext::CreateBuffer(size, data);
  if (!readBuffer) {
    std::cerr << "[エラー]" << __func__ << ": コピー元バッファの作成に失敗(size=" <<
      size << ").\n";
    return false;
  }
  const GLsizei offset = static_cast<GLsizei>(unitSize * offsetCount);
  glCopyNamedBufferSubData(readBuffer, writeBuffer, 0, offset, size);
  glDeleteBuffers(1, &readBuffer);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ": データのコピーに失敗(size=" <<
      size << ", offset=" << offset << ").\n";
    return false;
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
* プリミティブ用のメモリを確保する.
*
* @param maxVertexCount  格納可能な最大頂点数.
* @param maxIndexCount   格納可能な最大インデックス数.
*/
PrimitiveBuffer::PrimitiveBuffer(GLsizei maxVertexCount, GLsizei maxIndexCount)
{
  vboPosition = GLContext::CreateBuffer(sizeof(glm::vec3) * maxVertexCount, nullptr);
  vboColor = GLContext::CreateBuffer(sizeof(glm::vec4) * maxVertexCount, nullptr);
  vboTexcoord = GLContext::CreateBuffer(sizeof(glm::vec2) * maxVertexCount, nullptr);
  ibo = GLContext::CreateBuffer(sizeof(GLushort) * maxIndexCount, nullptr);
  vao = GLContext::CreateVertexArray(vboPosition, vboColor, vboTexcoord, ibo);
  if (!vboPosition || !vboColor || !vboTexcoord || !ibo || !vao) {
    std::cerr << "[エラー]" << __func__ << ": VAOの作成に失敗.\n";
  }

  primitives.reserve(1000);

  this->maxVertexCount = maxVertexCount;
  this->maxIndexCount = maxIndexCount;
}

/**
* デストラクタ.
*/
PrimitiveBuffer::~PrimitiveBuffer()
{
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &ibo);
  glDeleteBuffers(1, &vboTexcoord);
  glDeleteBuffers(1, &vboColor);
  glDeleteBuffers(1, &vboPosition);
}

/**
* 描画データを追加する.
*
* @param vertexCount 追加する頂点データの数.
* @param pPosition   座標データへのポインタ.
* @param pColor      色データへのポインタ.
* @param pTexcoord   テクスチャ座標データへのポインタ.
* @param indexCount  追加するインデックスデータの数.
* @param pIndex      インデックスデータへのポインタ.
*
* @retval true  追加に成功.
* @retval false 追加に失敗.
*/
bool PrimitiveBuffer::Add(size_t vertexCount, const glm::vec3* pPosition,
  const glm::vec4* pColor, const glm::vec2* pTexcoord,
  size_t indexCount, const GLushort* pIndex)
{
  // エラーチェック.
  if (!vao) {
    std::cerr << "[エラー]" << __func__ <<
      ": VAOの作成に失敗しています.\n";
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

  // GPUメモリに頂点座標データをコピー.
  if (!CopyData(vboPosition, sizeof(glm::vec3), curVertexCount, vertexCount,
    pPosition)) {
    return false;
  }

  // GPUメモリに色データをコピー.
  if (!CopyData(vboColor, sizeof(glm::vec4), curVertexCount, vertexCount, pColor)) {
    return false;
  }

  // GPUメモリにテクスチャ座標データをコピー.
  if (!CopyData(vboTexcoord, sizeof(glm::vec2), curVertexCount, vertexCount,
    pTexcoord)) {
    return false;
  }

  // GPUメモリにインデックスデータをコピー.
  if (!CopyData(ibo, sizeof(GLushort), curIndexCount, indexCount, pIndex)) {
    return false;
  }

  // 描画データを作成.
  const Primitive prim(GL_TRIANGLES, static_cast<GLsizei>(indexCount),
    sizeof(GLushort) * curIndexCount, curVertexCount);

  // 描画データを配列に追加.
  primitives.push_back(prim);

  // 現在のデータ数を、追加したデータ数だけ増やす.
  curVertexCount += static_cast<GLsizei>(vertexCount);
  curIndexCount += static_cast<GLsizei>(indexCount);

  return true;
}

/**
* OBJファイルからプリミティブを追加する.
*
* @param filename ロードするOBJファイル名.
*
* @retval true  追加成功.
* @retval false 追加失敗.
*/
bool PrimitiveBuffer::AddFromObjFile(const char* filename)
{
  // ファイルを開く.
  std::ifstream ifs(filename);
  if (!ifs) {
    std::cerr << "[エラー]" << __func__ << ":`" << filename << "`を開けません.\n";
    return false;
  }

  // データ読み取り用の変数を準備
  std::vector<glm::vec3> objPositions; // 頂点座標用
  std::vector<glm::vec2> objTexcoords; // テクスチャ座標用
  // インデックス用
  struct Index {
    int v, vt;
  };
  std::vector<Index> objIndices;

  // 容量を予約.
  objPositions.reserve(10'000);
  objTexcoords.reserve(10'000);
  objIndices.reserve(10'000);

  // ファイルからモデルのデータを読み込む.
  size_t lineNo = 0; // 読み込んだ行数.
  while (!ifs.eof()) {
    std::string line;
    std::getline(ifs, line); // ファイルから1行読み込む
    ++lineNo;

    // 行の先頭にある空白を読み飛ばす.
    const size_t posData = line.find_first_not_of(' ');
    if (posData != std::string::npos) {
      line = line.substr(posData);
    }

    // 空行またはコメント行なら無視して次の行へ進む.
    if (line.empty() || line[0] == '#') {
      continue;
    }

    // データの種類を取得.
    const size_t endOfType = line.find(' ');
    const std::string type = line.substr(0, endOfType);
    const char* p = line.c_str() + endOfType; // 数値部分を指すポインタ.

    // タイプ別のデータ読み込み処理.
    if (type == "v") { // 頂点座標.
      glm::vec3 v(0);
      if (sscanf(p, "%f %f %f", &v.x, &v.y, &v.z) != 3) {
        std::cerr << "[警告]" << __func__ << ":頂点座標の読み取りに失敗.\n" <<
          "  " << filename << "(" << lineNo << "行目): " << line << "\n";
      }
      objPositions.push_back(v);

    } else if (type == "vt") { // テクスチャ座標.
      glm::vec2 vt(0);
      if (sscanf(p, "%f %f", &vt.x, &vt.y) != 2) {
        std::cerr << "[警告]" << __func__ << ":テクスチャ座標の読み取りに失敗.\n" <<
          "  " << filename << "(" << lineNo << "行目): " << line << "\n";
      }
      objTexcoords.push_back(vt);

    } else if (type == "f") { // 面.
      Index f[3];
      const int n = sscanf(p, " %d/%d %d/%d %d/%d",
        &f[0].v, &f[0].vt,
        &f[1].v, &f[1].vt,
        &f[2].v, &f[2].vt);
      if (n == 6) {
        for (int i = 0; i < 3; ++i) {
          objIndices.push_back(f[i]);
        }
      } else {
        std::cerr << "[警告]" << __func__ << ":面データの読み取りに失敗.\n"
          "  " << filename << "(" << lineNo << "行目): " << line << "\n";
      }

    } else {
      std::cerr << "[警告]" << __func__ << ":未対応の形式です.\n" <<
        "  " << filename << "(" << lineNo << "行目): " << line << "\n";
    }
  }

  // 頂点データとインデックスデータ用の変数を準備.
  std::vector<glm::vec3> positions;
  std::vector<glm::vec4> colors;
  std::vector<glm::vec2> texcoords;
  std::vector<GLushort> indices;

  // データ変換用のメモリを確保.
  const size_t indexCount = objIndices.size();
  positions.reserve(indexCount);
  texcoords.reserve(indexCount);
  indices.reserve(indexCount);

  // OBJファイルのデータをOpenGLのデータに変換.
  for (size_t i = 0; i < indexCount; ++i) {
    // インデックスデータを追加.
    indices.push_back(static_cast<GLushort>(i));

    // 頂点座標を変換.
    const int v = objIndices[i].v - 1;
    if (v < static_cast<int>(objPositions.size())) {
      positions.push_back(objPositions[v]);
    } else {
      std::cerr << "[警告]" << __func__ << ":頂点座標インデックス" << v <<
        "は範囲[0, " << objPositions.size() - 1 << "]の外を指しています.\n" <<
        "  " << filename << "\n";
      positions.push_back(glm::vec3(0));
    }

    // テクスチャ座標を変換.
    const int vt = objIndices[i].vt - 1;
    if (vt < static_cast<int>(objTexcoords.size())) {
      texcoords.push_back(objTexcoords[vt]);
    } else {
      std::cerr << "[警告]" << __func__ << ":テクスチャ座標インデックス" << vt <<
        "は範囲[0, " << objTexcoords.size() - 1 << "]の外を指しています.\n" <<
        "  " << filename << "\n";
      texcoords.push_back(glm::vec2(0));
    }
  }

  // 色データを設定.
  colors.resize(positions.size(), glm::vec4(1));

  // プリミティブを追加する.
  const bool result = Add(positions.size(), positions.data(), colors.data(),
    texcoords.data(), indices.size(), indices.data());
  if (result) {
    std::cout << "[情報]" << __func__ << ":" << filename << "(頂点数=" <<
      positions.size() << " インデックス数=" << indices.size() << ")\n";
  } else {
    std::cerr << "[エラー]" << __func__ << ":" << filename << "の読み込みに失敗.\n";
  }
  return result;
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
  if (n > static_cast<int>(primitives.size())) {
    std::cerr << "[警告]" << __func__ << ":" << n <<
      "は無効なインデックスです(size=" << primitives.size() << ").\n";
    // 仮の描画データを返す.
    static const Primitive dummy;
    return dummy;
  }
  return primitives[n];
}

/**
* VAOをグラフィックスパイプラインにバインドする.
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

