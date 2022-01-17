/**
* @file Renderer.cpp
*/
#include "Renderer.h"
#include "ProgramPipeline.h"
#include "Texture.h"
#include "Actor.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* �N���[�����쐬����
*/
RendererPtr PrimitiveRenderer::Clone() const
{
  return std::make_shared<PrimitiveRenderer>(*this);
}

/**
* �v���~�e�B�u��`�悷��
*/
void PrimitiveRenderer::Draw(const Actor& actor,
  const ProgramPipeline& pipeline, const glm::mat4& matVP)
{
  // ���f���s����v�Z����
  glm::mat4 matT = glm::translate(glm::mat4(1), actor.position);
  glm::mat4 matR = glm::rotate(glm::mat4(1), actor.rotation, glm::vec3(0, 1, 0));
  glm::mat4 matS = glm::scale(glm::mat4(1), actor.scale);
  glm::mat4 matA = glm::translate(glm::mat4(1), actor.adjustment);
  glm::mat4 matModel = matT * matR * matS * matA;

  // MVP�s����v�Z����
  glm::mat4 matMVP = matVP * matModel;

  // ���f���s���MVP�s���GPU�������ɃR�s�[����
  pipeline.SetUniform(locMatTRS, matMVP);
  if (actor.layer == Layer::Default) {
    pipeline.SetUniform(locMatModel, matModel);

    // �}�e���A���f�[�^��ݒ�
    const glm::uint texture = 0;
    pipeline.SetUniform(locMaterialColor, glm::vec4(1));
    pipeline.SetUniform(locMaterialTexture, &texture, 1);

    // �O���[�v�s���ݒ�
    constexpr glm::mat4 m[32] = {
      glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1),
      glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1),
      glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1),
      glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1),
    };
    pipeline.SetUniform(locMatGroupModels, m, 32);
  }

  // TODO: �e�L�X�g���ǉ�
  const GLint locColor = 200;
  pipeline.SetUniform(locColor, actor.color);

  if (tex) {
    tex->Bind(0); // �e�N�X�`�������蓖�Ă�
  }
  prim.Draw();  // �v���~�e�B�u��`�悷��
}

/**
* �N���[�����쐬����
*/
RendererPtr MeshRenderer::Clone() const
{
  return std::make_shared<MeshRenderer>(*this);
}

/**
* ���b�V����`�悷��
*/
void MeshRenderer::Draw(const Actor& actor,
  const ProgramPipeline& pipeline, const glm::mat4& matVP)
{
  if (!mesh) {
    return;
  }

  // ���f���s����v�Z����
  glm::mat4 matT = glm::translate(glm::mat4(1), actor.position);
  glm::mat4 matR = glm::rotate(glm::mat4(1), actor.rotation, glm::vec3(0, 1, 0));
  glm::mat4 matS = glm::scale(glm::mat4(1), actor.scale);
  glm::mat4 matA = glm::translate(glm::mat4(1), actor.adjustment);
  glm::mat4 matModel = matT * matR * matS * matA;

  // MVP�s����v�Z����
  glm::mat4 matMVP = matVP * matModel;

  // GPU�������ɑ��邽�߂̃}�e���A���f�[�^���X�V
  if (materialChanged) {
    materialChanged = false;
    colors.resize(materials.size());
    for (int i = 0; i < materials.size(); ++i) {
      colors[i] = materials[i].color;
    }
    textures = GetTextureList(materials);
    textureIndices = GetTextureIndexList(materials, textures);
  }

  // ���f���s���MVP�s���GPU�������ɃR�s�[����
  pipeline.SetUniform(locMatTRS, matMVP);
  if (actor.layer == Layer::Default) {
    pipeline.SetUniform(locMatModel, matModel);

    // �}�e���A���f�[�^��ݒ�
    pipeline.SetUniform(locMaterialColor, colors.data(), colors.size());
    pipeline.SetUniform(locMaterialTexture,
      textureIndices.data(), textureIndices.size());

    // TODO: �e�L�X�g���ǉ�
    // �O���[�v�s���ݒ�
    const std::vector<glm::mat4> m = CalcGroupMatirices();
    pipeline.SetUniform(locMatGroupModels, m.data(), m.size());
  }

  // TODO: �e�L�X�g���ǉ�
  const GLint locColor = 200;
  pipeline.SetUniform(locColor, actor.color);

  const GLuint bindingPoints[] = { 0, 2, 3, 4, 5, 6, 7, 8 };
  const size_t size = std::min(textures.size(), std::size(bindingPoints));
  for (int i = 0; i < size; ++i) {
    textures[i]->Bind(bindingPoints[i]);
  }
  mesh->primitive.Draw();
}

/**
* ���b�V����ݒ肷��
*/
void MeshRenderer::SetMesh(const MeshPtr& p)
{
  mesh = p;
  if (mesh) {
    materials = mesh->materials;
    // �}�e���A�������݂��Ȃ����b�V���̏ꍇ�A�}�e���A�����Œ�1�����Ԃɂ���
    if (materials.empty()) {
      materials.push_back(Mesh::Material{});
    }
    matGroupModels.resize(mesh->groups.size(), glm::mat4(1));
    matGroupModels.shrink_to_fit();
    materialChanged = true;
  }
}

/**
* �e�q�֌W���l������N�Ԗڂ̃O���[�v�s����v�Z����
*/
void MeshRenderer::CalcNthGroupMatrix(int n,
  std::vector<glm::mat4>& m, std::vector<bool>& calculated) const
{
  // �v�Z�ς݂Ȃ牽�����Ȃ�
  if (calculated[n]) {
    return;
  }

  // N�Ԗڂ̍s���ݒ�
  m[n] = mesh->groups[n].matBindPose *  // �{���̈ʒu�ɖ߂�
    matGroupModels[n] *                 // ���W�ϊ����s��
    mesh->groups[n].matInverseBindPose; // ���_�Ɉړ�������

  // �e������ꍇ�A�e�̃O���[�v�s�����Z
  const int parent = mesh->groups[n].parent;
  if (parent != Mesh::Group::noParent) {
    CalcNthGroupMatrix(parent, m, calculated);
    m[n] = m[parent] * m[n];
  }

  // �v�Z�����������̂Ōv�Z�ς݃t���O�𗧂Ă�
  calculated[n] = true;
}

/**
* �e�q�֌W���l�����Ă��ׂẴO���[�v�s����v�Z����
*/
std::vector<glm::mat4> MeshRenderer::CalcGroupMatirices() const 
{
  // ���b�V�����ݒ肳��Ă��Ȃ���΋�̔z���Ԃ�
  if (!mesh) {
    return std::vector<glm::mat4>();
  }

  // �O���[�v�s����v�Z����
  std::vector<glm::mat4> m(mesh->groups.size());
  std::vector<bool> calculated(m.size(), false);
  for (int n = 0; n < m.size(); ++n) {
    CalcNthGroupMatrix(n, m, calculated);
  }
  return m;
}

