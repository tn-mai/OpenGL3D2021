/**
* @file Actor.cpp
*/
#include "Actor.h"
#include "GameData.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

/**
* コンストラクタ.
*
* @param name 作成するアクターに付ける名前.
* @param pid       アクターの外見を表すプリミティブのID.
* @param tex       プリミティブに貼り付けるテクスチャ.
* @param pos       アクターの座標.
*/
Actor::Actor(std::string name, const Mesh::Primitive* prim,
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

  // アニメーションデータがあればアニメーションする.
  if (animation && !animation->list.empty()) {
    // ループフラグがtrue、またはループフラグがfalse
    // かつアニメーション番号がプリミティブリストのデータ数を超えていない場合、
    // アニメーションを更新する.
    if (animation->isLoop || animationNo < animation->list.size() - 1) {
      animationTimer += deltaTime;
      // アニメーションタイマーがインターバル時間を超えていたら、
      // タイマーを減らして、アニメーション番号を薦める.
      if (animationTimer >= animation->interval) {
        animationTimer -= animation->interval;
        ++animationNo;
        // アニメーション番号がプリミティブリストのデータ数を超えた場合、
        // アニメーション番号を0に戻す.
        if (animationNo >= animation->list.size()) {
          animationNo = 0;
        }
      }
    }
    primitive = animation->list[animationNo];
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
    glm::mat4 matTransY = glm::mat4(1);
    matTransY[3][1] = position.y;
    glm::mat4 matTransXZ = glm::mat4(1);
    matTransXZ[3][0] = position.x;
    matTransXZ[3][2] = position.z;
    const glm::mat4 matModelShadow =
      matTransXZ * matShadow * matTransY * matRotateY * matRotateZ * matRotateX * matScale;

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
* @param top    円柱の上端の座標.
* @param bottom 円柱の下端の座標.
* @param radius 円柱の半径.
*/
void Actor::SetCylinderCollision(float top, float bottom, float radius)
{
  collision.shape = Collision::Shape::cylinder;
  collision.top = top;
  collision.bottom = bottom;
  collision.radius = radius;
}

/**
* 直方体の衝突判定を設定する.
*
* @param min 直方体の最小座標.
* @param max 直方体の最大座標.
*/
void Actor::SetBoxCollision(const glm::vec3& min, const glm::vec3& max)
{
  collision.shape = Collision::Shape::box;
  collision.boxMin = min;
  collision.boxMax = max;
}

/**
* アニメーションを設定する.
*
* @param animation アニメーションデータ.
*/
void Actor::SetAnimation(
  std::shared_ptr<Animation> animation)
{
  // 既に同じアニメーションが設定されている場合は何もしない.
  if (this->animation == animation) {
    return;
  }

  this->animation = animation;
  animationNo = 0;
  animationTimer = 0;
  if (animation && !animation->list.empty()) {
    primitive = animation->list[0];
  }
}

/**
* アクターリストを更新する.
*
* @param actorList  更新するアクターリスト.
* @param deltaTime  前回の更新からの経過時間(秒).
*/
void UpdateActorList(ActorList& actorList, float deltaTime)
{
  // 状態更新.
  for (size_t i = 0; i < actorList.size(); ++i) {
    actorList[i]->Update(deltaTime);
  }

  // 衝突判定.
  for (size_t ia = 0; ia < actorList.size(); ++ia) {
    Actor& a = *actorList[ia]; // アクターA
    // アクターAが死亡している場合は衝突しない.
    if (a.dead) {
      continue;
    }
    // 計算済み及び自分自身を除く、残りのアクターとの間で衝突判定を実行.
    for (size_t ib = ia + 1; ib < actorList.size(); ++ib) {
      Actor& b = *actorList[ib]; // アクターB
      // アクターBが死亡している場合は衝突しない.
      if (b.dead) {
        continue;
      }
      // 衝突判定.
      if (DetectCollision(a, b)) {
        // 衝突していたら、双方のOnHit関数を実行する.
        a.OnHit(a, b);
        b.OnHit(b, a);
      }
    } // 閉じ括弧の数に注意.
  }

  // dead状態のアクターを削除.
  const auto isDead = [](ActorPtr p) { return p->dead; };
  const ActorList::iterator i = std::remove_if(actorList.begin(), actorList.end(), isDead);
  actorList.erase(i, actorList.end());
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
  GameData& global = GameData::Get();
  for (size_t i = 0; i < actorList.size(); ++i) {
    actorList[i]->Draw(*global.pipeline, matVP, matShadow);
  }
}

/**
* 円柱と円柱の衝突を処理する.
*
* @param a       衝突形状が円柱のアクターA.
* @param b       衝突形状が円柱のアクターB.
* @param isBlock 貫通させない場合true. 貫通する場合false
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool CollideCylinderAndCylinder(Actor& a, Actor& b, bool isBlock)
{
  // 円柱Aの下端が円柱Bの上端の上にあるなら衝突していない.
  const float bottomA = a.position.y + a.collision.bottom;
  const float topB = b.position.y + b.collision.top;
  if (bottomA >= topB) {
    return false;
  }
  // 円柱Aの上端が円柱Bの下端の下にあるなら衝突していない.
  const float topA = a.position.y + a.collision.top;
  const float bottomB = b.position.y + b.collision.bottom;
  if (topA <= bottomB) {
    return false;
  }
  // アクターAとBの衝突判定(円柱)の中心間の距離の2乗(d2)を計算.
  const float dx = a.position.x - b.position.x;
  const float dz = a.position.z - b.position.z;
  const float d2 = dx * dx + dz * dz;
  // 衝突しない距離rを計算.
  const float r = a.collision.radius + b.collision.radius;
  // d2が衝突しない距離rの2乗以上なら衝突していない.
  if (d2 >= r * r) {
    return false;
  }

  // ブロック指定がなければ衝突したという情報だけを返す.
  if (!isBlock) {
    return true;
  }

  // ブロック指定があるので相手を押し返す.

  // Y軸方向の重なっている部分の長さを計算.
  const float overlapY = std::min(topA, topB) - std::max(bottomA, bottomB);

  // 短いほうの円柱の高さの半分を計算.
  const float halfY = std::min(topA - bottomA, topB - bottomB) * 0.5f;

  // 重なっている長さが短い方の円柱の高さの半分未満なら上に押し返す.
  // 半分以上なら横に押し返す.
  if (overlapY < halfY || d2 <= 0) {
    // 下端が高い位置にあるアクターを上に移動.
    if (bottomA > bottomB) {
      a.position.y += topB - bottomA; // Aを上に移動.
    } else {
      b.position.y += topA - bottomB; // Bを上に移動.
    }
  } else {
    // 中心間の距離dを計算.
    const float d = std::sqrt(d2);
    // 押し返す距離sを計算.
    const float s = r - d;
    // 円柱の中心軸間の方向ベクトルnを計算.
    const glm::vec3 n(dx / d, 0, dz / d);
    // アクターAとBを均等に押し返す.
    a.position += n * s * 0.5f;
    b.position -= n * s * 0.5f;
  }
  return true;
}

/**
* 円柱と直方体の衝突を処理する.
*
* @param a 衝突形状が円柱のアクター.
* @param b 衝突形状が直方体のアクター.
* @param isBlock 貫通させない場合true. 貫通する場合false
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool CollideCylinderAndBox(Actor& a, Actor& b, bool isBlock)
{
  // 円柱の下端が直方体の上端の上にあるなら衝突していない.
  const float bottomA = a.position.y + a.collision.bottom;
  const float topB = b.position.y + b.collision.boxMax.y;
  if (bottomA >= topB) {
    return false;
  }
  // 円柱の上端が直方体の下端の下にあるなら衝突していない.
  const float topA = a.position.y + a.collision.top;
  const float bottomB = b.position.y + b.collision.boxMin.y;
  if (topA <= bottomB) {
    return false;
  }

  // 円柱の中心軸と直方体の最も近いX軸上の位置pxを計算.
  float px = a.position.x;
  if (px < b.position.x + b.collision.boxMin.x) {
    px = b.position.x + b.collision.boxMin.x;
  } else if (px > b.position.x + b.collision.boxMax.x) {
    px = b.position.x + b.collision.boxMax.x;
  }

  // 円柱の中心軸と直方体の最も近いZ軸上の位置pzを計算.
  float pz = a.position.z;
  if (pz < b.position.z + b.collision.boxMin.z) {
    pz = b.position.z + b.collision.boxMin.z;
  } else if (pz > b.position.z + b.collision.boxMax.z) {
    pz = b.position.z + b.collision.boxMax.z;
  }
  // 円柱の中心軸から最も近い点までの距離の2乗(d2)を計算.
  const float dx = a.position.x - px;
  const float dz = a.position.z - pz;
  const float d2 = dx * dx + dz * dz;
  // d2が円柱の半径の2乗以上なら衝突していない.
  if (d2 >= a.collision.radius * a.collision.radius) {
    return false;
  }

  // ブロック指定がなければ衝突したという情報だけを返す.
  if (!isBlock) {
    return true;
  }

  // ブロック指定があるので円柱を押し返す.

  // Y軸方向の重なっている部分の長さを計算.
  const float overlapY = std::min(topA, topB) - std::max(bottomA, bottomB);

  // 円柱と直方体のうち、短いほうの高さの半分を計算.
  const float halfY = std::min(topA - bottomA, topB - bottomB) * 0.5f;

  // 重なっている長さが円柱の半分未満なら上または下に押し返す.
  // 半分以上なら横に押し返す.
  if (overlapY < halfY) {
    // 円柱の下端が直方体の下端より高い位置にあるなら円柱を上に移動.
    // そうでなければ下に移動.
    if (bottomA > bottomB) {
      a.position.y += topB - bottomA;
    } else {
      a.position.y -= topA - bottomB; // 円柱vs円柱と違うので注意.
    }
  } else if (d2 > 0) {
    // 中心軸と最近接点の距離dを計算.
    const float d = std::sqrt(d2);
    // 押し返す距離sを計算.
    const float s = a.collision.radius - d;
    // 最近接点から中心軸への方向ベクトルnを計算.
    const glm::vec3 n(dx / d, 0, dz / d);
    // アクターAを押し返す.
    a.position += n * s;
  } else {
    // 直方体の中心座標(cx, cz)を計算.
    const float cx = b.position.x +
      (b.collision.boxMin.x + b.collision.boxMax.x) * 0.5f;
    const float cz = b.position.z +
      (b.collision.boxMin.z + b.collision.boxMax.z) * 0.5f;

    // 直方体のX及びZ軸方向の長さの半分(hx, hz)を計算.
    const float hx = b.collision.boxMax.x - cx;
    const float hz = b.collision.boxMax.z - cz;

    // 直方体の中心座標から円柱の中心軸への距離(ox, oz)を計算.
    const float ox = a.position.x - cx;
    const float oz = a.position.z - cz;

    // 押し出す距離を計算.
    const float px = a.collision.radius + hx - std::abs(ox);
    const float pz = a.collision.radius + hz - std::abs(oz);

    // 押し出す距離が短いほうに押し出す.
    if (px < pz) {
      // 円柱の中心軸が-X側にあるなら-X方向の移動距離のほうが短い.
      if (ox < 0) {
        a.position.x -= px;
      } else {
        a.position.x += px;
      }
    } else {
      // 円柱の中心軸が-Z側にあるなら-Z方向の移動距離のほうが短い.
      if (oz < 0) {
        a.position.z -= pz;
      } else {
        a.position.z += pz;
      }
    }
  }

  return true;
}

/**
* アクターの衝突を処理する.
*
* @param a 衝突を処理するアクターA.
* @param b 衝突を処理するアクターB.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool DetectCollision(Actor& a, Actor& b)
{
  // アクターAとアクターBの両方が通り抜け禁止なら押し返す
  const bool isBlock = a.collision.isBlock && b.collision.isBlock;

  // 衝突形状ごとに処理を分ける.
  switch (a.collision.shape) {
  // アクターAが円柱の場合.
  case Collision::Shape::cylinder:
    switch (b.collision.shape) {
    case Collision::Shape::cylinder:
      return CollideCylinderAndCylinder(a, b, isBlock);
    case Collision::Shape::box:
      return CollideCylinderAndBox(a, b, isBlock);
    default:
      return false;
    }

  // アクターAが直方体の場合.
  case Collision::Shape::box:
    switch (b.collision.shape) {
    case Collision::Shape::cylinder:
      return CollideCylinderAndBox(b, a, isBlock);
    case Collision::Shape::box:
      return false;
    default:
      return false;
    }

  // アクターAがそれ以外の場合.
  default:
    return false;
  }
}

