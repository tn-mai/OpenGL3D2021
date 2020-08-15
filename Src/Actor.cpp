/**
* @file Actor.cpp
*/
#include "Actor.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* コンストラクタ.
*
* @param actorName 作成するアクターに付ける名前.
* @param pid       アクターの外見を表すプリミティブのID.
* @param tex       プリミティブに貼り付けるテクスチャ.
* @param pos       アクターの座標.
*/
Actor::Actor(std::string actorName, const Mesh::Primitive* prim,
  std::shared_ptr<Texture::Image2D> tex, const glm::vec3& pos) :
  name(name), primitive(prim), texture(tex), position(pos)
{
}

/**
* アクターの状態を更新する.
*
* @param deltaTime 前回の更新からの経過時間(秒).
*/
void Actor::Update(float deltaTime)
{
  // 座標を更新.
  position += velocity * deltaTime;

  // アニメーションを更新.
  if (!animation.empty()) {
    animationTimer += deltaTime;
    if (animationTimer >= animationInterval) {
      animationTimer -= animationInterval;
      ++animationNo;
      if (animationNo >= animation.size()) {
        animationNo = 0;
      }
      primitive = animation[animationNo];
    }
  }
}

/**
* アクターを描画する.
*
* @param pipeline 行列の設定先となるパイプラインオブジェクト.
* @param matVP    描画に使用するビュープロジェクション行列.
*/
void Actor::Draw(const Shader::Pipeline& pipeline, const glm::mat4& matVP, const glm::mat4& matShadow) const
{
  // プリミティブが設定されていないときは何もせず終了.
  if (!primitive) {
    return;
  }

  // 平行移動させる行列を作る.
  const glm::mat4 matTranslate = glm::translate(glm::mat4(1), position);
  // X軸回転させる行列を作る.
  const glm::mat4 matRotateX = glm::rotate(
    glm::mat4(1), rotation.x, glm::vec3(1, 0, 0));
  // Y軸回転させる行列を作る.
  const glm::mat4 matRotateY = glm::rotate(
    glm::mat4(1), rotation.y, glm::vec3(0, 1, 0));
  // Z軸回転させる行列を作る.
  const glm::mat4 matRotateZ = glm::rotate(
    glm::mat4(1), rotation.z, glm::vec3(0, 0, 1));
  // 大きさを変える行列を作る.
  const glm::mat4 matScale = glm::scale(glm::mat4(1), scale);

  // 平行移動・回転・大きさ変更の行列を掛け算して、ひとつのモデル行列にまとめる.
  const glm::mat4 matModel =
    matTranslate * matRotateY * matRotateZ * matRotateX * matScale;

  // GPUメモリに行列を転送.
  pipeline.SetModelMatrix(matModel);
  pipeline.SetMVP(matVP * matModel);

  // テクスチャイメージスロット0番にテクスチャを割り当てる.
  texture->Bind(0);

  // プリミティブを描画.
  primitive->Draw();

    // 影を描画.
  if (hasShadow) {
    // 平行移動・回転・大きさ変更の行列を掛け算して、ひとつのモデル行列にまとめる.
    const glm::mat4 matModelShadow =
      matTranslate * matShadow * matRotateY * matRotateZ * matRotateX * matScale;

    // GPUメモリに影行列を転送.
    pipeline.SetModelMatrix(matModelShadow);
    pipeline.SetMVP(matVP * matModelShadow);

    // プリミティブを描画.
    primitive->Draw();
  }
}

/**
* 垂直円柱の衝突判定を設定する.
*
* @param top    る円柱の上端の座標.
* @param bottom る円柱の下端の座標.
* @param radius 円柱の半径.
*/
void Actor::SetCylinderCollision(float top, float bottom, float radius)
{
  collision.top = top;
  collision.bottom = bottom;
  collision.radius = radius;
}

/**
* アクターリストを更新する.
*
* @param actorList  更新するアクターリスト.
* @param deltaTime  前回の更新からの経過時間(秒).
*/
void UpdateActorList(ActorList& actorList, float deltaTime)
{
  for (size_t i = 0; i < actorList.size(); ++i) {
    actorList[i]->Update(deltaTime);
  }
}

/**
* アクターリストを描画する.
*
* @param actorList 描画するアクターリスト.
* @param matVP     描画に使用するビュープロジェクション行列.
*/
void RenderActorList(const ActorList& actorList,
  const glm::mat4& matVP, const glm::mat4& matShadow)
{
  Global& global = Global::Get();
  for (size_t i = 0; i < actorList.size(); ++i) {
    actorList[i]->Draw(*global.pipeline, matVP, matShadow);
  }
}

/**
* 2つのアクターの衝突状態を調べる.
*
* @param a アクターその1.
* @param b アクターその2.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool DetectCollision(const Actor& a, const Actor& b)
{
  if (a.position.y + a.collision.bottom >= b.position.y + b.collision.top) {
    return false;
  }
  if (a.position.y + a.collision.top <= b.position.y + b.collision.bottom) {
    return false;
  }
  const float dx = a.position.x - b.position.x;
  const float dz = a.position.z - b.position.z;
  const float d2 = dx * dx + dz * dz;
  const float r = a.collision.radius + b.collision.radius;
  return d2 < r * r;

/*
  // X軸の衝突判定.
  if (a.colWorld.min.x > b.colWorld.max.x) {
    return false;
  }
  if (a.colWorld.max.x < b.colWorld.min.x) {
    return false;
  }

  // Y軸の衝突判定.
  if (a.colWorld.min.y > b.colWorld.max.y) {
    return false;
  }
  if (a.colWorld.max.y < b.colWorld.min.y) {
    return false;
  }

  // Z軸の衝突判定.
  if (a.colWorld.min.z > b.colWorld.max.z) {
    return false;
  }
  if (a.colWorld.max.z < b.colWorld.min.z) {
    return false;
  }

  return true; // 衝突している.
*/
}

/**
* アクターの衝突を処理する.
*
* @param actors 衝突を処理するアクターの配列.
*/
void HandleCollisions(ActorList& actors)
{
  for (size_t ia = 0; ia < actors.size(); ++ia) {
    ActorPtr a = actors[ia]; // アクターA
    // 計算済み及び自分自身を除く、残りのアクターとの間で衝突判定を実行.
    for (size_t ib = ia + 1; ib < actors.size(); ++ib) {
      ActorPtr b = actors[ib]; // アクターB
      if (DetectCollision(*a, *b)) {
        // アクターAとBの衝突判定(円柱)の中心間の距離dを計算.
        const float dx = a->position.x - b->position.x;
        const float dz = a->position.z - b->position.z;
        const float d = std::sqrt(dx * dx + dz * dz);
        // dが長い方の半径より短ければ、垂直方向に重なっているとみなす.
        // そうでなければ水平方向に重なっているとみなす.
        if (d < std::max(a->collision.radius, b->collision.radius)) {
          // 円柱の下端の高さを計算.
          const float bottomA = a->position.y + a->collision.bottom;
          const float bottomB = b->position.y + b->collision.bottom;
          // 下端が高いほうを上に移動.
          if (bottomA > bottomB) {
            a->position.y += (b->position.y + b->collision.top) - bottomA;
          } else {
            b->position.y += (a->position.y + a->collision.top) - bottomB;
          }
        } else {
          // 衝突しない距離rを計算.
          const float r = a->collision.radius + b->collision.radius;
          // 押し返す距離sを計算.
          const float s = r - d;
          // 円柱の中心軸間の方向ベクトルnを計算.
          const glm::vec3 n(dx / d, 0, dz / d);
          // アクターAとBを均等に押し返す.
          a->position += n * s * 0.5f;
          b->position -= n * s * 0.5f;
        }
      }
      // 閉じカッコの数に注意.
    }
  }
}

