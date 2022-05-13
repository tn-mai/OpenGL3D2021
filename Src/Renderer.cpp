/**
* @file Renderer.cpp
*/
#include "Renderer.h"
#include "ProgramPipeline.h"
#include "Texture.h"
#include "Actor.h"
#include "GltfMesh.h"
#include "VertexArrayObject.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <numeric>

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
*
*/
namespace GlobalAnimatedMeshState {

namespace /* unnamed */ {
ShaderStorageBufferPtr ssbo;
std::vector<glm::mat4> dataBuffer;
} // unnamed namespace

/**
* アニメーションメッシュ用のバッファを作成
*/
bool Initialize(size_t maxCount)
{
  ssbo = std::make_shared<ShaderStorageBuffer>(sizeof(glm::mat4) * maxCount);
  dataBuffer.reserve(maxCount);
  return ssbo.get();
}

/**
* アニメーションメッシュ用のバッファを削除
*/
void Finalize()
{
  if (ssbo) {
    ssbo.reset();
    dataBuffer.clear();
    dataBuffer.shrink_to_fit();
  }
}

/**
* アニメーションメッシュの描画用データをすべて削除
*/
void ClearData()
{
  dataBuffer.clear();
}

/**
* アニメーションメッシュの描画用データを追加
*/
GLintptr AddData(const Data& data)
{
  GLintptr offset = static_cast<GLintptr>(dataBuffer.size() * sizeof(glm::mat4));
  dataBuffer.push_back(data.matRoot);
  dataBuffer.insert(dataBuffer.end(), data.matBones.begin(), data.matBones.end());

  // オフセット境界が256バイトになるようにする
  dataBuffer.resize(((dataBuffer.size() + 3) / 4) * 4);
  return offset;
}

/**
* アニメーションメッシュの描画用データをGPUメモリにコピー
*/
void Upload()
{
  ssbo->BufferSubData(0, dataBuffer.size() * sizeof(Data), dataBuffer.data());
  ssbo->SwapBuffers();
}

/**
* アニメーションメッシュの描画に使うSSBO領域を割り当てる
*/
void Bind(GLuint bindingPoint, GLintptr offset, GLsizeiptr size)
{
  ssbo->Bind(bindingPoint, offset, size);
}

/**
* アニメーションメッシュの描画に使うSSBO領域の割り当てを解除する
*/
void Unbind(GLuint bindingPoint)
{
  ssbo->Unbind(bindingPoint);
}

} // GlobalAnimatedMeshState

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

  for (const auto& e : animation.translationList) {
    auto& trans = transList[e.targetNodeId];
    const glm::vec3 translation = Interporation(e, keyFrame);
    trans.m *= glm::translate(glm::mat4(1), translation);
  }
  for (const auto& e : animation.rotationList) {
    auto& trans = transList[e.targetNodeId];
    const glm::quat rotation = Interporation(e, keyFrame);
    trans.m *= glm::mat4_cast(rotation);
  }
  for (const auto& e : animation.scaleList) {
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
* @param frame     アニメーションの再生位置
*
* @return アニメーションを適用した座標変換行列リスト
*/
GlobalAnimatedMeshState::Data CalculateTransform(const GltfFilePtr& file,
  const GltfNode* meshNode, const GltfAnimation* animation,
  const std::vector<int>& nonAnimatedNodeList, float frame)
{
  GlobalAnimatedMeshState::Data data;
  if (!file || !meshNode) {
    return data;
  }

  if (animation) {
    const TransformationList transList = CalcAnimatedTransformations(*file, *animation, nonAnimatedNodeList, frame);
    if (meshNode->skin >= 0) {
      // アニメーションあり+スキンあり
      // @note jointsにはノード番号が格納されているが、頂点データのJOINTS_nには
      //       ノード番号ではなく「joints配列のインデックス」が格納されている。
      //       つまり、ボーン行列配列をjointsの順番でSSBOに格納する必要がある。
      const auto& joints = file->skins[meshNode->skin].joints;
      data.matBones.resize(joints.size());
      for (size_t i = 0; i < joints.size(); ++i) {
        const auto& joint = joints[i];
        data.matBones[i] = transList[joint.nodeId].m * joint.matInverseBindPose;
      }
      data.matRoot = glm::mat4(1);
    } else {
      // アニメーションあり+スキンなし
      const size_t nodeId = meshNode - &file->nodes[0];
      data.matRoot = transList[nodeId].m;
    }
  } else {
    // アニメーションなし
    data.matRoot = meshNode->matGlobal;
    if (meshNode->skin >= 0) {
      // スキンあり
      const auto& joints = file->skins[meshNode->skin].joints;
      data.matBones.resize(joints.size(), glm::mat4(1));
    }
  }
  return data;
}

} // unnamed namespace

/**
* クローンを作成する
*/
RendererPtr AnimatedMeshRenderer::Clone() const
{
  return std::make_shared<AnimatedMeshRenderer>(*this);
}

/**
* 再生するシーンを設定する
*/
void AnimatedMeshRenderer::SetScene(const GltfFilePtr& f, int sceneNo)
{
  file = f;
  this->sceneNo = sceneNo;
  scene = &f->scenes[sceneNo];
  animation = nullptr;
  ssboRangeList.clear();

  state = State::stop;
  frame = 0;
  animationSpeed = 1;
  isLoop = true;
}

/**
* アニメーション状態を更新する
*
* @param deltaTime 前回の更新からの経過時間
* @param actor     描画対象のアクター
*/
void AnimatedMeshRenderer::Update(const Actor& actor, float deltaTime)
{
  // 再生フレーム更新
  if (animation && state == State::play) {
    frame += deltaTime * animationSpeed;
    if (isLoop) {
      if (frame >= animation->totalTime) {
        frame -= animation->totalTime;
      } else if (frame < 0) {
        const float n = std::ceil(-frame / animation->totalTime);
        frame += animation->totalTime * n;
      }
    } else {
      if (frame >= animation->totalTime) {
        frame = animation->totalTime;
      } else if (frame < 0) {
        frame = 0;
      }
    }
  }

  // SSBOにコピーするデータを追加
  const glm::mat4 matModel = actor.GetModelMatrix();
  ssboRangeList.clear();
  for (const auto e : scene->meshNodes) {
    GlobalAnimatedMeshState::Data data = CalculateTransform(file, e, animation, nonAnimatedNodeList, frame);
    data.matRoot = matModel * data.matRoot;
    for (auto& m : data.matBones) {
      m = matModel * m;
    }
    const GLintptr offset = GlobalAnimatedMeshState::AddData(data);
    const GLsizeiptr size = static_cast<GLsizeiptr>((data.matBones.size() + 1) * sizeof(glm::mat4));
    ssboRangeList.push_back({ offset, size });
  }

  // 状態を更新
  if (animation) {
    switch (state) {
    case State::stop:
      break;
    case State::play:
      if (!isLoop && (frame >= animation->totalTime)) {
        state = State::stop;
      }
      break;
    case State::pause:
      break;
    }
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
    GlobalAnimatedMeshState::Bind(0, ssboRangeList[i].offset, ssboRangeList[i].size);
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

  GlobalAnimatedMeshState::Unbind(0);
  glBindVertexArray(0);
}

/**
* アニメーションの再生状態を取得する
*
* @return 再生状態を示すState列挙型の値
*/
AnimatedMeshRenderer::State AnimatedMeshRenderer::GetState() const
{
  return state;
}

/**
* メッシュに関連付けられたアニメーションのリストを取得する
*
* @return アニメーションリスト
*/
const std::vector<GltfAnimation>& AnimatedMeshRenderer::GetAnimationList() const
{
  if (!file) {
    static const std::vector<GltfAnimation> dummy;
    return dummy;
  }
  return file->animations;
}

/**
* アニメーション時間を取得する
*
* @return アニメーション時間(秒)
*/
float AnimatedMeshRenderer::GetTotalAnimationTime() const
{
  if (!file || !animation) {
    return 0;
  }
  return animation->totalTime;
}

/**
*
*/
std::vector<int> MakeNonAnimatedNodeList(size_t size, const GltfAnimation* animation)
{
  std::vector<int> nonAnimatedNodeList(size);
  std::iota(nonAnimatedNodeList.begin(), nonAnimatedNodeList.end(), 0);
  for (auto e : animation->scaleList) {
    nonAnimatedNodeList[e.targetNodeId] = -1;
  }
  for (auto e : animation->rotationList) {
    nonAnimatedNodeList[e.targetNodeId] = -1;
  }
  for (auto e : animation->translationList) {
    nonAnimatedNodeList[e.targetNodeId] = -1;
  }
  nonAnimatedNodeList.erase(
    std::remove(nonAnimatedNodeList.begin(), nonAnimatedNodeList.end(), -1),
    nonAnimatedNodeList.end());
  return nonAnimatedNodeList;
}

/**
* アニメーションを再生する
*
* @param animationName 再生するアニメーションの名前
* @param isLoop        ループ再生の指定(true=ループする false=ループしない)
*
* @retval true  再生開始
* @retval false 再生失敗
*/
bool AnimatedMeshRenderer::Play(const std::string& animationName, bool isLoop)
{
  if (file) {
    for (const auto& e : file->animations) {
      if (e.name == animationName) {
        animation = &e;
        frame = 0;
        state = State::play;
        this->isLoop = isLoop;
        nonAnimatedNodeList = MakeNonAnimatedNodeList(file->nodes.size(), animation);
        return true;
      }
    }
  }
  return false;
}

/**
*
*/
bool AnimatedMeshRenderer::Play(size_t index, bool isLoop)
{
  if (file && index < file->animations.size()) {
    animation = &file->animations[index];
    frame = 0;
    state = State::play;
    this->isLoop = isLoop;
    nonAnimatedNodeList = MakeNonAnimatedNodeList(file->nodes.size(), animation);
    return true;
  }
  return false;
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
    case State::play:
      state = State::stop;
      return true;
    case State::stop:
      return true;
    case State::pause:
      state = State::stop;
      return true;
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
    case State::play:
      state = State::pause;
      return true;
    case State::stop:
      return false;
    case State::pause:
      return true;
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
    case State::play:
      return true;
    case State::stop:
      return false;
    case State::pause:
      state = State::play;
      return true;
    }
  }
  return false;
}

/**
* 再生中のアニメーション名を取得する
*
* @return 再生中のアニメーション名
*         一度もPlay()に成功していない場合、空の文字列が返される
*/
const std::string& AnimatedMeshRenderer::GetAnimation() const
{
  if (!animation) {
    static const std::string dummy("");
    return dummy;
  }
  return animation->name;
}

/**
* アニメーションの再生速度を設定する
*
* @param speed 再生速度(1.0f=等速, 2.0f=2倍速, 0.5f=1/2倍速)
*/
void AnimatedMeshRenderer::SetAnimationSpeed(float speed)
{
  animationSpeed = speed;
}

/**
* アニメーションの再生速度を取得する
*
* @return 再生速度
*/
float AnimatedMeshRenderer::GetAnimationSpeed() const
{
  return animationSpeed;
}

/**
* アニメーションの再生位置を設定する
*
* @param position 再生位置(秒)
*/
void AnimatedMeshRenderer::SetPosition(float position)
{
  frame = position;
  if (animation) {
    if (isLoop) {
      if (frame >= animation->totalTime) {
        frame -= animation->totalTime;
      } else if (frame < 0) {
        const float n = std::ceil(-frame / animation->totalTime);
        frame += animation->totalTime * n;
      }
    } else {
      if (frame >= animation->totalTime) {
        frame = animation->totalTime;
      } else if (frame < 0) {
        frame = 0;
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
  return frame;
}

/**
* アニメーションの再生が終了しているか調べる
*
* @retval true  終了している
* @retval false 終了していない、または一度も有効な名前でPlay()が実行されていない
*
* ループ再生中の場合、この関数は常にfalseを返すことに注意
*/
bool AnimatedMeshRenderer::IsFinished() const
{
  if (!file || !animation) {
    return false;
  }
  return animation->totalTime <= frame;
}

/**
* ループ再生の有無を取得する
*
* @retval true  ループ再生される
* @retval false ループ再生されない
*/
bool AnimatedMeshRenderer::IsLoop() const
{
  return isLoop;
}

/**
* ループ再生の有無を設定する
*
* @param isLoop ループ再生の有無
*/
void AnimatedMeshRenderer::SetLoop(bool isLoop)
{
  this->isLoop = isLoop;
}

