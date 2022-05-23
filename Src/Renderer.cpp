/**
* @file Renderer.cpp
*/
#include "Renderer.h"
#include "ProgramPipeline.h"
#include "Texture.h"
#include "Actor.h"
#include "GameEngine.h"
#include "GltfMesh.h"
#include "VertexArrayObject.h"
#include <glm/gtc/matrix_transform.hpp>
#include <numeric>
#include <algorithm>

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
      time -= animation->totalTime * std::floor(time / animation->totalTime);
    } else {
      time = std::clamp(time, 0.0f, animation->totalTime);
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
  // 全メッシュのアニメーション行列を更新
  const glm::mat4 matModel = actor.GetModelMatrix();
  ssboRangeList.clear();
  for (const GltfNode* e : scene->meshNodes) {
    // アニメーション行列を計算
    auto matBones = CalcAnimationMatrices(
      file, e, animation.get(), nonAnimatedNodes, time);

    // アニメーション行列にモデル行列を合成
    for (auto& m : matBones) {
      m = matModel * m;
    }

    // アニメーション行列をバッファに追加し、追加先のオフセットとサイズを記録
    const GLintptr offset = fileBuffer->AddAnimationMatrices(matBones);
    const GLsizeiptr size =
      static_cast<GLsizeiptr>(matBones.size() * sizeof(glm::mat4));
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

  // ノードに含まれる全てのメッシュを描画
  for (size_t i = 0; i < scene->meshNodes.size(); ++i) {
    const glm::uint meshNo = scene->meshNodes[i]->mesh;
    const GltfMesh& meshData = file->meshes[meshNo];

    // SSBOをバインド
    fileBuffer->BindAnimationBuffer(0, ssboRangeList[i].offset, ssboRangeList[i].size);

    // メッシュに含まれる全てのプリミティブを描画
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

  // SSBOとVAOのバインドを解除
  fileBuffer->UnbindAnimationBuffer(0);
  glBindVertexArray(0);
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
* アニメーションを設定する
*
* @param animation 再生するアニメーション
* @param isLoop    ループ再生の指定(true=ループする false=ループしない)
*
* @retval true  設定成功
* @retval false 設定失敗
*/
bool AnimatedMeshRenderer::SetAnimation(const GltfAnimationPtr& animation, bool isLoop)
{
  // ファイルが設定されていなければ何もしない
  if (!file) {
    return false;
  }

  // 同じアニメーションが指定された場合は何もしない
  if (this->animation == animation) {
    return true;
  }

  // アニメーションを設定
  this->animation = animation;

  // アニメーションがnullptrの場合は再生状態をを「停止」にする
  if (!animation) {
    state = State::stop;
    return false;
  }

  // アニメーションを行わないノードのリストを作る
  {
    const int withAnimation = -1; // 「アニメーションあり」を表す値

    // 全ノード番号のリストを作成
    const size_t size = file->nodes.size();
    nonAnimatedNodes.resize(size);
    std::iota(nonAnimatedNodes.begin(), nonAnimatedNodes.end(), 0);

    // アニメーション対象のノード番号を「アニメーションあり」で置き換える
    for (const auto& e : animation->scales) {
      if (e.targetNodeId < size) {
        nonAnimatedNodes[e.targetNodeId] = withAnimation;
      }
    }
    for (const auto& e : animation->rotations) {
      if (e.targetNodeId < size) {
        nonAnimatedNodes[e.targetNodeId] = withAnimation;
      }
    }
    for (const auto& e : animation->translations) {
      if (e.targetNodeId < size) {
        nonAnimatedNodes[e.targetNodeId] = withAnimation;
      }
    }

    // 「アニメーションあり」をリストから削除する
    const auto itr = std::remove(
      nonAnimatedNodes.begin(), nonAnimatedNodes.end(), withAnimation);
    nonAnimatedNodes.erase(itr, nonAnimatedNodes.end());
  }

  // 状態を「停止中」に設定
  time = 0;
  state = State::stop;
  this->isLoop = isLoop;

  return true;
}

/**
* アニメーションを設定する
*
* @param name   再生するアニメーションの名前
* @param isLoop ループ再生の指定(true=ループする false=ループしない)
*
* @retval true  設定成功
* @retval false 設定失敗
*/
bool AnimatedMeshRenderer::SetAnimation(const std::string& name, bool isLoop)
{
  if (!file) {
    return false;
  }

  for (const auto& e : file->animations) {
    if (e->name == name) {
      return SetAnimation(e, isLoop);
    }
  }
  return false;
}

/**
* アニメーションを設定する
*
* @param index  再生するアニメーション番号
* @param isLoop ループ再生の指定(true=ループする false=ループしない)
*
* @retval true  設定成功
* @retval false 設定失敗
*/
bool AnimatedMeshRenderer::SetAnimation(size_t index, bool isLoop)
{
  if (!file || index >= file->animations.size()) {
    return false;
  }
  return SetAnimation(file->animations[index], isLoop);
}

/**
* アニメーションの再生を開始・再開する
*
* @retval true  成功
* @retval false 失敗(アニメーションが設定されていない)
*/
bool AnimatedMeshRenderer::Play()
{
  if (animation) {
    switch (state) {
    case State::play:  return true;
    case State::stop:  state = State::play; return true;
    case State::pause: state = State::play; return true;
    }
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
* アニメーションの再生位置を設定する
*
* @param position 再生位置(秒)
*/
void AnimatedMeshRenderer::SetPosition(float position)
{
  time = position;
  if (animation) {
    if (isLoop) {
      time -= animation->totalTime * std::floor(time / animation->totalTime);
    } else {
      time = std::clamp(time, 0.0f, animation->totalTime);
    }
  } // animation
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
  if (!file || !animation || isLoop) {
    return false;
  }

  // 再生速度(方向)によって終了判定を変える
  if (animationSpeed < 0) {
    return time <= 0;
  }
  return time >= animation->totalTime;
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

