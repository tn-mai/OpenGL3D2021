/**
* @file Renderer.cpp
*/
#include "Renderer.h"
#include "ProgramPipeline.h"
#include "Texture.h"
#include "Actor.h"
#include "GltfMesh.h"
#include "VertexArrayObject.h"
#include "GameEngine.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <numeric>
#include <algorithm>

namespace /* unnamed */ {

/**
* アニメーション用の中間データ
*/
struct Transformation
{
  glm::mat4 m = glm::mat4(1);
  bool isCalculated = false;
};
using TransformationList = std::vector<Transformation>;

/**
* ノードのグローバルモデル行列を計算する
*/
const glm::mat4& CalcGlobalTransform(const std::vector<GltfNode>& nodes,
  const GltfNode& node, TransformationList& transList)
{
  const intptr_t currentNodeId = &node - &nodes[0];
  Transformation& trans = transList[currentNodeId];
  if (trans.isCalculated) {
    return trans.m;
  }

  glm::mat4 matParent;
  if (node.parent) {
    matParent = CalcGlobalTransform(nodes, *node.parent, transList);
  } else {
    matParent = glm::mat4(1);
  }
  trans.m = matParent * trans.m;
  trans.isCalculated = true;

  return trans.m;
}

/**
* アニメーション補間された座標変換行列を計算する
*/
TransformationList CalcAnimatedTransformations(const GltfFile& file,
  const GltfAnimation& animation, const std::vector<int>& nonAnimatedNodeList,
  float keyFrame)
{
  TransformationList transList;
  transList.resize(file.nodes.size());
  for (const auto e : nonAnimatedNodeList) {
    transList[e].m = file.nodes[e].matLocal;
  }

  for (const auto& e : animation.translations) {
    auto& trans = transList[e.targetNodeId];
    const glm::vec3 translation = Interporation(e, keyFrame);
    trans.m *= glm::translate(glm::mat4(1), translation);
  }
  for (const auto& e : animation.rotations) {
    auto& trans = transList[e.targetNodeId];
    const glm::quat rotation = Interporation(e, keyFrame);
    trans.m *= glm::mat4_cast(rotation);
  }
  for (const auto& e : animation.scales) {
    auto& trans = transList[e.targetNodeId];
    const glm::vec3 scale = Interporation(e, keyFrame);
    trans.m *= glm::scale(glm::mat4(1), scale);
  }

  for (auto& e : file.nodes) {
    CalcGlobalTransform(file.nodes, e, transList);
  }

  return transList;
}

/**
* アニメーションを適用した座標変換行列リストを計算する
*
* @param file      アニメーションとノードを所有するファイルオブジェクト
* @param node      スキニング対象のノード
* @param animation 計算の元になるアニメーション
* @param time     アニメーションの再生位置
*
* @return アニメーションを適用した座標変換行列リスト
*/
GltfFileBuffer::AnimationMatrices CalculateTransform(const GltfFilePtr& file,
  const GltfNode* meshNode, const GltfAnimation* animation,
  const std::vector<int>& nonAnimatedNodeList, float time)
{
  GltfFileBuffer::AnimationMatrices matBones;
  if (!file || !meshNode) {
    return matBones;
  }

  if (animation) {
    const TransformationList transList = CalcAnimatedTransformations(*file, *animation, nonAnimatedNodeList, time);
    if (meshNode->skin >= 0) {
      // アニメーションあり+スキンあり
      // @note jointsにはノード番号が格納されているが、頂点データのJOINTS_nには
      //       ノード番号ではなく「joints配列のインデックス」が格納されている。
      //       つまり、ボーン行列配列をjointsの順番でSSBOに格納する必要がある。
      const auto& joints = file->skins[meshNode->skin].joints;
      matBones.resize(joints.size());
      for (size_t i = 0; i < joints.size(); ++i) {
        const auto& joint = joints[i];
        matBones[i] = transList[joint.nodeId].m * joint.matInverseBindPose;
      }
    } else {
      // アニメーションあり+スキンなし
      const size_t nodeId = meshNode - &file->nodes[0];
      matBones.push_back(transList[nodeId].m);
    }
  } else {
    // アニメーションなし
    if (meshNode->skin >= 0) {
      // スキンあり
      const auto& joints = file->skins[meshNode->skin].joints;
      matBones.resize(joints.size(), meshNode->matGlobal);
    } else {
      matBones.push_back(meshNode->matGlobal);
    }
  }
  return matBones;
}

} // unnamed namespace

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

/**
* コンストラクタ
*/
InstancedMeshRenderer::InstancedMeshRenderer(size_t instanceCount)
{
  instances.reserve(instanceCount);
  ssbo = std::make_shared<ShaderStorageBuffer>(instanceCount * sizeof(InstanceData));
}

/**
* クローンを作成する
*/
RendererPtr InstancedMeshRenderer::Clone() const
{
  auto clone = std::make_shared<InstancedMeshRenderer>(*this);
  clone->ssbo = std::make_shared<ShaderStorageBuffer>(ssbo->GetSize());
  return clone;
}

/**
* メッシュを描画する
*/
void InstancedMeshRenderer::Draw(const Actor& actor,
  const ProgramPipeline& pipeline,
  const glm::mat4& matVP)
{
  if (!mesh) {
    return;
  }
  if (latestInstanceSize <= 0) {
    return;
  }

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

  // モデル行列をGPUメモリにコピーする
  pipeline.SetUniform(locMatModel, actor.GetModelMatrix());
  if (actor.layer == Layer::Default) {
    // マテリアルデータを設定
    pipeline.SetUniform(locMaterialColor, colors.data(), colors.size());
    pipeline.SetUniform(locMaterialTexture,
      textureIndices.data(), textureIndices.size());
  }

  ssbo->Bind(0);

  // TODO: テキスト未追加
  const GLint locColor = 200;
  pipeline.SetUniform(locColor, actor.color);

  const GLuint bindingPoints[] = { 0, 2, 3, 4, 5, 6, 7, 8 };
  const size_t size = std::min(textures.size(), std::size(bindingPoints));
  for (int i = 0; i < size; ++i) {
    textures[i]->Bind(bindingPoints[i]);
  }
  mesh->primitive.DrawInstanced(latestInstanceSize);

  ssbo->FenceSync();
  ssbo->Unbind(0);
}

/**
* インスタンスのモデル行列を更新する
*/
void InstancedMeshRenderer::UpdateInstanceTransforms()
{
  // 死んでいるインスタンスを削除する
  const auto i = std::remove_if(instances.begin(), instances.end(),
    [](const ActorPtr& e) { return e->isDead; });
  instances.erase(i, instances.end());

  // インスタンス数を計算
  latestInstanceSize = std::min(
    ssbo->GetSize() / sizeof(glm::mat4), instances.size());
  if (latestInstanceSize <= 0) {
    return;
  }

  // モデル行列を計算
  std::vector<InstanceData> transforms(latestInstanceSize);
  for (size_t i = 0; i < latestInstanceSize; ++i) {
    transforms[i].matModel = instances[i]->GetModelMatrix();
    transforms[i].color = instances[i]->color;
  }

  // モデル行列をGPUメモリにコピー
  ssbo->BufferSubData(0, latestInstanceSize * sizeof(InstanceData), transforms.data());
  ssbo->SwapBuffers();
}

/**
* インスタンスのモデル行列を更新する
*/
void InstancedMeshRenderer::UpdateInstanceData(size_t size, const InstanceData* data)
{
  // インスタンス数を計算
  latestInstanceSize = std::min(
    ssbo->GetSize() / sizeof(InstanceData), size);
  if (latestInstanceSize <= 0) {
    return;
  }

  // モデル行列をGPUメモリにコピー
  ssbo->BufferSubData(0, latestInstanceSize * sizeof(InstanceData), data);
  ssbo->SwapBuffers();
}

/**
* クローンを作成する
*/
RendererPtr StaticMeshRenderer::Clone() const
{
  return std::make_shared<StaticMeshRenderer>(*this);
}

/**
* メッシュを描画する
*/
void StaticMeshRenderer::Draw(const Actor& actor,
  const ProgramPipeline& pipeline, const glm::mat4& matVP)
{
  if (!file || meshIndex < 0 || meshIndex >= file->meshes.size()) {
    return;
  }

  // モデル行列をGPUメモリにコピーする
  pipeline.SetUniform(locMatModel, actor.GetModelMatrix());

  for (const auto& prim : file->meshes[meshIndex].primitives) {
    // マテリアルデータを設定
    const GltfMaterial& m = file->materials[prim.materialNo];
    pipeline.SetUniform(locMaterialColor, m.baseColor);
    m.texBaseColor->Bind(0);

    prim.vao->Bind();
    glDrawElementsBaseVertex(prim.mode, prim.count, prim.type,
      prim.indices, prim.baseVertex);
  }
  glBindVertexArray(0);
}

/**
* クローンを作成する
*/
RendererPtr AnimatedMeshRenderer::Clone() const
{
  return std::make_shared<AnimatedMeshRenderer>(*this);
}

/**
* 表示するファイルを設定する
*/
void AnimatedMeshRenderer::SetFile(const GltfFilePtr& f, int sceneNo)
{
  file = f;
  scene = &f->scenes[sceneNo];
  animation = nullptr;
  ssboRangeList.clear();

  state = State::stop;
  time = 0;
  animationSpeed = 1;
  isLoop = true;
}

/**
* アニメーション状態を更新する
*
* @param deltaTime 前回の更新からの経過時間
* @param actor     描画対象のアクター
*/
void AnimatedMeshRenderer::Update(Actor& actor, float deltaTime)
{
  // 再生フレーム更新
  if (animation && state == State::play) {
    time += deltaTime * animationSpeed;
    if (isLoop) {
      if (time >= animation->totalTime) {
        time -= animation->totalTime;
      } else if (time < 0) {
        const float n = std::ceil(-time / animation->totalTime);
        time += animation->totalTime * n;
      }
    } else {
      if (time >= animation->totalTime) {
        time = animation->totalTime;
      } else if (time < 0) {
        time = 0;
      }
    }
  }

  // 状態を更新
  if (animation) {
    switch (state) {
    case State::stop:
      break;
    case State::play:
      if (!isLoop && (time >= animation->totalTime)) {
        state = State::stop;
      }
      break;
    case State::pause:
      break;
    }
  }
}

/**
* 描画の前処理を実行
*/
void AnimatedMeshRenderer::PreDraw(const Actor& actor)
{
  // SSBOにコピーするデータを追加
  const glm::mat4 matModel = actor.GetModelMatrix();
  ssboRangeList.clear();
  for (const auto e : scene->meshNodes) {
    auto matBones = CalculateTransform(file, e, animation.get(), nonAnimatedNodeList, time);
    for (auto& m : matBones) {
      m = matModel * m;
    }
    const GLintptr offset = fileBuffer->AddAnimationData(matBones);
    const GLsizeiptr size = static_cast<GLsizeiptr>(matBones.size() * sizeof(glm::mat4));
    ssboRangeList.push_back({ offset, size });
  }
}

/**
* スケルタルメッシュを描画する
*/
void AnimatedMeshRenderer::Draw(const Actor& actor,
  const ProgramPipeline& pipeline, const glm::mat4& matVP)
{
  if (!file || !scene || ssboRangeList.empty()) {
    return;
  }

  // モデル行列をGPUメモリにコピーする
  //pipeline.SetUniform(locMatModel, actor.GetModelMatrix());

  for (size_t i = 0; i < scene->meshNodes.size(); ++i) {
    const glm::uint meshNo = scene->meshNodes[i]->mesh;
    const GltfMesh& meshData = file->meshes[meshNo];
    fileBuffer->BindAnimationBuffer(0, ssboRangeList[i].offset, ssboRangeList[i].size);
    //pipeline.SetUniform(11, &meshNo, 1);
    for (const auto& prim : meshData.primitives) {
      // マテリアルデータを設定
      const GltfMaterial& m = file->materials[prim.materialNo];
      pipeline.SetUniform(locMaterialColor, m.baseColor);

      // @todo テクスチャがない場合に白いテクスチャを貼り付ける
      if (m.texBaseColor) {
        m.texBaseColor->Bind(0);
      }

      prim.vao->Bind();
      glDrawElementsBaseVertex(prim.mode, prim.count, prim.type,
        prim.indices, prim.baseVertex);
    }
  }
  fileBuffer->UnbindAnimationBuffer(0);
  glBindVertexArray(0);
}

/**
* アニメーションを再生する
*
* @param animation 再生するアニメーション
* @param isLoop    ループ再生の指定(true=ループする false=ループしない)
*
* @retval true  再生開始
* @retval false 再生失敗
*/
bool AnimatedMeshRenderer::Play(const GltfAnimationPtr& animation, bool isLoop)
{
  if (!file) {
    return false;
  }

  if (this->animation != animation) {
    this->animation = animation;
    if (!animation) {
      state = State::stop;
      return false;
    }

    // アニメーションを行わないノードのリストを作る

    const int noAnimation = -1; // 「アニメーションなし」を表す値

    // 全ノード番号のリストを作成
    const size_t size = file->nodes.size();
    nonAnimatedNodeList.resize(size);
    std::iota(nonAnimatedNodeList.begin(), nonAnimatedNodeList.end(), 0);

    // アニメーション対象のノード番号を「アニメーションなし」で置き換える
    for (const auto& e : animation->scales) {
      if (e.targetNodeId < size) {
        nonAnimatedNodeList[e.targetNodeId] = noAnimation;
      }
    }
    for (const auto& e : animation->rotations) {
      if (e.targetNodeId < size) {
        nonAnimatedNodeList[e.targetNodeId] = noAnimation;
      }
    }
    for (const auto& e : animation->translations) {
      if (e.targetNodeId < size) {
        nonAnimatedNodeList[e.targetNodeId] = noAnimation;
      }
    }

    // 「アニメーションなし」をリストから削除する
    const auto itr = std::remove(
      nonAnimatedNodeList.begin(), nonAnimatedNodeList.end(), noAnimation);
    nonAnimatedNodeList.erase(itr, nonAnimatedNodeList.end());
  }

  time = 0;
  state = State::play;
  this->isLoop = isLoop;

  return true;
}

/**
* アニメーションを再生する
*
* @param name   再生するアニメーションの名前
* @param isLoop ループ再生の指定(true=ループする false=ループしない)
*
* @retval true  再生開始
* @retval false 再生失敗
*/
bool AnimatedMeshRenderer::Play(const std::string& name, bool isLoop)
{
  if (!file) {
    return false;
  }

  for (const auto& e : file->animations) {
    if (e->name == name) {
      return Play(e, isLoop);
    }
  }
  return false;
}

/**
* アニメーションを再生する
*
* @param index  再生するアニメーション番号
* @param isLoop ループ再生の指定(true=ループする false=ループしない)
*
* @retval true  再生開始
* @retval false 再生失敗
*/
bool AnimatedMeshRenderer::Play(size_t index, bool isLoop)
{
  if (!file || index >= file->animations.size()) {
    return false;
  }
  return Play(file->animations[index], isLoop);
}
/**
* アニメーションの再生を停止する
*
* @retval true  成功
* @retval false 失敗(アニメーションが設定されていない)
*/
bool AnimatedMeshRenderer::Stop()
{
  if (animation) {
    switch (state) {
    case State::play:  state = State::stop; return true;
    case State::stop:  return true;
    case State::pause: state = State::stop; return true;
    }
  }
  return false;
}

/**
* アニメーションの再生を一時停止する
*
* @retval true  成功
* @retval false 失敗(アニメーションが停止している、またはアニメーションが設定されていない)
*/
bool AnimatedMeshRenderer::Pause()
{
  if (animation) {
    switch (state) {
    case State::play:  state = State::pause; return true;
    case State::stop:  return false;
    case State::pause: return true;
    }
  }
  return false;
}

/**
* アニメーションの再生を再開する
*
* @retval true  成功
* @retval false 失敗(アニメーションが停止している、またはアニメーションが設定されていない)
*/
bool AnimatedMeshRenderer::Resume()
{
  if (animation) {
    switch (state) {
    case State::play:  return true;
    case State::stop:  return false;
    case State::pause: state = State::play; return true;
    }
  }
  return false;
}

/**
* アニメーションの再生位置を設定する
*
* @param position 再生位置(秒)
*/
void AnimatedMeshRenderer::SetPosition(float position)
{
  time = position;
  if (animation) {
    if (isLoop) {
      if (time >= animation->totalTime) {
        time -= animation->totalTime;
      } else if (time < 0) {
        const float n = std::ceil(-time / animation->totalTime);
        time += animation->totalTime * n;
      }
    } else {
      if (time >= animation->totalTime) {
        time = animation->totalTime;
      } else if (time < 0) {
        time = 0;
      }
    }
  }
}

/**
* アニメーションの再生位置を取得する
*
* @return 再生位置(秒)
*/
float AnimatedMeshRenderer::GetPosition() const
{
  return time;
}

/**
* アニメーションの再生が終了しているか調べる
*
* @retval true  終了している
* @retval false 終了していない、または一度もPlay()が実行されていない
*
* ループ再生中の場合、この関数は常にfalseを返すことに注意
*/
bool AnimatedMeshRenderer::IsFinished() const
{
  if (!file || !animation) {
    return false;
  }
  return animation->totalTime <= time;
}

/**
* アクターにスタティックメッシュレンダラを設定する
*
* @param actor    レンダラを設定するアクター
* @param filename glTFファイル名
* @param meshNo   描画するメッシュのインデックス
*/
StaticMeshRendererPtr SetStaticMeshRenderer(
  Actor& actor, const char* filename, int meshNo)
{
  GameEngine& engine = GameEngine::Get();
  auto renderer = std::make_shared<StaticMeshRenderer>();
  renderer->SetMesh(engine.LoadGltfFile(filename), meshNo);
  actor.renderer = renderer;
  actor.shader = Shader::StaticMesh;
  return renderer;
}

/**
* アクターにアニメーションメッシュレンダラを設定する
*
* @param actor    レンダラを設定するアクター
* @param filename glTFファイル名
* @param sceneNo  描画するシーンの番号
*/
AnimatedMeshRendererPtr SetAnimatedMeshRenderer(
  Actor& actor, const char* filename, int sceneNo)
{
  GameEngine& engine = GameEngine::Get();
  auto renderer = std::make_shared<AnimatedMeshRenderer>();
  GltfFilePtr p = engine.LoadGltfFile(filename);
  renderer->SetFileBuffer(engine.GetGltfFileBuffer());
  renderer->SetFile(p, sceneNo);
  actor.renderer = renderer;
  actor.shader = Shader::AnimatedMesh;
  return renderer;
}

