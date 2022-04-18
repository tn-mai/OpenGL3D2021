/**
* @file Primitive.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "Primitive.h"
#include "GLContext.h"
#include "GameEngine.h"
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
* マテリアルで使われているテクスチャの一覧を取得する
*/
TextureList GetTextureList(
  const std::vector<Mesh::Material>& materials)
{
  TextureList textures;
  for (const auto& e : materials) {
    if (!e.tex) {
      continue;
    }
    const auto itr = std::find(textures.begin(), textures.end(), e.tex);
    if (itr == textures.end()) {
      textures.push_back(e.tex);
    }
  }
  return textures;
}

/**
* マテリアルが使うテクスチャの番号一覧を取得する
*/
TextureIndexList GetTextureIndexList(
  const std::vector<Mesh::Material>& materials, const TextureList& textures)
{
  TextureIndexList indices(materials.size(), 0);
  for (int m = 0; m < materials.size(); ++m) {
    for (int i = 0; i < textures.size(); ++i) {
      if (textures[i] == materials[m].tex) {
        indices[m] = i;
        break;
      }
    }
  }
  return indices;
}

/**
* MTLファイルからマテリアルを読み込む
*
* @param  foldername ファイルのあるフォルダ名
* @return filename   MTLファイル名
*/
std::vector<Mesh::Material> LoadMaterial(
  const std::string& foldername, const std::string& filename)
{
  std::vector<Mesh::Material> materials;

  const std::string mtlname = foldername + filename;
  std::ifstream ifs(mtlname);
  if (!ifs) {
    return materials;
  }

  GameEngine& engine = GameEngine::Get();

  Mesh::Material m; // データ読み取り用変数
  size_t lineNo = 0; // 読み込んだ行数
  while (!ifs.eof()) {
    std::string line;
    std::getline(ifs, line); // ファイルから1行読み込む
    ++lineNo;

    // 行の先頭にある空白を読み飛ばす
    int n;
    char ctype[64];
    if (sscanf(line.c_str(), " %63s%n", ctype, &n) < 1) {
      continue;
    }
    // コメント行なら無視して次の行へ進む
    if (ctype[0] == '#') {
      continue;
    }

    const char* p = line.c_str() + n; // 数値部分を指すポインタ

    // タイプ別のデータ読み込み処理
    if (strcmp(ctype, "newmtl") == 0) { // マテリアル名
      // 読み取ったマテリアルを配列に追加
      if (!m.name.empty()) {
        materials.push_back(m);
        m = Mesh::Material(); // マテリアルデータを初期化
      }
      for (; *p == ' ' || *p == '\t'; ++p) {} // 先頭の空白を除去
      m.name = std::string(p);
    }
    else if (strcmp(ctype, "Kd") == 0) { // ディフューズカラー
      if (sscanf(p, " %f %f %f", &m.color.x, &m.color.y, &m.color.z) != 3) {
        std::cerr << "[警告]" << __func__ << ":ディフューズカラーの読み取りに失敗.\n" <<
          "  " << mtlname << "(" << lineNo << "行目): " << line << "\n";
      }
    }
    else if (strcmp(ctype, "d") == 0) { // アルファ値
      if (sscanf(p, " %f", &m.color.w) != 1) {
        std::cerr << "[警告]" << __func__ << ":アルファ値の読み取りに失敗.\n" <<
          "  " << mtlname << "(" << lineNo << "行目): " << line << "\n";
      }
    }
    else if (strcmp(ctype, "map_Kd") == 0) { // ディフューズテクスチャ
      for (; *p == ' ' || *p == '\t'; ++p) {} // 先頭の空白を除去
      const std::string textureName = foldername + p;
      m.tex = engine.LoadTexture(textureName.c_str());
    }
  }

  // 最後に読み取ったマテリアルを配列に追加
  if (!m.name.empty()) {
    materials.push_back(m);
  }
  return materials;
}

/**
* プリミティブを描画する.
*/
void Primitive::Draw() const
{
  glDrawElementsBaseVertex(mode, count, GL_UNSIGNED_SHORT, indices, baseVertex);
}

/**
* インスタンシングありでプリミティブを描画する
*
* @param instanceCount 描画するインスタンス数
*/
void Primitive::DrawInstanced(size_t instanceCount) const
{
  glDrawElementsInstancedBaseVertex(mode, count, GL_UNSIGNED_SHORT, indices,
    static_cast<GLsizei>(instanceCount), baseVertex);
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
  vboNormal = GLContext::CreateBuffer(sizeof(glm::vec3) * maxVertexCount, nullptr);
  vboMaterialGroup =
    GLContext::CreateBuffer(sizeof(glm::u8vec2) * maxVertexCount, nullptr);
  ibo = GLContext::CreateBuffer(sizeof(GLushort) * maxIndexCount, nullptr);
  vao = GLContext::CreateVertexArray(
    vboPosition, vboColor, vboTexcoord, vboNormal, vboMaterialGroup, ibo);
  if (!vboPosition || !vboColor || !vboTexcoord || !vboNormal || !ibo || !vao) {
    std::cerr << "[エラー]" << __func__ << ": VAOの作成に失敗.\n";
  }

  primitives.reserve(1000);
  meshes.reserve(1000);

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
  glDeleteBuffers(1, &vboMaterialGroup);
  glDeleteBuffers(1, &vboNormal);
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
* @param pNormal     法線データへのポインタ.
* @param indexCount  追加するインデックスデータの数.
* @param pIndex      インデックスデータへのポインタ.
*
* @retval true  追加に成功.
* @retval false 追加に失敗.
*/
bool PrimitiveBuffer::Add(size_t vertexCount, const glm::vec3* pPosition,
  const glm::vec4* pColor, const glm::vec2* pTexcoord, const glm::vec3* pNormal,
  const glm::u8vec2* pMaterialGroup,
  size_t indexCount, const GLushort* pIndex, const char* name, GLenum type)
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

  // GPUメモリに法線データをコピー.
  if (!CopyData(vboNormal, sizeof(glm::vec3), curVertexCount, vertexCount, pNormal)) {
    return false;
  }

  // GPUメモリにマテリアル番号・グループ番号をコピー
  // データがない場合はダミーデータをコピーする
  std::vector<glm::u8vec2> dummy;
  if (!pMaterialGroup) {
    dummy.resize(vertexCount, glm::u8vec2(0));
    pMaterialGroup = dummy.data();
  }
  if (!CopyData(vboMaterialGroup, sizeof(glm::u8vec2), curVertexCount,
    vertexCount, pMaterialGroup)) {
    return false;
  }

  // GPUメモリにインデックスデータをコピー.
  if (!CopyData(ibo, sizeof(GLushort), curIndexCount, indexCount, pIndex)) {
    return false;
  }

  // 描画データを作成.
  const Primitive prim(name, type, static_cast<GLsizei>(indexCount),
    sizeof(GLushort) * curIndexCount, curVertexCount);

  // 描画データを配列に追加.
  primitives.push_back(prim);

  // 現在のデータ数を、追加したデータ数だけ増やす.
  curVertexCount += static_cast<GLsizei>(vertexCount);
  curIndexCount += static_cast<GLsizei>(indexCount);

  return true;
}

// インデックス用
struct Index {
  int v = 0;
  int vt = 0;
  int vn = 0;
};

/**
* ear-clipping algorithm.
*/
std::vector<Index> EarClipping(const std::vector<glm::vec3>& positions, std::vector<Index> f)
{
  std::vector<Index> result;
  result.reserve((f.size() - 2) * 3);

  // 最初に法線作成に適した3頂点を探して法線を作成し、その後ear-clippingする.
  // のだが、現在の実装では法線作成に適した3頂点の見つけ方がまずくて、意図した三角形にならない可能性がある.
  glm::vec3 normal;
  bool hasNormal = false;
  for (int i = 0; f.size() > 3; i = (i + 1) % static_cast<int>(f.size())) {
    const int size = static_cast<int>(f.size());
    const glm::vec3 a = positions[f[i + 0].v - 1];
    const glm::vec3 b = positions[f[(i + 1) % size].v - 1];
    const glm::vec3 c = positions[f[(i + 2) % size].v - 1];
    const glm::vec3 ab = b - a;
    const glm::vec3 bc = c - b;
    const glm::vec3 ca = a - c;
    if (hasNormal) {
      // 内角が180度より大きければEarではない.
      const glm::vec3 nba = a - b;
      const glm::vec3 s = glm::cross(bc, nba);
      if (glm::dot(normal, s) < 0) {
        continue;
      }
    }

    const glm::vec3 v = glm::cross(ab, bc); // 面法線(仮)
    bool isContain = false;
    for (int j = (i + 3) % size; j != i; j = (j + 1) % size) {
      const glm::vec3 p = positions[f[j].v - 1];
      const glm::vec3 ap = p - a;
      const glm::vec3 bp = p - b;
      const glm::vec3 cp = p - c;
      const float v0 = glm::dot(v, glm::cross(ab, bp));
      const float v1 = glm::dot(v, glm::cross(bc, cp));
      const float v2 = glm::dot(v, glm::cross(ca, ap));
      isContain |= (v0 > 0 && v1 > 0 && v2 > 0) || (v0 < 0 && v1 < 0 && v2 < 0);
      if (isContain) {
        break;
      }
    }
    // 内部に他の頂点を含まないならEarである.
    if (!isContain) {
      if (hasNormal) {
        result.push_back(f[i + 0]);
        result.push_back(f[(i + 1) % size]);
        result.push_back(f[(i + 2) % size]);
        f.erase(f.begin() + (i + 1) % size);
        --i;
      } else {
        hasNormal = true;
        normal = v;
        i = -1;
      }
    }
  }
  result.push_back(f[0]);
  result.push_back(f[1]);
  result.push_back(f[2]);

  return result;
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

  // フォルダ名を取り出す
  std::string foldername(filename);
  const size_t lastSlashPos = foldername.find_last_of("/\\");
  if (lastSlashPos == std::string::npos) {
    foldername.clear();
  } else {
    foldername.resize(lastSlashPos + 1);
  }

  // データ読み取り用の変数を準備
  std::vector<glm::vec3> objPositions; // OBJファイルの頂点座標用
  std::vector<glm::vec2> objTexcoords; // OBJファイルのテクスチャ座標用
  std::vector<glm::vec3> objNormals;   // OBJファイルの法線
  std::vector<Index> objIndices; // OBJファイルのインデックス

  Mesh mesh;

  // 容量を予約.
  objPositions.reserve(10'000);
  objTexcoords.reserve(10'000);
  objNormals.reserve(10'000);
  objIndices.reserve(10'000);

  // ファイルからモデルのデータを読み込む.
  size_t lineNo = 0; // 読み込んだ行数
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
    const char* p = line.c_str() + endOfType; // 数値部分を指すポインタ

    // タイプ別のデータ読み込み処理.
    if (type == "v") { // 頂点座標
      glm::vec3 v(0);
      if (sscanf(p, "%f %f %f", &v.x, &v.y, &v.z) != 3) {
        std::cerr << "[警告]" << __func__ << ":頂点座標の読み取りに失敗.\n" <<
          "  " << filename << "(" << lineNo << "行目): " << line << "\n";
      }
      objPositions.push_back(v);

    } else if (type == "vt") { // テクスチャ座標
      glm::vec2 vt(0);
      if (sscanf(p, "%f %f", &vt.x, &vt.y) != 2) {
        std::cerr << "[警告]" << __func__ << ":テクスチャ座標の読み取りに失敗.\n" <<
          "  " << filename << "(" << lineNo << "行目): " << line << "\n";
      }
      objTexcoords.push_back(vt);

    } else if (type == "vn") { // 法線
      glm::vec3 vn(0);
      if (sscanf(p, "%f %f %f", &vn.x, &vn.y, &vn.z) != 3) {
        std::cerr << "[警告]" << __func__ << ":法線の読み取りに失敗.\n" <<
          "  " << filename << "(" << lineNo << "行目): " << line << "\n";
      }
      objNormals.push_back(vn);

    } else if (type == "f") { // 面
      std::vector<Index> f;
      for (size_t i = 0; ; ++i) {
        int readBytes = 0;
        Index tmp;
        if (sscanf(p, " %d/%d/%d %n", &tmp.v, &tmp.vt, &tmp.vn, &readBytes) == 3) {
          f.push_back(tmp);
          p += readBytes;
        } else if (sscanf(p, " %d//%d %n", &tmp.v, &tmp.vn, &readBytes) == 2) {
          f.push_back(tmp);
          p += readBytes;
        } else if (sscanf(p, " %d/%d %n", &tmp.v, &tmp.vt, &readBytes) == 2) {
          f.push_back(tmp);
          p += readBytes;
        } else if (sscanf(p, " %d %n", &tmp.v, &readBytes) == 1) {
          f.push_back(tmp);
          p += readBytes;
        } else {
          break;
        }
      }

      if (f.size() >= 3) {
        if (false && f.size() > 3) {
          std::vector<Index> tmp = EarClipping(objPositions, f);
          objIndices.insert(objIndices.end(), tmp.begin(), tmp.end());
          // ユーズマテリアルとグループのインデックス数を更新する
          if (!mesh.useMaterials.empty()) {
            mesh.useMaterials.back().indexCount += static_cast<GLsizei>(tmp.size());
          }
          if (!mesh.groups.empty()) {
            mesh.groups.back().indexCount += static_cast<GLsizei>(tmp.size());
          }
        } else {
          for (size_t i = 2; i < f.size(); ++i) {
            objIndices.push_back(f[0]);
            objIndices.push_back(f[i - 1]);
            objIndices.push_back(f[i]);
          }

          // インデックス数を更新する
          const GLsizei triangleCount = static_cast<GLsizei>(f.size() - 2);
          if (!mesh.useMaterials.empty()) {
            mesh.useMaterials.back().indexCount += triangleCount * 3;
          }
          if (!mesh.groups.empty()) {
            mesh.groups.back().indexCount += triangleCount * 3;
          }
        }
      } else {
        std::cerr << "[警告]" << __func__ << ":面データの読み取りに失敗.\n"
          "  " << filename << "(" << lineNo << "行目): " << line << "\n";
      }
    }
    else if (type == "mtllib") {
      for (; *p == ' '; ++p) {} // 先頭の空白を除去
      const std::vector<Mesh::Material> m = LoadMaterial(foldername, p);
      mesh.materials.insert(mesh.materials.end(), m.begin(), m.end());
    }
    else if (type == "g" || type == "o") {
      // 新しいグループを作成
      for (; *p == ' '; ++p) {} // 先頭の空白を除去
      mesh.groups.push_back(Mesh::Group());
      mesh.groups.back().name = p;
    }
    else if (type == "usemtl") {
      // 新しいユーズマテリアルを作成
      for (; *p == ' '; ++p) {} // 先頭の空白を除去
      mesh.useMaterials.push_back(Mesh::UseMaterial{});

      // 名前が一致するマテリアルの番号を設定
      for (int i = 0; i < mesh.materials.size(); ++i) {
        if (mesh.materials[i].name == p) {
          mesh.useMaterials.back().materialNo = i;
          break;
        }
      }
    }
    else {
      std::cerr << "[警告]" << __func__ << ":未対応の形式です.\n" <<
        "  " << filename << "(" << lineNo << "行目): " << line << "\n";
    }
  }

  // データ変換用の変数を準備.
  std::vector<glm::vec3> positions; // OpenGL用の頂点座標
  std::vector<glm::vec4> colors;    // OpenGL用の色
  std::vector<glm::vec2> texcoords; // OpenGL用のテクスチャ座標
  std::vector<glm::vec3> normals;   // OpenGL用の法線
  std::vector<GLushort> indices;    // OpenGL用のインデックス

  // データ変換用のメモリを確保.
  const size_t indexCount = objIndices.size();
  positions.reserve(indexCount);
  texcoords.reserve(indexCount);
  normals.reserve(indexCount);
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
        "は範囲[0, " << objPositions.size() << ")の外を指しています.\n" <<
        "  " << filename << "\n";
      positions.push_back(glm::vec3(0));
    }

    // テクスチャ座標を変換.
    if (objIndices[i].vt == 0) {
      texcoords.push_back(glm::vec2(0));
    } else {
      const int vt = objIndices[i].vt - 1;
      if (vt < static_cast<int>(objTexcoords.size())) {
        texcoords.push_back(objTexcoords[vt]);
      } else {
        std::cerr << "[警告]" << __func__ << ":テクスチャ座標インデックス" << vt <<
          "は範囲[0, " << objTexcoords.size() << ")の外を指しています.\n" <<
          "  " << filename << "\n";
        texcoords.push_back(glm::vec2(0));
      }
    }

    // 法線データがない場合、面の頂点座標から法線を計算する.
    if (objIndices[i].vn == 0) {
      // 面の頂点座標を配列pに取得.
      const size_t n = i / 3;
      glm::vec3 p[3];
      for (size_t j = 0; j < 3; ++j) {
        const int v = objIndices[n * 3 + j].v - 1;
        p[j] = objPositions[v];
      }

      // 辺aと辺bを計算.
      const glm::vec3 a = p[1] - p[0];
      const glm::vec3 b = p[2] - p[0];

      // aとbに垂直なベクトルを計算.
      glm::vec3 normal = glm::cross(a, b);

      // 垂直ベクトルの長さを計算.
      const float length = sqrt(
        normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);

      // 垂直ベクトルを単位ベクトルに変換.
      normal = normal / length;

      normals.push_back(normal);
    }
    else {
      // 法線を変換.
      const int vn = objIndices[i].vn - 1;
      if (vn < static_cast<int>(objNormals.size())) {
        normals.push_back(objNormals[vn]);
      } else {
        std::cerr << "[警告]" << __func__ << ":法線インデックス" << vn <<
          "は範囲[0, " << objNormals.size() << ")の外を指しています.\n" <<
          "  " << filename << "\n";
        normals.push_back(glm::vec3(0, 1, 0));
      }
    }
  }

  // 色データを設定.
  colors.resize(positions.size(), glm::vec4(1));

  // マテリアル番号を設定
  std::vector<glm::u8vec2> materialGroups(positions.size(), glm::u8vec2(0));
  GLsizei rangeFirst = 0;
  for (int useNo = 0; useNo < mesh.useMaterials.size(); ++useNo) {
    const Mesh::UseMaterial& m = mesh.useMaterials[useNo];
    const int maxMaterialNo = 9; // シェーダで使えるマテリアルは最大10個
    const int n = glm::clamp(m.materialNo, 0, maxMaterialNo);
    for (int i = 0; i < m.indexCount; ++i) {
      const int vertexNo = indices[rangeFirst + i];
      materialGroups[vertexNo].x = n;
    }
    rangeFirst += m.indexCount;
  }

  // グループ番号を設定
  rangeFirst = 0;
  for (int groupNo = 0; groupNo < mesh.groups.size(); ++groupNo) {
    const Mesh::Group& g = mesh.groups[groupNo];
    for (int i = 0; i < g.indexCount; ++i) {
      const int vertexNo = indices[rangeFirst + i];
      materialGroups[vertexNo].y = groupNo;
    }
    rangeFirst += g.indexCount;
  }

  // 頂点データとインデックスデータをGPUメモリにコピーする.
  const bool result = Add(positions.size(), positions.data(), colors.data(),
    texcoords.data(), normals.data(), materialGroups.data(),
    indices.size(), indices.data(), filename);
  if (result) {
    std::cout << "[情報]" << __func__ << ":" << filename << "(頂点数=" <<
      positions.size() << " インデックス数=" << indices.size() << ")\n";
  } else {
    std::cerr << "[エラー]" << __func__ << ":" << filename << "の読み込みに失敗.\n";
  }

  // メッシュを追加する
  mesh.primitive = primitives.back();
  meshes.push_back(std::make_shared<Mesh>(mesh));

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
* プリミティブを取得する.
*
* @param name プリミティブの名前
*
* @return 名前がnameと一致するプリミティブ.
*/
const Primitive& PrimitiveBuffer::Find(const char* name) const
{
  for (int i = 0; i < primitives.size(); ++i) {
    if (primitives[i].GetName() == name) {
      // 名前が一致する描画データを見つけた(見つけた描画データを返す)
      return primitives[i];
    }
  }

  // 名前が一致する描画データは見つからなかった(仮の描画データを返す)
  static const Primitive dummy;
  return dummy;
}

/**
* メッシュを取得する
*/
const MeshPtr& PrimitiveBuffer::GetMesh(const char* name) const
{
  for (size_t i = 0; i < meshes.size(); ++i) {
    if (meshes[i]->primitive.GetName() == name) {
      return meshes[i];
    }
  }
  static const MeshPtr dummy;
  return dummy;
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

