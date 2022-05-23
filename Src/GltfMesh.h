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
class ShaderStorageBuffer;
using ShaderStorageBufferPtr = std::shared_ptr<ShaderStorageBuffer>;

/**
* アニメーションの補間方法
*/
enum class GltfInterpolation
{
  step,        // 補間なし
  linear,      // 線形補間
  cubicSpline, // 3次スプライン補間
};

/**
* アニメーションのキーフレーム
*/
template<typename T>
struct GltfKeyframe
{
  float time; // 時間
  T value;    // 適用する値
};

/**
* アニメーションのチャネル
*/
template<typename T>
struct GltfChannel
{
  int targetNodeId;                       // 値を適用するノードID
  GltfInterpolation interpolation;        // 補間方法
  std::vector<GltfKeyframe<T>> keyframes; // キーフレーム配列
};

/**
* アニメーション
*/
struct GltfAnimation
{
  std::string name; // アニメーション名
  std::vector<GltfChannel<glm::vec3>> translations; // 平行移動チャネルの配列
  std::vector<GltfChannel<glm::quat>> rotations;    // 回転チャネルの配列
  std::vector<GltfChannel<glm::vec3>> scales;       // 拡大縮小チャネルの配列
  float totalTime = 0;
};
using GltfAnimationPtr = std::shared_ptr<GltfAnimation>;

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
* スキン
*/
struct GltfSkin
{
  std::string name; // スキン名

  struct Joint {
    int nodeId;
    glm::mat4 matInverseBindPose;
  };
  std::vector<Joint> joints;
};

/**
* ノード
*/
struct GltfNode
{
  std::string name; // ノード名
  int mesh = -1;    // メッシュ番号
  int skin = -1;    // スキン番号
  GltfNode* parent = nullptr;         // 親ノード
  std::vector<GltfNode*> children;    // 子ノード配列
  glm::mat4 matLocal = glm::mat4(1);  // ローカル行列
  glm::mat4 matGlobal = glm::mat4(1); // グローバル行列
};

/**
* シーン
*/
struct GltfScene
{
  std::vector<const GltfNode*> nodes;
  std::vector<const GltfNode*> meshNodes;
};

/**
* ファイル
*/
struct GltfFile
{
  std::string name; // ファイル名
  std::vector<GltfScene> scenes;
  std::vector<GltfNode> nodes;
  std::vector<GltfSkin> skins;
  std::vector<GltfMesh> meshes;
  std::vector<GltfMaterial> materials;
  std::vector<GltfAnimationPtr> animations;
};
using GltfFilePtr = std::shared_ptr<GltfFile>;

// アニメーション用座標変換行列の配列
using GltfAnimationMatrices = std::vector<glm::mat4>;

GltfAnimationMatrices CalcAnimationMatrices(const GltfFilePtr& file,
  const GltfNode* meshNode, const GltfAnimation* animation,
  const std::vector<int>& nonAnimatedNodes, float time);

/**
* メッシュを管理するクラス
*/
class GltfFileBuffer
{
public:
  GltfFileBuffer(size_t maxBufferSize, size_t maxMatrixSize);
  ~GltfFileBuffer();

  // コピーを禁止
  GltfFileBuffer(const GltfFileBuffer&) = delete;
  GltfFileBuffer& operator=(const GltfFileBuffer&) = delete;

  bool AddFromFile(const char* filename);
  GltfFilePtr GetFile(const char* filename) const;

  void ClearAnimationBuffer();
  GLintptr AddAnimationMatrices(const GltfAnimationMatrices& matBones);
  void UploadAnimationBuffer();
  void BindAnimationBuffer(GLuint bindingPoint, GLintptr offset, GLsizeiptr size);
  void UnbindAnimationBuffer(GLuint bindingPoint);

private:
  // glTF用のバッファオブジェクト
  GLuint buffer = 0;
  GLsizei maxBufferSize = 0;
  GLsizei curBufferSize = 0;
  
  // メッシュファイル管理用の連想配列
  std::unordered_map<std::string, GltfFilePtr> files;

  // アニメーション行列用バッファ
  ShaderStorageBufferPtr ssbo;
  GltfAnimationMatrices matrixBuffer;
};
using GltfFileBufferPtr = std::shared_ptr<GltfFileBuffer>;

#endif// GLTFMESH_H_INCLUDED
