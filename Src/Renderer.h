/**
* @file Renderer.h
*/
#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED
#include "glad/glad.h"
#include "Primitive.h"
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
using RendererPtr = std::shared_ptr<Renderer>;
using PrimitiveRendererPtr = std::shared_ptr<PrimitiveRenderer>;
using MeshRendererPtr = std::shared_ptr<MeshRenderer>;

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

  // TODO: �e�L�X�g���ǉ�
  // �O���[�v�s���ݒ肷��
  void SetGroupMatrix(size_t i, const glm::mat4& m) { matGroupModels[i] = m; }

  // �O���[�v�s����擾����
  const glm::mat4& GetGroupMatrix(size_t i) const { return matGroupModels[i]; }

  // �O���[�v�����擾����
  size_t GetGroupCount() const { return matGroupModels.size(); }

private:
  void CalcNthGroupMatrix(int n, std::vector<glm::mat4>& m,
    std::vector<bool>& b) const;
  std::vector<glm::mat4> CalcGroupMatirices() const;

  MeshPtr mesh;
  std::vector<Mesh::Material> materials;

  bool materialChanged = true;
  TextureList textures;
  TextureIndexList textureIndices;
  std::vector<glm::vec4> colors;

  // TODO: �e�L�X�g���ǉ�
  std::vector<glm::mat4> matGroupModels;
};

#endif // RENDERER_H_INCLUDED
