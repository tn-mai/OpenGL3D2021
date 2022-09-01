/**
* @file Renderer.h
*/
#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED
#include "glad/glad.h"
#include "Primitive.h"
#include "ShaderStorageBuffer.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

// 先行宣言
class ProgramPipeline;
class Actor;
class Texture;
class Renderer;
class PrimitiveRenderer;
class MeshRenderer;
class InstancedMeshRenderer;
using RendererPtr = std::shared_ptr<Renderer>;
using PrimitiveRendererPtr = std::shared_ptr<PrimitiveRenderer>;
using MeshRendererPtr = std::shared_ptr<MeshRenderer>;
using InstancedMeshRendererPtr = std::shared_ptr<InstancedMeshRenderer>;
using ActorPtr = std::shared_ptr<Actor>;

struct GltfAnimation;
using GltfAnimationPtr = std::shared_ptr<GltfAnimation>;
struct GltfScene;
struct GltfFile;
using GltfFilePtr = std::shared_ptr<GltfFile>;
class GltfFileBuffer;
using GltfFileBufferPtr = std::shared_ptr<GltfFileBuffer>;

/**
* 描画機能の基本クラス
*/
class Renderer
{
public:
  static const GLint locMatTRS = 0;
  static const GLint locMatModel = 1;
  static const GLint locMaterialColor = 10;
  static const GLint locMaterialParameters = 20;
  static const GLint locMatGroupModels = 30;
  static const GLint locMatShadow = 100;
  static const GLint locRadiusScaleIntensity = 100;
  static const GLint locCameraPosition = 101;
  static const GLint locMapSize = 101;
  static const GLint locMatInvProj = 101;
  static const GLint locCamera = 102;

  static const GLint locDirectionalLight = 110;
  static const GLint locAmbientLight = 112;

  // テキスト未実装
  static const GLint locColor = 200;

  static constexpr GLuint colorBindingPoints[] = { 0, 2, 3, 4, 5, 6, 7, 8 };
  static constexpr size_t colorBindingPointSize = std::size(colorBindingPoints);
  static constexpr GLuint shadowBindingPoint = 1;
  static constexpr GLuint normalBindingPoints[] = { 9, 10, 11, 12, 13, 14, 15, 16 };
  static constexpr size_t normalBindingPointSize = std::size(normalBindingPoints);

  Renderer() = default;
  virtual ~Renderer() = default;
  virtual RendererPtr Clone() const = 0;
  virtual void Update(Actor& actor, float deltaTime) {}
  virtual void PreDraw(const Actor& actor) {}
  virtual void Draw(const Actor& actor,
    const ProgramPipeline& pipeline, const glm::mat4& matVP) = 0;
};

/**
* プリミティブ描画クラス
*/
class PrimitiveRenderer : public Renderer
{
public:
  PrimitiveRenderer() = default;
  virtual ~PrimitiveRenderer() = default;
  virtual RendererPtr Clone() const override;
  virtual void Draw(const Actor& actor,
    const ProgramPipeline& pipeline,
    const glm::mat4& matVP) override;

  void SetPrimitive(const Primitive& p) { prim = p; }
  const Primitive& GetPrimitive() const { return prim; }

  void SetTexture(const std::shared_ptr<Texture>& t) { tex = t; }
  std::shared_ptr<Texture> GetTexture() const { return tex; }

private:
  Primitive prim;
  std::shared_ptr<Texture> tex;
};

/**
* メッシュ描画クラス
*/
class MeshRenderer : public Renderer
{
public:
  MeshRenderer() = default;
  virtual ~MeshRenderer() = default;
  virtual RendererPtr Clone() const override;
  virtual void Draw(const Actor& actor,
    const ProgramPipeline& pipeline,
    const glm::mat4& matVP) override;

  void SetMesh(const MeshPtr& p);
  const MeshPtr& GetMesh() const { return mesh; }

  void SetMaterial(size_t i, const Mesh::Material& m) {
    materials[i] = m;
    materialChanged = true;
  }
  const Mesh::Material& GetMaterial(size_t i) const { return materials[i]; }
  size_t GetMaterialCount() const { return materials.size(); }

  // グループ行列を設定する(2021_21で追加)
  void SetGroupMatrix(size_t i, const glm::mat4& m) { matGroupModels[i] = m; }

  // グループ行列を取得する(2021_21で追加)
  const glm::mat4& GetGroupMatrix(size_t i) const { return matGroupModels[i]; }

  // グループ数を取得する(2021_21で追加)
  size_t GetGroupCount() const { return matGroupModels.size(); }

protected:
  void CalcNthGroupMatrix(int n, std::vector<glm::mat4>& m,
    std::vector<bool>& b) const;
  std::vector<glm::mat4> CalcGroupMatirices() const;

  MeshPtr mesh;
  std::vector<Mesh::Material> materials;

  bool materialChanged = true;
  TextureList textures;
  TextureList texNormals;
  MaterialParameterList materialParameters;
  std::vector<glm::vec4> colors;
  std::vector<glm::mat4> matGroupModels; // グループ用の座標変換行列(2021_21で追加)
};

/**
* インスタンス化メッシュ描画クラス
*/
class InstancedMeshRenderer : public MeshRenderer
{
public:
  struct InstanceData
  {
    glm::mat4 matModel;
    glm::vec4 color;
  };

  InstancedMeshRenderer(size_t instanceCount);
  virtual ~InstancedMeshRenderer() = default;
  virtual RendererPtr Clone() const override;
  virtual void Draw(const Actor& actor,
    const ProgramPipeline& pipeline,
    const glm::mat4& matVP) override;

  size_t AddInstance(const ActorPtr& actor) {
    instances.push_back(actor);
    return instances.size() - 1;
  }
  size_t GetInstanceCount() const { return instances.size(); }
  const ActorPtr& GetInstance(size_t index) const { return instances[index]; }
  void RemoveInstance(size_t index) { instances.erase(instances.begin() + index); }
  void UpdateInstanceTransforms();
  void UpdateInstanceData(size_t size, const InstanceData* data);

private:
  std::vector<ActorPtr> instances;
  size_t latestInstanceSize = 0; // 最終更新時のインスタンス数
  ShaderStorageBufferPtr ssbo;
};

/**
* スタティックメッシュ描画クラス
*/
class StaticMeshRenderer : public Renderer
{
public:
  StaticMeshRenderer() = default;
  virtual ~StaticMeshRenderer() = default;
  virtual RendererPtr Clone() const override;
  virtual void Draw(const Actor& actor,
    const ProgramPipeline& pipeline,
    const glm::mat4& matVP) override;

  void SetMesh(const GltfFilePtr& p, int index) { file = p; meshIndex = index; }
  const GltfFilePtr& GetFile() const { return file; }
  int GetMeshIndex() const { return meshIndex; }

private:
  GltfFilePtr file;
  int meshIndex = -1;
};
using StaticMeshRendererPtr = std::shared_ptr<StaticMeshRenderer>;

/**
* アニメーションするメッシュ描画クラス
*/
class AnimatedMeshRenderer : public Renderer
{
public:
  // アニメーションの再生状態
  enum class State {
    stop, ///< 停止中
    play, ///< 再生中
    pause, ///< 一時停止中
  };

  AnimatedMeshRenderer() = default;
  virtual ~AnimatedMeshRenderer() = default;
  virtual RendererPtr Clone() const override;
  virtual void Update(Actor& actor, float deltaTime) override;
  virtual void PreDraw(const Actor& actor) override;
  virtual void Draw(const Actor& actor,
    const ProgramPipeline& pipeline,
    const glm::mat4& matVP) override;

  void SetFileBuffer(const GltfFileBufferPtr& p) { fileBuffer = p; }
  void SetFile(const GltfFilePtr& f, int sceneNo = 0);
  bool SetAnimation(const GltfAnimationPtr& animation, bool isLoop = true);
  bool SetAnimation(const std::string& name, bool isLoop = true);
  bool SetAnimation(size_t index, bool isLoop = true);
  bool Play();
  bool Stop();
  bool Pause();

  const GltfFilePtr& GetFile() const { return file; }
  State GetState() const { return state; }
  const GltfAnimationPtr& GetAnimation() const { return animation; }

  // @note 再生速度(1.0f=等速, 2.0f=2倍速, 0.5f=1/2倍速)
  void SetAnimationSpeed(float speed) { animationSpeed = speed; }
  float GetAnimationSpeed() const { return animationSpeed; }

  // @note true=ループする, false=ループしない
  void SetLoopFlag(bool isLoop) { this->isLoop = isLoop; }
  bool IsLoop() const { return isLoop; }

  void SetPosition(float);
  float GetPosition() const;
  bool IsFinished() const;

private:
  GltfFileBufferPtr fileBuffer;
  GltfFilePtr file;
  const GltfScene* scene = nullptr;
  GltfAnimationPtr animation;
  std::vector<int> nonAnimatedNodes;

  struct Range {
    GLintptr offset;
    GLsizeiptr size;
  };
  std::vector<Range> ssboRangeList;

  State state = State::stop;
  float time = 0;
  float animationSpeed = 1;
  bool isLoop = true;
};
using AnimatedMeshRendererPtr = std::shared_ptr<AnimatedMeshRenderer>;

StaticMeshRendererPtr SetStaticMeshRenderer(
  Actor& actor, const char* filename, int index = 0);

AnimatedMeshRendererPtr SetAnimatedMeshRenderer(
  Actor& actor, const char* filename, int sceneNo = 0);

#endif // RENDERER_H_INCLUDED
