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

// ��s�錾
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

/**
* �`��@�\�̊�{�N���X
*/
class Renderer
{
public:
  static const GLint locMatTRS = 0;
  static const GLint locMatModel = 1;
  static const GLint locMaterialColor = 10;
  static const GLint locMaterialTexture = 20;
  static const GLint locMatGroupModels = 30;
  static const GLint locMatShadow = 100;
  static const GLint locMapSize = 101;
  static const GLint locCamera = 102;
  static const GLint locColor = 200;

  Renderer() = default;
  virtual ~Renderer() = default;
  virtual RendererPtr Clone() const = 0;
  virtual void Draw(const Actor& actor,
    const ProgramPipeline& pipeline, const glm::mat4& matVP) = 0;
};

/**
* �v���~�e�B�u�`��N���X
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
* ���b�V���`��N���X
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

  // �O���[�v�s���ݒ肷��(2021_21�Œǉ�)
  void SetGroupMatrix(size_t i, const glm::mat4& m) { matGroupModels[i] = m; }

  // �O���[�v�s����擾����(2021_21�Œǉ�)
  const glm::mat4& GetGroupMatrix(size_t i) const { return matGroupModels[i]; }

  // �O���[�v�����擾����(2021_21�Œǉ�)
  size_t GetGroupCount() const { return matGroupModels.size(); }

protected:
  void CalcNthGroupMatrix(int n, std::vector<glm::mat4>& m,
    std::vector<bool>& b) const;
  std::vector<glm::mat4> CalcGroupMatirices() const;

  MeshPtr mesh;
  std::vector<Mesh::Material> materials;

  bool materialChanged = true;
  TextureList textures;
  TextureIndexList textureIndices;
  std::vector<glm::vec4> colors;
  std::vector<glm::mat4> matGroupModels; // �O���[�v�p�̍��W�ϊ��s��(2021_21�Œǉ�)
};

/**
* �C���X�^���X�����b�V���`��N���X
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
  size_t latestInstanceSize = 0; // �ŏI�X�V���̃C���X�^���X��
  ShaderStorageBufferPtr ssbo;
};

#endif // RENDERER_H_INCLUDED
