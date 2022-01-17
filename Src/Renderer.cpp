/**
* @file Renderer.cpp
*/
#include "Renderer.h"
#include "ProgramPipeline.h"
#include "Texture.h"
#include "Actor.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* クローンを作成する
*/
RendererPtr PrimitiveRenderer::Clone() const
{
  return std::make_shared<PrimitiveRenderer>(*this);
}

/**
* プリミティブを描画する
*/
void PrimitiveRenderer::Draw(const Actor& actor,
  const ProgramPipeline& pipeline, const glm::mat4& matVP)
{
  // モデル行列を計算する
  glm::mat4 matT = glm::translate(glm::mat4(1), actor.position);
  glm::mat4 matR = glm::rotate(glm::mat4(1), actor.rotation, glm::vec3(0, 1, 0));
  glm::mat4 matS = glm::scale(glm::mat4(1), actor.scale);
  glm::mat4 matA = glm::translate(glm::mat4(1), actor.adjustment);
  glm::mat4 matModel = matT * matR * matS * matA;

  // MVP行列を計算する
  glm::mat4 matMVP = matVP * matModel;

  // モデル行列とMVP行列をGPUメモリにコピーする
  pipeline.SetUniform(locMatTRS, matMVP);
  if (actor.layer == Layer::Default) {
    pipeline.SetUniform(locMatModel, matModel);

    // マテリアルデータを設定
    const glm::uint texture = 0;
    pipeline.SetUniform(locMaterialColor, glm::vec4(1));
    pipeline.SetUniform(locMaterialTexture, &texture, 1);

    // グループ行列を設定
    constexpr glm::mat4 m[32] = {
      glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1),
      glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1),
      glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1),
      glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1),
    };
    pipeline.SetUniform(locMatGroupModels, m, 32);
  }

  // TODO: テキスト未追加
  const GLint locColor = 200;
  pipeline.SetUniform(locColor, actor.color);

  if (tex) {
    tex->Bind(0); // テクスチャを割り当てる
  }
  prim.Draw();  // プリミティブを描画する
}

/**
* クローンを作成する
*/
RendererPtr MeshRenderer::Clone() const
{
  return std::make_shared<MeshRenderer>(*this);
}

/**
* メッシュを描画する
*/
void MeshRenderer::Draw(const Actor& actor,
  const ProgramPipeline& pipeline, const glm::mat4& matVP)
{
  if (!mesh) {
    return;
  }

  // モデル行列を計算する
  glm::mat4 matT = glm::translate(glm::mat4(1), actor.position);
  glm::mat4 matR = glm::rotate(glm::mat4(1), actor.rotation, glm::vec3(0, 1, 0));
  glm::mat4 matS = glm::scale(glm::mat4(1), actor.scale);
  glm::mat4 matA = glm::translate(glm::mat4(1), actor.adjustment);
  glm::mat4 matModel = matT * matR * matS * matA;

  // MVP行列を計算する
  glm::mat4 matMVP = matVP * matModel;

  // GPUメモリに送るためのマテリアルデータを更新
  if (materialChanged) {
    materialChanged = false;
    colors.resize(materials.size());
    for (int i = 0; i < materials.size(); ++i) {
      colors[i] = materials[i].color;
    }
    textures = GetTextureList(materials);
    textureIndices = GetTextureIndexList(materials, textures);
  }

  // モデル行列とMVP行列をGPUメモリにコピーする
  pipeline.SetUniform(locMatTRS, matMVP);
  if (actor.layer == Layer::Default) {
    pipeline.SetUniform(locMatModel, matModel);

    // マテリアルデータを設定
    pipeline.SetUniform(locMaterialColor, colors.data(), colors.size());
    pipeline.SetUniform(locMaterialTexture,
      textureIndices.data(), textureIndices.size());

    // TODO: テキスト未追加
    // グループ行列を設定
    const std::vector<glm::mat4> m = CalcGroupMatirices();
    pipeline.SetUniform(locMatGroupModels, m.data(), m.size());
  }

  // TODO: テキスト未追加
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
* メッシュを設定する
*/
void MeshRenderer::SetMesh(const MeshPtr& p)
{
  mesh = p;
  if (mesh) {
    materials = mesh->materials;
    // マテリアルが存在しないメッシュの場合、マテリアルが最低1つある状態にする
    if (materials.empty()) {
      materials.push_back(Mesh::Material{});
    }
    matGroupModels.resize(mesh->groups.size(), glm::mat4(1));
    matGroupModels.shrink_to_fit();
    materialChanged = true;
  }
}

/**
* 親子関係を考慮してN番目のグループ行列を計算する
*/
void MeshRenderer::CalcNthGroupMatrix(int n,
  std::vector<glm::mat4>& m, std::vector<bool>& calculated) const
{
  // 計算済みなら何もしない
  if (calculated[n]) {
    return;
  }

  // N番目の行列を設定
  m[n] = mesh->groups[n].matBindPose *  // 本来の位置に戻す
    matGroupModels[n] *                 // 座標変換を行う
    mesh->groups[n].matInverseBindPose; // 原点に移動させる

  // 親がいる場合、親のグループ行列を乗算
  const int parent = mesh->groups[n].parent;
  if (parent != Mesh::Group::noParent) {
    CalcNthGroupMatrix(parent, m, calculated);
    m[n] = m[parent] * m[n];
  }

  // 計算が完了したので計算済みフラグを立てる
  calculated[n] = true;
}

/**
* 親子関係を考慮してすべてのグループ行列を計算する
*/
std::vector<glm::mat4> MeshRenderer::CalcGroupMatirices() const 
{
  // メッシュが設定されていなければ空の配列を返す
  if (!mesh) {
    return std::vector<glm::mat4>();
  }

  // グループ行列を計算する
  std::vector<glm::mat4> m(mesh->groups.size());
  std::vector<bool> calculated(m.size(), false);
  for (int n = 0; n < m.size(); ++n) {
    CalcNthGroupMatrix(n, m, calculated);
  }
  return m;
}

