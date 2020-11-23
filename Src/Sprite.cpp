/**
* @file Sprite.cpp
*/
#include "Sprite.h"
#include "GLContext.h"
#include "GameData.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>
#include <limits>

/**
* コンストラクタ.
*
* @param pos    表示する位置.
* @param tex    描画に称するテクスチャ.
* @param origin 表示範囲の左下座標(テクスチャ座標系).
* @param size   表示範囲の大きさ(テクスチャ座標系).
*/
Sprite::Sprite(const glm::vec3& pos, std::shared_ptr<Texture::Image2D> tex,
  const glm::vec2& origin, const glm::vec2& size) :
  texture(tex), imageOrigin(origin), imageSize(size), position(pos)
{
}

/**
* スプライトの状態を更新する.
*
* @param deltaTime 前回の更新からの経過時間(秒).
*/
void Sprite::Update(float deltaTime)
{
  // 寿命チェック.
  if (lifespan > 0) {
    // 寿命を減らした結果が0以下になったら死亡.
    lifespan -= deltaTime;
    if (lifespan <= 0) {
      isDead = true;
    }
  }

  // 移動速度に重力を加算.
  const glm::vec3 gravity = GameData::Get().gravity;
  velocity += gravity * gravityScale * deltaTime;

  // 座標を更新.
  position += velocity * deltaTime;
  rotation += angularVelocity * deltaTime;
  scale += scaleVelocity * deltaTime;
  color += colorVelocity * deltaTime;
}

/**
* スプライトの配列を更新する.
*
* @param sprites   スプライトの配列
* @param deltaTime 前回の更新からの経過時間.
*/
void UpdateSpriteList(std::vector<std::shared_ptr<Sprite>>& sprites, float deltaTime)
{
  // 配列が空なら何もしない.
  if (sprites.empty()) {
    return;
  }

  // すべてのスプライトの寿命、座標、速度などを更新.
  for (auto& e : sprites) {
    e->Update(deltaTime);
  }

  // dead状態のスプライトを削除.
  const auto isDead = [](std::shared_ptr<Sprite> p) { return p->isDead; };
  const auto i = std::remove_if(sprites.begin(), sprites.end(), isDead);
  sprites.erase(i, sprites.end());
}

/**
* デストラクタ.
*/
SpriteRenderer::~SpriteRenderer()
{
  Free();
}

/**
* スプライト用のメモリを確保する.
*
* @param maxSpriteCount 格納可能な最大スプライト数.
*
* @retval true  確保成功.
* @retval false 確保失敗、または既に確保済み.
*/
bool SpriteRenderer::Allocate(size_t maxSpriteCount)
{
  // vaoが存在する場合は作成済み.
  if (buffers[1].vao) {
    std::cerr << "[警告]" << __func__ << ": VAOは作成済みです.\n";
    return false;
  }

  // インデックスデータを作成.
  const size_t maxIndex = std::min<size_t>(maxSpriteCount * 6, 65536) / 6;
  std::vector<GLushort> indices;
  indices.resize(maxIndex * 6);
  for (GLushort i = 0; i < maxIndex; ++i) {
    const GLushort vertexIndex = i * 4;
    const size_t arrayIndex = static_cast<size_t>(i) * 6;
    indices[arrayIndex + 0] = vertexIndex + 0;
    indices[arrayIndex + 1] = vertexIndex + 1;
    indices[arrayIndex + 2] = vertexIndex + 2;
    indices[arrayIndex + 3] = vertexIndex + 2;
    indices[arrayIndex + 4] = vertexIndex + 3;
    indices[arrayIndex + 5] = vertexIndex + 0;
  }
  ibo = GLContext::CreateBuffer(indices.size() * sizeof(GLushort), indices.data());

  // GPUメモリを確保し、VAOを作成.
  const GLbitfield flags = GL_DYNAMIC_STORAGE_BIT;// GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
  // 最大スプライト数の4倍が必要な頂点数.
  const GLsizei vertexCount = static_cast<GLsizei>(maxSpriteCount * 4);
  for (auto& e : buffers) {
    e.vboPosition = GLContext::CreateBuffer(vertexCount * sizeof(glm::vec3), nullptr, flags);
    e.vboColor = GLContext::CreateBuffer(vertexCount * sizeof(glm::vec4), nullptr, flags);
    e.vboTexcoord = GLContext::CreateBuffer(vertexCount * sizeof(glm::vec2), nullptr, flags);
    e.vboNormal = GLContext::CreateBuffer(vertexCount * sizeof(glm::vec3), nullptr, flags);
    e.vao = GLContext::CreateVertexArray(e.vboPosition, e.vboColor, e.vboTexcoord, e.vboNormal, ibo);
    if (!e.vboPosition || !e.vboColor || !e.vboTexcoord || !e.vboNormal || !ibo || !e.vao) {
      std::cerr << "[エラー]" << __func__ << ": VAOの作成に失敗.\n";
      Free();
      return false;
    }
  }

  primitives.reserve(100);
  this->maxSpriteCount = maxSpriteCount;

  return true;
}

/**
* 描画データを破棄しGPUメモリを解放する.
*/
void SpriteRenderer::Free()
{
  primitives.clear();
  maxSpriteCount = 0;
  updatingBufferIndex = 0;

  for (auto& e : buffers) {
    glDeleteVertexArrays(1, &e.vao);
    e.vao = 0;
    glDeleteBuffers(1, &e.vboTexcoord);
    e.vboTexcoord = 0;
    glDeleteBuffers(1, &e.vboColor);
    e.vboColor = 0;
    glDeleteBuffers(1, &e.vboPosition);
    e.vboPosition = 0;
    glDeleteBuffers(1, &e.vboNormal);
    e.vboNormal = 0;
  }
  glDeleteBuffers(1, &ibo);
  ibo = 0;
}

/**
* 描画データを更新する.
*
* @param sprites  描画するスプライトの配列.
* @param matView  描画に使用するビュー行列.
*/
void SpriteRenderer::Update(const std::vector<std::shared_ptr<Sprite>>& sprites, const glm::mat4& matView)
{
  // 描画データを削除.
  primitives.clear();

  // スプライトがひとつもなければこれ以上やることはない.
  if (sprites.empty()) {
    return;
  }

  // スプライトをカメラからの距離順で並べた配列tmpを作成.
  using SortingData = std::pair<float, const Sprite*>;
  std::vector<SortingData> tmp;
  tmp.resize(sprites.size());
  for (size_t i = 0; i < sprites.size(); ++i) {
    tmp[i].first = (matView * glm::vec4(sprites[i]->position, 1)).z;
    tmp[i].second = sprites[i].get();
  }
  std::sort(tmp.begin(), tmp.end(),
    [](const SortingData& a, const SortingData& b) {
      return a.first < b.first;
    });

  // 表示要求されたスプライトの数が多すぎて、確保したGPUメモリでは足りない場合、
  // 警告メッセージを表示し、入り切らない分は切り捨てる.
  if (tmp.size() > maxSpriteCount) {
    std::cout << "[警告]" << __func__ <<
      ": スプライト数が多すぎます(要求=" << tmp.size() <<
      "/最大=" << maxSpriteCount << ").\n";
    tmp.resize(maxSpriteCount);
  }

  // スプライトをカメラに向ける「逆ビュー回転行列」を作成する.
  // 1. 平行移動成分を除去するためglm::mat3コンストラクタで左上3x3を取得.
  // 2. 拡大縮小成分を除去するためinverse-transpose変換を行う.
  // 3. カメラの回転を打ち消すため、回転成分の逆行列を作成.
  const glm::mat3 matViewR = glm::transpose(glm::inverse(glm::mat3(matView)));
  const glm::mat4 matInverseViewR = glm::inverse(matViewR);

  // 頂点データを格納する配列を用意.
  // GPUメモリにコピーしたらもう不要なのでローカル変数を使う.
  std::vector<glm::vec3> positions;
  std::vector<glm::vec4> colors;
  std::vector<glm::vec2> texcoords;
  std::vector<glm::vec3> normals;
  const size_t vertexCount = tmp.size() * 4;
  positions.resize(vertexCount);
  colors.resize(vertexCount);
  texcoords.resize(vertexCount);
  normals.resize(vertexCount);

  // 最初のプリミティブを設定.
  primitives.push_back({ 0, 0, tmp[0].second->texture });

  // すべてのスプライトを頂点データに変換.
  for (size_t i = 0; i < tmp.size(); ++i) {
    const Sprite& sprite = *tmp[i].second;

    // テクスチャ座標の右下originと大きさsizeを取得.
    const glm::vec2 origin = sprite.imageOrigin;
    const glm::vec2 size = sprite.imageSize;

    // 法線用と頂点用の座標変換行列を作成.
    const glm::mat4 matT = glm::translate(glm::mat4(1), sprite.position);
    const glm::mat4 matR = glm::rotate(glm::mat4(1), sprite.rotation, glm::vec3(0, 0, 1));
    const glm::mat4 matS = glm::scale(glm::mat4(1), glm::vec3(sprite.scale, 1));
    const glm::mat3 matNormal = matInverseViewR * matR;
    const glm::mat4 matModel = matT * matInverseViewR * matR * matS;

    // データの格納開始位置vを計算.
    size_t v = i * 4;

    // 座標、色、テクスチャ座標、法線を必要に応じて変換して配列に代入.
    positions[v] = matModel * glm::vec4(-0.5f, -0.5f, 0, 1);
    colors[v] = sprite.color;
    texcoords[v] = origin;
    normals[v] = matNormal * glm::normalize(glm::vec3(-1, -1, 1));
    ++v; // 次の格納位置へ.

    positions[v] = matModel * glm::vec4(0.5f, -0.5f, 0, 1);
    colors[v] = sprite.color;
    texcoords[v] = glm::vec2(origin.x + size.x, origin.y);
    normals[v] = matNormal * glm::normalize(glm::vec3(1, -1, 1));
    ++v; // 次の格納位置へ.

    positions[v] = matModel * glm::vec4(0.5f, 0.5f, 0, 1);
    colors[v] = sprite.color;
    texcoords[v] = origin + size;
    normals[v] = matNormal * glm::normalize(glm::vec3(1, 1, 1));
    ++v; // 次の格納位置へ.

    positions[v] = matModel * glm::vec4(-0.5f, 0.5f, 0, 1);
    colors[v] = sprite.color;
    texcoords[v] = glm::vec2(origin.x, origin.y + size.y);
    normals[v] = matNormal * glm::normalize(glm::vec3(-1, 1, 1));

    // 描画プリミティブを更新.
    Primitive& e = primitives.back();
    if (e.texture == sprite.texture) {
      // 同じテクスチャを使うスプライトの場合はデータ数を増やすだけ.
      // ただし、インデックスが最大値を超える場合は新しい描画データを追加.
      if (e.count + 6 <= 65536) {
        e.count += 6;
      } else {
        const GLint vertexCount = (e.count / 6) * 4;
        primitives.push_back({ 6, e.baseVertex + vertexCount, sprite.texture });
      }
    } else {
      // テクスチャが違っている場合は新しい描画データを追加.
      const GLint vertexCount = (e.count / 6) * 4;
      primitives.push_back({ 6, e.baseVertex + vertexCount, sprite.texture });
    }
  }

  // 頂点データをGPUメモリにコピー.
  Buffer& e = buffers[updatingBufferIndex];
  glNamedBufferSubData(e.vboPosition, 0, positions.size() * sizeof(glm::vec3), positions.data());
  glNamedBufferSubData(e.vboColor, 0, colors.size() * sizeof(glm::vec4), colors.data());
  glNamedBufferSubData(e.vboTexcoord, 0, texcoords.size() * sizeof(glm::vec2), texcoords.data());
  glNamedBufferSubData(e.vboNormal, 0, normals.size() * sizeof(glm::vec3), normals.data());

  // コピー先のGPUメモリを切り替える.
  updatingBufferIndex = !updatingBufferIndex;
}

/**
* スプライトを描画する.
*
* @param pipeline 描画に使用するグラフィックスパイプライン.
* @param matVP    描画に使用するビュープロジェクション行列.
*/
void SpriteRenderer::Draw(
  std::shared_ptr<Shader::Pipeline> pipeline,
  const glm::mat4& matVP) const
{
  // データがなければ何もしない.
  if (primitives.empty()) {
    return;
  }

  // パイプラインをバインドし、各種データを設定する.
  pipeline->Bind();
  pipeline->SetMVP(matVP);
  pipeline->SetModelMatrix(glm::mat4(1));
  pipeline->SetObjectColor(glm::vec4(1));

  // 深度テストは行うが、書き込みはしないように設定.
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // アルファブレンディングを有効化.
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // 裏面も描画するように設定.
  glDisable(GL_CULL_FACE);

  // VAOをバインド.
  glBindVertexArray(buffers[!updatingBufferIndex].vao);

  // 描画データを順番に描画する.
  for (const auto& e : primitives) {
    e.texture->Bind(0);
    glDrawElementsBaseVertex(GL_TRIANGLES, e.count, GL_UNSIGNED_SHORT, nullptr, e.baseVertex);
  }

  // テクスチャのバインドを解除.
  const GLuint id = 0;
  glBindTextures(0, 1, &id);

  // VAOのバインドを解除.
  glBindVertexArray(0);

  // 深度バッファへの書き込みを許可.
  glDepthMask(GL_TRUE);

  // 裏面は描画しないように設定.
  glEnable(GL_CULL_FACE);
}

