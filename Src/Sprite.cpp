/**
* @file Sprite.cpp
*/
#include "Sprite.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>

/**
* コンストラクタ
*/
Sprite::Sprite(const glm::vec3& position, std::shared_ptr<Texture> tex,
  const glm::vec2& uv0, const glm::vec2& uv1, float pixelsPerMeter) :
  Actor("Sprite", Primitive(), tex, position, glm::vec3(1), 0, glm::vec3(0)),
  uv0(uv0), uv1(uv1), pixelsPerMeter(pixelsPerMeter)
{
  layer = Layer::Sprite;
}

/**
* VAOに頂点アトリビュートを設定する
*/
void SetVertexAttribute(GLuint vao, GLuint index, 
  GLint size, GLenum type, GLboolean normalized,
  GLuint relativeOffset, GLuint bindingPoint)
{
  glEnableVertexArrayAttrib(vao, index);
  glVertexArrayAttribFormat(vao, index, size, type, normalized, relativeOffset);
  glVertexArrayAttribBinding(vao, index, bindingPoint);
}

/**
* スプライト用の頂点データ
*/
struct SpriteVertex
{
  glm::vec3   position;
  glm::u8vec4 color;
  glm::vec2   texcoord;
};

/**
* スプライト用のメモリを確保する
*
* @param maxSpriteCount 格納可能な最大スプライト数
*
* @retval true  確保成功
* @retval false 確保失敗、または既に確保済み
*/
bool SpriteRenderer::Allocate(size_t maxSpriteCount)
{
  // 最後のvaoが存在する場合は作成済み
  if (buffer[std::size(buffer) - 1].vao) {
    std::cerr << "[警告]" << __func__ << ": VAOは作成済みです\n";
    return false;
  }

  // IBOを作成
  const size_t maxVertexNo = std::min<size_t>(maxSpriteCount * 4, 65536);
  std::vector<GLushort> indices(maxVertexNo / 4 * 6);
  GLushort vertexNo = 0;
  for (size_t indexNo = 0; indexNo < indices.size(); indexNo += 6) {
    // 三角形1個目
    indices[indexNo + 0] = vertexNo + 0;
    indices[indexNo + 1] = vertexNo + 1;
    indices[indexNo + 2] = vertexNo + 2;

    // 三角形2個目
    indices[indexNo + 3] = vertexNo + 2;
    indices[indexNo + 4] = vertexNo + 3;
    indices[indexNo + 5] = vertexNo + 0;

    vertexNo += 4;
  }
  ibo = GLContext::CreateBuffer(
    indices.size() * sizeof(GLushort), indices.data());

  // VBOを作成
  const size_t vboSize = sizeof(SpriteVertex) * maxSpriteCount * 4;
  for (Buffer& e : buffer) {
    e.vbo = GLContext::CreateBuffer(vboSize, nullptr);
  }

  // VAOを作成
  const GLuint bindingPoint = 0;
  for (Buffer& e : buffer) {
    glCreateVertexArrays(1, &e.vao);
    glVertexArrayElementBuffer(e.vao, ibo);
    glVertexArrayVertexBuffer(
      e.vao, bindingPoint, e.vbo, 0, sizeof(SpriteVertex));

    // 頂点アトリビュートを設定
    SetVertexAttribute(e.vao, 0, 3, GL_FLOAT, GL_FALSE,
      offsetof(SpriteVertex, position), bindingPoint);

    SetVertexAttribute(e.vao, 1, 4, GL_UNSIGNED_BYTE, GL_TRUE,
      offsetof(SpriteVertex, color), bindingPoint);

    SetVertexAttribute(e.vao, 2, 2, GL_FLOAT, GL_FALSE,
      offsetof(SpriteVertex, texcoord), bindingPoint);
  }

  primitives.reserve(100);
  this->maxSpriteCount = maxSpriteCount;

  return true;
}

/**
* 描画データを破棄しGPUメモリを解放する
*/
void SpriteRenderer::Deallocate()
{
  primitives.clear();
  maxSpriteCount = 0;
  updatingBufferIndex = 0;

  for (Buffer& e : buffer) {
    glDeleteVertexArrays(1, &e.vao);
    glDeleteBuffers(1, &e.vbo);
    e = Buffer();
  }
  glDeleteBuffers(1, &ibo);
  ibo = 0;
}

/**
* 描画データを更新する
*
* @param sprites  更新するスプライトの配列
* @param matView  更新に使用するビュー行列
*/
void SpriteRenderer::Update(
  const std::vector<std::shared_ptr<Actor>>& sprites, const glm::mat4& matView)
{
  // プリミティブデータを削除
  primitives.clear();

  // スプライトがひとつもなければ何もしない
  if (sprites.empty()) {
    return;
  }

  // スプライトの数が多すぎる場合、描画するスプライト数を制限する
  size_t spriteCount = sprites.size();
  if (sprites.size() > maxSpriteCount) {
    std::cerr << "[警告]" << __func__ <<
      ": スプライト数が多すぎます(要求=" << sprites.size() <<
      "/最大=" << maxSpriteCount << ")\n";
    spriteCount = maxSpriteCount;
  }

#if 1
  // カメラからの距離とスプライトのアドレスをペアにして配列に代入
  struct SortingData {
    float z;
    const Sprite* sprite;
  };
  std::vector<SortingData> sortedSprites(sprites.size());
  for (size_t i = 0; i < sprites.size(); ++i) {
    const glm::vec3 p = matView * glm::vec4(sprites[i]->position, 1);
    sortedSprites[i].z = p.z;
    sortedSprites[i].sprite = static_cast<const Sprite*>(sprites[i].get());
  }

  // 配列をカメラからの距離(Z軸)順で並べ替える
  std::sort(sortedSprites.begin(), sortedSprites.end(),
    [](const SortingData& a, const SortingData& b) { return a.z < b.z; });
  sortedSprites.resize(spriteCount);

  // 最初のプリミティブを作成
  const PrimitiveRenderer* renderer = static_cast<PrimitiveRenderer*>(
    sortedSprites[0].sprite->renderer.get());
  primitives.push_back(Primitive{ 0, 0, renderer->GetTexture() });
#else
  // 最初のプリミティブを作成
  primitives.push_back(Primitive{ 0, 0, sprites[0]->tex });
#endif

  // スプライトをカメラに向ける「逆ビュー回転行列」を作成する.
  // 1. 平行移動成分を除去するためglm::mat3コンストラクタで左上3x3を取得.
  // 2. 拡大縮小成分を除去するためinverse-transpose変換を行う.
  // 3. カメラの回転を打ち消すため、回転成分の逆行列を作成.
  const glm::mat3 matViewR = glm::transpose(glm::inverse(glm::mat3(matView)));
  const glm::mat4 matInvViewR = glm::inverse(matViewR);

  // すべてのスプライトを頂点データに変換
  std::vector<SpriteVertex> vertices(spriteCount * 4);
  for (int i = 0; i < spriteCount; ++i) {
#if 1
    const Sprite& sprite = *sortedSprites[i].sprite;
#else
    const Sprite& sprite = static_cast<Sprite&>(*sprites[i]);
#endif
    const PrimitiveRenderer* renderer = 
      static_cast<PrimitiveRenderer*>(sprite.renderer.get());
    std::shared_ptr<Texture> tex = renderer->GetTexture();

    // 表示サイズを計算
    const float sx =
      sprite.scale.x * tex->GetWidth() / sprite.pixelsPerMeter;
    const float sy =
      sprite.scale.y * tex->GetHeight() / sprite.pixelsPerMeter;

    // 座標変換行列を作成
    const glm::mat4 matT = glm::translate(glm::mat4(1), sprite.position);
    const glm::mat4 matR =
      glm::rotate(glm::mat4(1), sprite.rotation, glm::vec3(0, 0, 1));
    const glm::mat4 matS = glm::scale(glm::mat4(1), glm::vec3(sx, sy, 1));
    const glm::mat4 matA = glm::translate(glm::mat4(1), sprite.adjustment);
    const glm::mat4 matModel = matT * matInvViewR * matR * matS * matA;

    // 色をvec4からu8vec4に変換
    const glm::u8vec4 color = glm::clamp(sprite.color, 0.0f, 1.0f) * 255.0f;

    // データの格納開始位置vを計算
    int v = i * 4;

    // 左下の頂点データを作成
    vertices[v].position = matModel * glm::vec4(-0.5f, -0.5f, 0, 1);
    vertices[v].color = color;
    vertices[v].texcoord = sprite.uv0;
    ++v; // 次の格納位置へ

    // 右下の頂点データを作成
    vertices[v].position = matModel * glm::vec4(0.5f, -0.5f, 0, 1);
    vertices[v].color = color;
    vertices[v].texcoord = glm::vec2(sprite.uv1.x, sprite.uv0.y);
    ++v; // 次の格納位置へ

    // 右上の頂点データを作成
    vertices[v].position = matModel * glm::vec4(0.5f, 0.5f, 0, 1);
    vertices[v].color = color;
    vertices[v].texcoord = sprite.uv1;
    ++v; // 次の格納位置へ

    // 左上の頂点データを作成
    vertices[v].position = matModel * glm::vec4(-0.5f, 0.5f, 0, 1);
    vertices[v].color = color;
    vertices[v].texcoord = glm::vec2(sprite.uv0.x, sprite.uv1.y);

    // インデックス数を更新
    // テクスチャが等しく、更新後のインデックス数がIBOの許容値以下なら、
    // インデックス数を更新する。そうでなければ新しいプリミティブを追加する。
    const int maxCountPerPrimitive = 65536 / 4 * 6;
    Primitive& prim = primitives.back();
    if ((prim.texture == tex) &&
      (prim.count + 6 < maxCountPerPrimitive)) {
      prim.count += 6;
    } else {
      primitives.push_back(Primitive{ 6, i * 4, tex });
    }
  } // spriteCount

  // 書き込み先のバッファが描画に使われている場合、描画の完了を待つ
  Buffer& buf = buffer[updatingBufferIndex];
  if (buf.sync) {
    const GLenum result = glClientWaitSync(buf.sync, 0, 0);
    switch (result) {
    case GL_ALREADY_SIGNALED:
      // 既に完了している(正常)
      break;
    case GL_TIMEOUT_EXPIRED:
      std::cerr << "[警告]" << __func__ << ":描画に時間がかかっています\n";
      glClientWaitSync(buf.sync, 0, 1'000'000); // 最大1秒間待つ
      break;
    default:
      std::cerr << "[エラー]" << __func__ << ":同期に失敗(" << result << ")\n";
      break;
    }
    glDeleteSync(buf.sync);
    buf.sync = 0;
  }

  // 頂点データをGPUメモリにコピー
  CopyData(buf.vbo, sizeof(SpriteVertex), 0,
    vertices.size(), vertices.data());

  // 更新対象を切り替える
  updatingBufferIndex = (updatingBufferIndex + 1) % std::size(buffer);
}

/**
* スプライトを描画する
*
* @param pipeline 描画に使用するグラフィックスパイプライン
* @param matVP    描画に使用するビュープロジェクション行列
*/
void SpriteRenderer::Draw(
  std::shared_ptr<ProgramPipeline> pipeline,
  const glm::mat4& matVP)
{
  // プリミティブがひとつもなければ何もしない
  if (primitives.empty()) {
    return;
  }

  // パイプラインをバインド
  const GLint locMatTRS = 0;
  pipeline->Bind();
  pipeline->SetUniform(locMatTRS, matVP);

  // アルファブレンディングを有効化
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // 裏面も描画するように設定
  glDisable(GL_CULL_FACE);

  // 深度テストは行うが、書き込みはしないように設定
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // VAOをバインド
  const size_t index = (updatingBufferIndex + std::size(buffer) - 1) % std::size(buffer);
  glBindVertexArray(buffer[index].vao);

  // 描画データを順番に描画する
  for (const Primitive& e : primitives) {
    e.texture->Bind(0);
    glDrawElementsBaseVertex(GL_TRIANGLES, e.count, GL_UNSIGNED_SHORT, nullptr, e.baseVertex);
  }

  // 描画の完了を監視する「同期オブジェクト」を作成
  buffer[index].sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

  // テクスチャのバインドを解除
  primitives.back().texture->Unbind(0);

  // VAOのバインドを解除
  glBindVertexArray(0);

  // 描画設定を戻す
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}
