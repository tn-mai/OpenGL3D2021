/**
* @file Actor.cpp
*/
#include "Actor.h"
#include "GameData.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>

/// モーフィングアニメーションの切り替えにかかる時間.
const float maxMorphTransitionTime = 0.2f;

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
  OnUpdate(deltaTime);

  // 寿命チェック.
  if (lifespan > 0) {
    // 寿命を減らした結果が0以下になったら死亡.
    lifespan -= deltaTime;
    if (lifespan <= 0) {
      lifespan = 0;
      isDead = true;
    }
  }

  // 死んでいたら更新終了.
  if (isDead) {
    return;
  }

  // タイマー減算.
  if (timer > 0) {
    timer = std::max(0.0f, timer - deltaTime);
  }

  // 移動速度に重力を加算.
  if (gravityScale) {
    velocity += GameData::Get().gravity * gravityScale * deltaTime;
  }

  // 摩擦による減速.
  if (drag) {
    const glm::vec3 v(velocity.x, 0, velocity.z);
    const float speed = glm::length(v);
    if (speed > 0) {
      if (speed > -drag * deltaTime) {
        velocity += glm::normalize(v) * drag * deltaTime;
      } else {
        velocity.x = velocity.z = 0;
      }
    }
    drag = 0;
  }

  // 座標を更新.
  position += velocity * deltaTime;

  // 衝突しそうな範囲を更新.
  const float vl = glm::length(velocity)* deltaTime * 2.0f;
  switch (collision.shape) {
  case Collision::Shape::cylinder:
    boundingBox.c = position;
    boundingBox.c.y += (collision.top + collision.bottom) * 0.5f;
    boundingBox.r.x = collision.radius + vl;
    boundingBox.r.y = (collision.top - collision.bottom) * 0.5f + vl;
    boundingBox.r.z = boundingBox.r.x;
    break;
  case Collision::Shape::box:
    boundingBox.c = position + (collision.boxMin + collision.boxMax) * 0.5f;
    boundingBox.r = (collision.boxMax - collision.boxMin) * 0.5f + vl;
    break;
  }

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

    // アニメーション切り替えタイマーを減算し、
    // 0以下になったら切り替え完了として直前のアニメーションを解除.
    if (morphTransitionTimer > 0) {
      morphTransitionTimer -= deltaTime;
      if (morphTransitionTimer <= 0) {
        // 直前のアニメーションを解除.
        prevMorphWeight = 0;
        prevBaseMesh = nullptr;
        prevMorphTarget = nullptr;
        morphTransitionTimer = 0;
      }
    }

    // モーフターゲットを更新.
    size_t nextAnimationNo = animationNo + 1;
    if (animation->isLoop) {
      nextAnimationNo %= animation->list.size();
    } else {
      nextAnimationNo = std::min(nextAnimationNo, animation->list.size() - 1);
    }
    morphTarget = animation->list[nextAnimationNo];
  }
}

/**
* アクターを描画する.
*
* @param pipeline 行列の設定先となるパイプラインオブジェクト.
* @param matVP    描画に使用するビュープロジェクション行列.
* @param drawType 描画の種類.
*/
void Actor::Draw(const Shader::Pipeline& pipeline, const glm::mat4& matVP,
  DrawType drawType) const
{
  // プリミティブが設定されていないときは何もせず終了.
  if (!primitive) {
    return;
  }

  // 影描画の時、影を作らないアクターは何もせず終了.
  if (drawType == DrawType::shadow && !isShadowCaster) {
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
  if (drawType == DrawType::color) {
    pipeline.SetObjectColor(baseColor);
    pipeline.SetModelMatrix(matModel);
  }
  pipeline.SetMVP(matVP * matModel);

  // ベースメッシュとモーフターゲットの合成比率を設定.
  if (animation) {
    pipeline.SetMorphWeight(glm::vec3(
      glm::clamp(animationTimer / animation->interval, 0.0f, 1.0f),
      prevMorphWeight,
      morphTransitionTimer / maxMorphTransitionTime));
  } else {
    pipeline.SetMorphWeight(glm::vec3(0));
  }

  const GameData& gamedata = GameData::Get();
  for (size_t i = 0; i < std::size(samplers); ++i) {
    if (samplers[i]) {
      samplers[i]->Bind(i);
    } else {
      gamedata.sampler.Bind(i);
    }
  }

  // テクスチャイメージスロット0番にテクスチャを割り当てる.
  texture->Bind(0);
  if (texNormal) {
    texNormal->Bind(1);
  } else {
    Texture::UnbindTexture(1);
  }
  if (texMetallicSmoothness) {
    texMetallicSmoothness->Bind(2);
  } else {
    Texture::UnbindTexture(2);
  }

  // プリミティブを描画.
  primitive->Draw(morphTarget, prevBaseMesh, prevMorphTarget);
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

  boundingBox.c = position;
  boundingBox.c.y += (collision.top + collision.bottom) * 0.5f;
  boundingBox.r.x = collision.radius;
  boundingBox.r.y = (collision.top - collision.bottom) * 0.5f;
  boundingBox.r.z = boundingBox.r.x;
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

  boundingBox.c = position + (collision.boxMin + collision.boxMax) * 0.5f;
  boundingBox.r = (collision.boxMax - collision.boxMin) * 0.5f;
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

  // 現在のアニメーションを直前のモーフィングデータとして記録
  if (this->animation) {
    prevMorphWeight = glm::clamp(animationTimer / this->animation->interval, 0.0f, 1.0f);
    prevBaseMesh = primitive;
    prevMorphTarget = morphTarget;
    morphTransitionTimer = maxMorphTransitionTime;
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

  // dead状態のアクターのOnDestroyを呼び出す.
  for (auto& e : actorList) {
    if (e->isDead) {
      e->OnDestroy();
    }
  }

  // dead状態のアクターを削除.
  const auto isDead = [](ActorPtr p) { return p->isDead; };
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
  const glm::mat4& matVP, Actor::DrawType drawType)
{
  GameData& global = GameData::Get();
  for (size_t i = 0; i < actorList.size(); ++i) {
    actorList[i]->Draw(*global.pipeline, matVP, drawType);
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
bool CollideCylinders(Actor& a, Actor& b, bool isBlock)
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

#if 1
  // 上下の押し返す距離が短い方向をpyとする.
  float py = 0;
  const float y0 = bottomB - topA;
  const float y1 = topB - bottomA;
  if (std::abs(y0) <= std::abs(y1)) {
    py = y0;
  } else {
    py = y1;
  }

  // Y方向の押し出す距離がXZ方向の押し出す距離より短ければY方向に押し出す.
  // 長ければXZ方向に押し出す.
  // 中心間の距離dを計算.
  const float d = std::sqrt(d2);
  // 押し返す距離sを計算.
  const float s = r - d;
  if (std::abs(py) < s) {
#if 0
    a.AddCollisionPenalty(glm::vec3(0, py * 0.5f, 0));
    b.AddCollisionPenalty(glm::vec3(0, -py * 0.5f, 0));
    if (py >= 0) {
      a.velocity.y = std::max(a.velocity.y, 0.0f);
    } else {
      b.velocity.y = std::max(b.velocity.y, 0.0f);
    }
    a.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
#else
    // 上方向が近い場合はAを上に移動.
    if (py >= 0) {
      a.position.y += topB - bottomA; // Aを上に移動.
      a.velocity.y = std::max(a.velocity.y, 0.0f);
      a.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
    } else {
      b.position.y += topA - bottomB; // Bを上に移動.
      b.velocity.y = std::max(b.velocity.y, 0.0f);
      b.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
    }
#endif
  } else {
    // AとBがぴったり重なっている場合は+X方向に押し出す.
    if (d2 < 0.00001f) {
      // アクターAとBを均等に押し返す.
      const glm::vec3 n(1, 0, 0);
      a.position += n * r * 0.5f;
      b.position -= n * r * 0.5f;
    } else {
      // 円柱の中心軸間の方向ベクトルnを計算.
      const glm::vec3 n(dx / d, 0, dz / d);
      // アクターAとBを均等に押し返す.
      a.position += n * s * 0.5f;
      b.position -= n * s * 0.5f;
    }
  }
#else
  // Y軸方向の重なっている部分の長さを計算.
  const float overlapY = std::min(topA, topB) - std::max(bottomA, bottomB);

  // 短いほうの円柱の高さの半分を計算.
  const float shortY = std::min(topA - bottomA, topB - bottomB);

  // 重なっている長さが短い方の円柱の高さの半分未満なら上に押し返す.
  // 半分以上なら横に押し返す.
  if (overlapY < shortY * 0.75f) {
    // 下端が高い位置にあるアクターを上に移動.
    if (bottomA > bottomB) {
      a.position.y += topB - bottomA; // Aを上に移動.
      a.velocity.y = std::max(a.velocity.y, 0.0f);
      a.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
    } else {
      b.position.y += topA - bottomB; // Bを上に移動.
      b.velocity.y = std::max(b.velocity.y, 0.0f);
      b.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
    }
  } else if (d2 < 0.0001f) {
    // アクターAとBを均等に押し返す.
    const glm::vec3 n(1, 0, 0);
    a.position += n * r * 0.5f;
    b.position -= n * r * 0.5f;
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
#endif
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

#if 1
  // 直方体の中心座標(cx, cz)を計算.
  const float cx = b.position.x +
    (b.collision.boxMin.x + b.collision.boxMax.x) * 0.5f;
  const float cz = b.position.z +
    (b.collision.boxMin.z + b.collision.boxMax.z) * 0.5f;

  // 直方体のX及びZ軸方向の中心座標からの長さ(hx, hz)を計算.
  const float hx = (b.collision.boxMax.x - b.collision.boxMin.x) * 0.5f;
  const float hz = (b.collision.boxMax.z - b.collision.boxMin.z) * 0.5f;

  // 直方体の中心座標から円柱の中心軸までの距離(ox, oz)を計算.
  const float ox = a.position.x - cx;
  const float oz = a.position.z - cz;

  // 押し返す距離pを計算.
  // 速度vが0でなければ移動方向に向かう面まで押し返す.
  // 0の場合は押し返す距離が短い方向を選択.
  const float r = a.collision.radius;
  glm::vec3 p = glm::vec3(0);

  if (ox < 0) {
    p.x = -(r + hx) - ox;
  } else {
    p.x = (r + hx) - ox;
  }

  const float y0 = bottomB - topA;
  const float y1 = topB - bottomA;
  if (std::abs(y0) <= std::abs(y1)) {
    p.y = y0;
  } else {
    p.y = y1;
  }
 
  if (oz < 0) {
    p.z = -(r + hz) - oz;
  } else {
    p.z = (r + hz) - oz;
  }

  // 押し出す距離が最も短い方向へ押し出す.
  const glm::vec3 absP = glm::abs(p);
  if (absP.x < absP.y) {
    if (absP.x < absP.z) {
      a.position.x += p.x;
      // 押し返す方向と逆向きに移動していたら速度を0にする.
      if (a.velocity.x * p.x < 0) {
        a.velocity.x = 0;
      }
    } else {
      a.position.z += p.z;
      // 押し返す方向と逆向きに移動していたら速度を0にする.
      if (a.velocity.z * p.z < 0) {
        a.velocity.z = 0;
      }
    }
  } else {
    if (absP.y < absP.z) {
      a.position.y += p.y;
      // 押し返す方向と逆向きに移動していたら速度を0にする.
      if (a.velocity.y * p.y < 0) {
        a.velocity.y = 0;
      }
      // 上に押し返す場合は摩擦を発生させ、落下していたらY要素を0にする.
      if (p.y >= 0) {
        a.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
      }
    } else {
      a.position.z += p.z;
      // 押し返す方向と逆向きに移動していたら速度を0にする.
      if (a.velocity.z * p.z < 0) {
        a.velocity.z = 0;
      }
    }
  }
  if (std::abs(a.position.x) > 100 || std::abs(a.position.y) > 100 || std::abs(a.position.z) > 100) {
    std::cerr << "[警告]" << __func__ << "計算エラー\n";
  }
#else
  // Y軸方向の重なっている部分の長さを計算.
  const float overlapY = std::min(topA, topB) - std::max(bottomA, bottomB);

  // 円柱と直方体のうち、短いほうの高さを計算.
  const float shortY = std::min(topA - bottomA, topB - bottomB);

  // 重なっている長さが円柱の半分未満なら上または下に押し返す.
  // 半分以上なら横に押し返す.
  if (overlapY < shortY * 0.5f) {
    // 円柱の下端が直方体の下端より高い位置にあるなら円柱を上に移動.
    // そうでなければ下に移動.
    if (bottomA > bottomB) {
      a.position.y += topB - bottomA;
      a.velocity.y = std::max(a.velocity.y, 0.0f);
      a.drag = (a.friction + b.friction) * 0.5f * GameData::Get().gravity.y;
    } else {
      a.position.y -= topA - bottomB; // 円柱vs円柱と違うので注意.
      a.velocity.y = std::min(a.velocity.y, 0.0f);
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
    const float hx = (b.collision.boxMax.x - b.collision.boxMin.x) * 0.5f;
    const float hz = (b.collision.boxMax.z - b.collision.boxMin.z) * 0.5f;

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
#endif

  return true;
}

/**
* 直方体同士の衝突を処理する.
*
* @param a 衝突形状が直方体のアクター.
* @param b 衝突形状が直方体のアクター.
* @param isBlock 貫通させない場合true. 貫通する場合false
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool CollideBoxAndBox(Actor& a, Actor& b, bool isBlock)
{
  // 直方体aの下端が直方体bの上端の上にあるなら衝突していない.
  const float bottomA = a.position.y + a.collision.boxMin.y;
  const float topB = b.position.y + b.collision.boxMax.y;
  if (bottomA >= topB) {
    return false;
  }
  // 直方体aの上端が直方体bの下端の下にあるなら衝突していない.
  const float topA = a.position.y + a.collision.boxMax.y;
  const float bottomB = b.position.y + b.collision.boxMin.y;
  if (topA <= bottomB) {
    return false;
  }

  // 直方体aの左端が直方体bの右端の右にあるなら衝突していない.
  const float leftA = a.position.x + a.collision.boxMin.x;
  const float rightB = b.position.x + b.collision.boxMax.x;
  if (leftA >= rightB) {
    return false;
  }
  // 直方体aの右端が直方体bの左端の左にあるなら衝突していない.
  const float rightA = a.position.x + a.collision.boxMax.x;
  const float leftB = b.position.x + b.collision.boxMin.x;
  if (rightA <= leftB) {
    return false;
  }

  // 直方体aの手前端が直方体bの奥端の奥にあるなら衝突していない.
  const float frontA = a.position.z + a.collision.boxMin.z;
  const float backB = b.position.z + b.collision.boxMax.z;
  if (frontA >= backB) {
    return false;
  }
  // 直方体aの上端が直方体bの下端の下にあるなら衝突していない.
  const float backA = a.position.z + a.collision.boxMax.z;
  const float frontB = b.position.z + b.collision.boxMin.z;
  if (backA <= frontB) {
    return false;
  }

  return true;
}

/**
* 線分と平面が交差する座標を求める.
*
*
*/
bool Intersect(const Segment& seg, const Plane& plane, glm::vec3* p)
{
  const float distance = glm::dot(plane.normal, plane.point - seg.start);
  const glm::vec3 v = seg.end - seg.start;

  // 分母がほぼ0の場合、線分は平面と平行なので交差しない.
  const float denom = glm::dot(plane.normal, v);
  if (std::abs(denom) < 0.0001f) {
    return false;
  }

  // 交点までの距離tが0未満または1より大きい場合、交点は線分の外側にあるので実際には交差しない.
  const float t = distance / denom;
  if (t < 0 || t > 1) {
    return false;
  }

  // 交点は線分上にある.
  *p = seg.start + v * t;
  return true;
}

/**
* 球が平面の表側にあるかどうかを調べる.
*
* @param sphere 球.
* @param plane  平面.
*
* @retval true  平面の表側にあるか、部分的に重なっている.
* @retval false 完全に裏側にある.
*/
bool SphereInsidePlane(const Sphere& sphere, const Plane& plane)
{
  const float d = glm::dot(plane.normal, sphere.center - plane.point);
  return d >= -sphere.radius;
}

/**
* 円錐が平面の表側にあるかどうかを調べる.
*
* @param cone   円錐.
* @param plane  平面.
*
* @retval true  平面の表側にあるか、部分的に重なっている.
* @retval false 完全に裏側にある.
*/
bool ConeInsidePlane(const Cone& cone, const Plane& plane)
{
  if (glm::dot(plane.normal, cone.tip - plane.point) >= 0) {
    return true;
  }
  // 平面の法線と円錐の向きに垂直なベクトルaを求める.
  // これは平面に平行なベクトルになる.
  const glm::vec3 a = glm::cross(plane.normal, cone.direction);

  // 円錐の向きとベクトルaに垂直なベクトルを求める.
  // これは底面の中心から平面への最短の向きベクトルになる.
  const glm::vec3 b = glm::normalize(glm::cross(cone.direction, a));

  // ベクトルbの方向の底面の端の座標を求める.
  const glm::vec3 q = cone.tip +
    cone.direction * cone.height + b * cone.radius;

  // ベクトルcの平面の法線方向に射影した長さが0以上ならqは表側にある.
  return glm::dot(plane.normal, q - plane.point) >= 0;
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
bool DetectCollision(Actor& a, Actor& b, bool block)
{
  // アクターAとアクターBの両方が通り抜け禁止なら押し返す
  const bool isBlock = block && a.collision.blockOtherActors && b.collision.blockOtherActors;

  // 衝突形状ごとに処理を分ける.
  switch (a.collision.shape) {
  // アクターAが円柱の場合.
  case Collision::Shape::cylinder:
    switch (b.collision.shape) {
    case Collision::Shape::cylinder:
      return CollideCylinders(a, b, isBlock);
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
      return CollideBoxAndBox(a, b, isBlock);
    default:
      return false;
    }

  // アクターAがそれ以外の場合.
  default:
    return false;
  }
}

