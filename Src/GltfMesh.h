/**
* @file GltfMesh.h
*/
#ifndef GLTFMESH_H_INCLUDED
#define GLTFMESH_H_INCLUDED
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

// 先行宣言
class Texture;
using TexturePtr = std::shared_ptr<Texture>;
class VertexArrayObject;
using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;

/**
* マテリアル
*/
struct GltfMaterial
{
  glm::vec4 baseColor = glm::vec4(1);
  TexturePtr texBaseColor;
};

/**
* プリミティブデータ
*/
struct GltfPrimitive
{
  GLenum mode = GL_TRIANGLES; // プリミティブの種類
  GLsizei count = 0; // 描画するインデックス数
  GLenum type = GL_UNSIGNED_SHORT; // インデックスデータ型
  const GLvoid* indices = 0; // 描画開始インデックスのバイトオフセット
  GLint baseVertex = 0; // インデックス0番とみなされる頂点配列内の位置

  VertexArrayObjectPtr vao;
  size_t materialNo = 0; // マテリアル番号
};

/**
* メッシュデータ
*/
struct GltfMesh
{
  std::string name; // メッシュ名
  std::vector<GltfPrimitive> primitives;
};

/**
* ファイル
*/
struct GltfFile
{
  std::string name; // ファイル名
  std::vector<GltfMesh> meshes;
  std::vector<GltfMaterial> materials;
};
using GltfFilePtr = std::shared_ptr<GltfFile>;

/**
* メッシュを管理するクラス
*/
class GltfFileBuffer
{
public:
  explicit GltfFileBuffer(size_t maxBufferSize);
  ~GltfFileBuffer();
  GltfFileBuffer(const GltfFileBuffer&) = delete;
  GltfFileBuffer& operator=(const GltfFileBuffer&) = delete;

  bool AddFromFile(const char* filename);
  GltfFilePtr GetFile(const char* filename) const;

private:
  // glTF用のバッファオブジェクト
  GLuint buffer = 0;
  GLsizei maxBufferSize = 0;
  GLsizei curBufferSize = 0;
  
  // メッシュファイル管理用の連想配列
  std::unordered_map<std::string, GltfFilePtr> files;
};
using GltfFileBufferPtr = std::shared_ptr<GltfFileBuffer>;

#endif// GLTFMESH_H_INCLUDED
