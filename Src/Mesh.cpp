/**
* @file Mesh.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "Mesh.h"
#include "GLContext.h"
#include <glm/glm.hpp>
#include <fstream>
#include <string>
#include <unordered_map>
#include <stdio.h>
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
*
* @param morphTarget モーフィング終了時のプリミティブ.
*                    モーフィングしない場合はnullptrを指定する.
*/
void Primitive::Draw(const Primitive* morphTarget) const
{
  primitiveBuffer->SetMorphBaseMesh(baseVertex);
  if (morphTarget) {
    primitiveBuffer->SetMorphTargetMesh(morphTarget->baseVertex);
  } else {
    primitiveBuffer->SetMorphTargetMesh(baseVertex);
  }
  glDrawElementsBaseVertex(mode, count, GL_UNSIGNED_SHORT, indices, 0);
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
* プリミティブ用のメモリを解放する.
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
    std::cerr << "[エラー]" << __func__ << ": 頂点カウントに異常があります(max=" <<
      maxVertexCount << ", cur=" << curVertexCount << ")\n";
    return false;
  } else if (maxIndexCount < curIndexCount) {
    std::cerr << "[エラー]" << __func__ << ": インデックスカウントに異常があります(max=" <<
      maxIndexCount << ", cur=" << curIndexCount << ")\n";
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
    sizeof(GLushort) * curIndexCount, curVertexCount, this));

  curVertexCount += static_cast<GLsizei>(vertexCount);
  curIndexCount += static_cast<GLsizei>(indexCount);

  return true;
}

/**
* プリミティブを追加する.
*
* @param filename ロードするOBJファイル名.
*
* @retval true  追加成功.
* @retval false 追加失敗.
*/
bool PrimitiveBuffer::AddFromObjFile(const char* filename)
{
  std::ifstream ifs(filename);
  if (!ifs) {
    std::cerr << "[エラー]" << __func__ << ":`" << filename << "`を開けません.\n";
    return false;
  }

  // データ読み取り用の変数を準備.
  std::vector<glm::vec3> positionList;
  std::vector<glm::vec2> texcoordList;
  std::vector<glm::vec3> normalList;
  struct Face {
    int v;
    int vt;
    int vn;

    bool operator==(const Face& o) const {
      return v == o.v && vt == o.vt && vn == o.vn;
    }
  };
  std::vector<Face> faceList;

  // 容量を予約.
  positionList.reserve(1000);
  texcoordList.reserve(1000);
  normalList.reserve(1000);
  faceList.reserve(1000);

  // ファイルからモデルのデータを読み込む.
  size_t lineNo = 0; // 読み込んだ行数.
  while (!ifs.eof()) {
    std::string line;
    std::getline(ifs, line);
    ++lineNo;

    // 空行は無視.
    if (line.empty()) {
      continue;
    }

    const std::string type = line.substr(0, line.find(' '));

    // コメント行は無視.
    if (type == "#") {
      continue;
    }

    const char* p = line.c_str() + type.size();
    if (type == "v") { // 頂点座標.
      glm::vec3 v(0);
      if (sscanf(p, "%f %f %f", &v.x, &v.y, &v.z) != 3) {
        std::cerr << "[エラー]" << __func__ << ":頂点座標の読み取りに失敗.\n" <<
          "  " << filename << "(" << lineNo << "行目): " << line << "\n";
      }
      positionList.push_back(v);
    } else if (type == "vt") { // テクスチャ座標.
      glm::vec2 vt(0);
      if (sscanf(p, "%f %f", &vt.x, &vt.y) != 2) {
        std::cerr << "[エラー]" << __func__ << ":テクスチャ座標の読み取りに失敗.\n" <<
          "  " << filename << "(" << lineNo << "行目): " << line << "\n";
      }
      texcoordList.push_back(vt);
    } else if (type == "vn") { // 法線.
      glm::vec3 vn(0);
      if (sscanf(p, "%f %f %f", &vn.x, &vn.y, &vn.z) != 3) {
        std::cerr << "[エラー]" << __func__ << ":法線の読み取りに失敗.\n" <<
          "  " << filename << "(" << lineNo << "行目): " << line << "\n";
      }
      // 法線を正規化.
      if (glm::dot(vn, vn) > 0) {
        vn = glm::normalize(vn);
      } else {
        vn = glm::vec3(0, 1, 0);
      }
      normalList.push_back(vn);
    } else if (type == "f") { // 面.
      // 三角形と四角形のみ対応.
      Face f[4];
      const int n = sscanf(p, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
        &f[0].v, &f[0].vt, &f[0].vn,
        &f[1].v, &f[1].vt, &f[1].vn,
        &f[2].v, &f[2].vt, &f[2].vn,
        &f[3].v, &f[3].vt, &f[3].vn);
      // インデックスが負数の場合、現在のデータ位置からの相対オフセットとして扱う.
      for (int i = 0; i < n / 3; ++i) {
        if (f[i].v < 0) {
          f[i].v += static_cast<int>(positionList.size());
        }
        if (f[i].vt < 0) {
          f[i].vt += static_cast<int>(texcoordList.size());
        }
        if (f[i].vn < 0) {
          f[i].vn += static_cast<int>(normalList.size());
        }
      }

      // データ数が9のときは三角形、12のときは四角形.
      if (n == 9) {
        for (int i = 0; i < 3; ++i) {
          faceList.push_back(f[i]);
        }
      } else if (n == 12) {
        static const int indices[] = { 0, 1, 2, 2, 3, 0 };
        for (int i = 0; i < 6; ++i) {
          faceList.push_back(f[indices[i]]);
        }
      } else {
        std::cerr << "[警告]" << __func__ << ":面の頂点数は3または4でなくてはなりません(頂点数=" << n << "). " << filename << "(" << lineNo << "行目).\n";
      }
    } else {
      std::cerr << "[警告]" << __func__ << ":未対応の形式です.\n" <<
        "  " << filename << "(" << lineNo << "行目): " << line << "\n";
    }
  }

  // 頂点データとインデックスデータ用の変数を準備.
  std::vector<glm::vec3> positions;
  std::vector<glm::vec2> texcoords;
  std::vector<glm::vec3> normals;
  std::vector<GLushort> indices;

  const size_t indexCount = faceList.size();
  positions.reserve(indexCount / 3);
  texcoords.reserve(indexCount / 3);
  normals.reserve(indexCount / 3);
  indices.reserve(indexCount);

  // 共有頂点検索用の連想配列.
  struct FaceHash {
    size_t operator()(const Face& f) const {
      const std::hash<int> hash;
      return hash(f.v) ^ hash(f.vt << 10) ^ hash(f.vn << 20);
    }
  };
  std::unordered_map<Face, GLushort, FaceHash> shareableVertexMap;

  // モデルのデータを頂点データとインデックスデータに変換する.
  for (size_t i = 0; i < indexCount; ++i) {
    // 共有可能なデータを検索.
    const auto itr = shareableVertexMap.find(faceList[i]);
    if (itr != shareableVertexMap.end()) {
      // 共有可能な面データが見つかったら、その面データのインデックスを使う.
      indices.push_back(itr->second);
      continue;
    }

    // 共有可能な面データが見つからなければ、新しいインデックスと頂点データを作成する.
    indices.push_back(static_cast<GLushort>(positions.size()));

    // 新しい面データとインデックスを共有可能な面データとして追加.
    shareableVertexMap.emplace(faceList[i], static_cast<GLushort>(positions.size()));
  
    // 頂点座標を変換.
    const int v = faceList[i].v - 1;
    if (v < static_cast<int>(positionList.size())) {
      positions.push_back(positionList[v]);
    } else {
      std::cerr << "[警告]" << __func__ << ":頂点座標インデックス" << v <<
        "は範囲[0, " << positionList.size() - 1 << "]の外を指しています.\n" <<
        "  " << filename << "\n";
      positions.push_back(glm::vec3(0));
    }

    // テクスチャ座標を変換.
    const int vt = faceList[i].vt - 1;
    if (vt < static_cast<int>(texcoordList.size())) {
      texcoords.push_back(texcoordList[vt]);
    } else {
      std::cerr << "[警告]" << __func__ << ":テクスチャ座標インデックス" << vt <<
        "は範囲[0, " << texcoordList.size() - 1 << "]の外を指しています.\n" <<
        "  " << filename << "\n";
      texcoords.push_back(glm::vec2(0));
    }

    // 法線を変換.
    const int vn = faceList[i].vn - 1;
    if (vn < static_cast<int>(normalList.size())) {
      normals.push_back(normalList[vn]);
    } else {
      std::cerr << "[警告]" << __func__ << ":法線インデックス" << vn <<
        "は範囲[0, " <<normalList.size() - 1 << "]の外を指しています.\n" <<
        "  " << filename << "\n";
      normals.push_back(glm::vec3(0, 1, 0));
    }
  }

  // プリミティブを追加する.
  const std::vector<glm::vec4> colors(positions.size(), glm::vec4(1));
  const bool result = Add(positions.size(), positions.data(), colors.data(),
    texcoords.data(), normals.data(), indices.size(), indices.data());
  if (result) {
    std::cout << "[情報]" << __func__ << ":" << filename << "(頂点数=" <<
      positions.size() << " インデックス数=" << indices.size() << ")\n";
  } else {
    std::cerr << "[警告]" << __func__ << ":" << filename << "の読み込みに失敗.\n";
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

/**
* モーフィング開始時のメッシュを設定する.
*
* @param baseVertex 頂点データの位置.
*/
void PrimitiveBuffer::SetMorphBaseMesh(GLuint baseVertex) const
{
  GLContext::SetMorphBaseMesh(vao, vboPosition, vboColor, vboTexcoord, vboNormal, baseVertex);
}

/**
* モーフィング終了時のメッシュを設定する.
*
* @param baseVertex 頂点データの位置.
*/
void PrimitiveBuffer::SetMorphTargetMesh(GLuint baseVertex) const
{
  GLContext::SetMorphTargetMesh(vao, vboPosition, vboNormal, baseVertex);
}

} // namespace Mesh

