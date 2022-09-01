/**
* @file Primitive.h
**/
#ifndef PRIMITIVE_H_INCLUDED
#define PRIMITIVE_H_INCLUDED
#include <glad/glad.h>
#include "Texture.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>

/**
* プリミティブデータ.
*/
class Primitive
{
public:
  Primitive() = default;
  Primitive(const char* name, GLenum m, GLsizei c, size_t o, GLint b) :
    name(name),
    mode(m), count(c), indices(reinterpret_cast<GLvoid*>(o)), baseVertex(b)
  {}
  ~Primitive() = default;

  void Draw() const;
  void DrawInstanced(size_t instanceCount) const;
  const std::string& GetName() const { return name; }

private:
  std::string name;
  GLenum mode = GL_TRIANGLES; ///< プリミティブの種類.
  GLsizei count = 0; ///< 描画するインデックス数.
  const GLvoid* indices = 0; ///< 描画開始インデックスのバイトオフセット.
  GLint baseVertex = 0; ///< インデックス0番とみなされる頂点配列内の位置.
};

/**
* プリミティブ描画データ
*/
class Mesh
{
public:
  /**
  * マテリアル(材質)データ
  */
  struct Material
  {
    std::string name;               // マテリアル名
    glm::vec4 color = glm::vec4(1); // ディフューズ色
    std::shared_ptr<Texture> tex;   // テクスチャ
    std::shared_ptr<Texture> texNormal; // 法線テクスチャ
    float roughness = 0.5f;         // 表面の粗さ

    float metalness = 0;    // 非金属=0, 金属=1
  };

  /**
  * マテリアルの割り当て範囲
  */
  struct UseMaterial
  {
    int materialNo = -1; // 割り当てるマテリアルの番号
    GLsizei indexCount = 0; // マテリアルを割り当てるインデックスデータの数
  };

  /**
  * ポリゴンをグループ化するデータ
  */
  struct Group
  {
    std::string name; // グループ名
    GLsizei indexCount = 0; // グループに含まれるインデックスデータの数

    // 21で実装. 21bは未実装.
    static const int noParent = -1; // 親がいないことを示す定数
    int parent = noParent; // 親グループ番号
    glm::mat4 matBindPose = glm::mat4(1);
    glm::mat4 matInverseBindPose = glm::mat4(1);
  };

  Mesh() = default;
  ~Mesh() = default;
  Mesh(const Mesh&) = default;
  Mesh& operator=(const Mesh&) = default;

  Primitive primitive;
  std::vector<Material> materials;
  std::vector<UseMaterial> useMaterials;
  std::vector<Group> groups;
};
using MeshPtr = std::shared_ptr<Mesh>;

/**
* 複数のプリミティブを管理するクラス.
*/
class PrimitiveBuffer
{
public:
  PrimitiveBuffer(GLsizei maxVertexCount, GLsizei maxIndexCount);
  ~PrimitiveBuffer();

  // プリミティブの追加(20にてpGroupAndMaterial引数を追加)
  bool Add(size_t vertexCount, const glm::vec3* pPosition, const glm::vec4* pColor,
    const glm::vec2* pTexcoord, const glm::vec3* pNormal,
    const glm::u8vec2* pMaterialGroup,
    size_t indexCount, const GLushort* pIndex, const char* name = nullptr,
    GLenum type = GL_TRIANGLES);
  bool AddFromObjFile(const char* filename);

  // プリミティブの取得.
  const Primitive& Get(size_t n) const;
  const Primitive& Find(const char* name) const;
  const MeshPtr& GetMesh(const char* name) const; // 20で実装. 20bは未実装.

  // VAOバインド管理.
  void BindVertexArray() const;
  void UnbindVertexArray() const;

  // TODO: テキスト未追加
  size_t GetCount() const { return primitives.size(); }

private:
  std::vector<Primitive> primitives;
  std::vector<MeshPtr> meshes;

  // バッファID.
  GLuint vboPosition = 0;
  GLuint vboColor = 0;
  GLuint vboTexcoord = 0;
  GLuint vboNormal = 0;
  GLuint vboMaterialGroup = 0;
  GLuint ibo = 0;
  GLuint vao = 0;

  GLsizei maxVertexCount = 0; // 格納できる最大頂点数.
  GLsizei curVertexCount = 0; // 格納済み頂点数.
  GLsizei maxIndexCount = 0; // 格納できる最大インデックス数.
  GLsizei curIndexCount = 0; // 格納済みインデックス数.
};

bool CopyData(GLuint writeBuffer, GLsizei unitSize,
  GLsizei offsetCount, size_t count, const void* data);

using TextureList = std::vector<std::shared_ptr<Texture>>;
using MaterialParameterList = std::vector<glm::vec4>;
TextureList GetTextureList(const std::vector<Mesh::Material>& materials);
TextureList GetNormalTextureList(const std::vector<Mesh::Material>& materials);
MaterialParameterList GetMaterialParameterList(const std::vector<Mesh::Material>& materials,
  const TextureList& textures, const TextureList& texNormals);

#endif // PRIMITIVE_H_INCLUDED
