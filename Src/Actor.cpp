/**
* @file Actor.cpp
*/
#include "Actor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

/**
* コンストラクタ
*/
Actor::Actor(
  const std::string& name,
  const Primitive& prim,
  std::shared_ptr<Texture> tex,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment) :
  name(name),
  prim(prim),
  tex(tex),
  position(position),
  scale(scale),
  rotation(rotation),
  adjustment(adjustment)
{
}

/**
* アクターの状態を更新する
*
* @param deltaTime 前回の更新からの経過時間(秒)
*/
void Actor::OnUpdate(float deltaTime)
{
  // 何もしない
}

/**
* 衝突を処理する
*
* @param contact 衝突情報
*/
void Actor::OnCollision(const struct Contact& contact)
{
  // 何もしない
}

/**
* 物体を描画する.
*/
void Draw(
  const Actor& actor,              // 物体の制御パラメータ
  const ProgramPipeline& pipeline, // 描画に使うプログラムパイプライン
  glm::mat4 matProj,               // 描画に使うプロジェクション行列
  glm::mat4 matView)               // 描画に使うビュー行列  
{
  // モデル行列を計算する
  glm::mat4 matT = glm::translate(glm::mat4(1), actor.position);
  glm::mat4 matR = glm::rotate(glm::mat4(1), actor.rotation, glm::vec3(0, 1, 0));
  glm::mat4 matS = glm::scale(glm::mat4(1), actor.scale);
  glm::mat4 matA = glm::translate(glm::mat4(1), actor.adjustment);
  glm::mat4 matModel = matT * matR * matS * matA;

  // MVP行列を計算する
  glm::mat4 matMVP = matProj * matView * matModel;

  // モデル行列とMVP行列をGPUメモリにコピーする
  const GLint locMatTRS = 0;
  const GLint locMatModel = 1;
  pipeline.SetUniform(locMatTRS, matMVP);
  if (actor.layer == Layer::Default) {
    pipeline.SetUniform(locMatModel, matModel);
  }

  // TODO: テキスト未追加
  const GLint locColor = 100;
  pipeline.SetUniform(locColor, actor.color);

  if (actor.tex) {
    actor.tex->Bind(0); // テクスチャを割り当てる
  } else {
  }
  actor.prim.Draw();  // プリミティブを描画する
}

/**
* 名前の一致するアクターを検索する.
*
* @param actors 検索対象の配列.
* @param name   検索するアクターの名前.
*
* @retval nullptr以外 最初にnameと名前の一致したアクター.
* @retval nullptr     actorsの中に名前の一致するアクターがない.
*/
std::shared_ptr<Actor> Find(std::vector<std::shared_ptr<Actor>>& actors, const char* name)
{
  for (int i = 0; i < actors.size(); ++i) {
    if (actors[i]->name == name) {
      return actors[i];
    }
  }
  return nullptr;
}

/**
* AとBを入れ替える.
*/
Contact Reverse(const Contact& contact)
{
  Contact result;

  result.a = contact.b;
  result.b = contact.a;
  result.velocityA = contact.velocityB;
  result.velocityB = contact.velocityA;
  result.accelA = contact.accelB;
  result.accelB = contact.accelA;
  result.penetration = -contact.penetration;
  result.normal = -contact.normal;
  result.position = contact.position;
  result.penLength = contact.penLength;
  result.massB = contact.a->mass;

  return result;
}

bool DummyCollisionFunc(Actor&, Actor&, Contact&)
{
  return false;
}

/**
* 衝突を検出する
*
* @param actorA  衝突しているか調べるアクター
* @param actorB  衝突しているか調べるアクター
* @param contact 衝突情報
*
* @retval true  衝突している
* @retval false 衝突していない
*/
bool DetectCollision(Actor& actorA, Actor& actorB, Contact& contact)
{
  // 質量を持たない物体同士は衝突しない
  if (actorA.mass <= 0 && actorB.mass <= 0) {
    return false;
  }

  // 動かせない物体同士は衝突しない
  if (actorA.isStatic && actorB.isStatic) {
    return false;
  }

  // コライダーが設定されていない物体は衝突しない
  if (!actorA.collider || !actorB.collider) {
    return false;
  }

  using CollisionFunc = bool(*)(Actor&, Actor&, Contact&);
  static const CollisionFunc funcArray[2][2] = {
    //              box, cylinder
    /* box      */ { CollisionBoxBox, CollisionBoxCylinder },
    /* cylinder */ { CollisionCylinderBox, CollisionCylinderCylinder },
  };
  const int y = static_cast<int>(actorA.collider->shapeType);
  const int x = static_cast<int>(actorB.collider->shapeType);
  return funcArray[y][x](actorA, actorB, contact);
}

bool CollisionBoxBox(Actor& actorA, Actor& actorB, Contact& contact)
{
  // ワールド座標系の衝突図形を計算する
  Box a = static_cast<Box&>(*actorA.collider);
  a.min += actorA.position;
  a.max += actorA.position;

  Box b = static_cast<Box&>(*actorB.collider);
  b.min += actorB.position;
  b.max += actorB.position;

  // aの左側面がbの右側面より右にあるなら、衝突していない
  const float dx0 = b.max.x - a.min.x;
  if (dx0 <= 0) {
    return false;
  }
  // aの右側面がbの左側面より左にあるなら、衝突していない
  const float dx1 = a.max.x - b.min.x;
  if (dx1 <= 0) {
    return false;
  }

  // aの下面がbの上面より上にあるなら、衝突していない
  const float dy0 = b.max.y - a.min.y;
  if (dy0 <= 0) {
    return false;
  }
  // aの上面がbの下面より下にあるなら、衝突していない
  const float dy1 = a.max.y - b.min.y;
  if (dy1 <= 0) {
    return false;
  }

  // aの奥側面がbの手前側面より手前にあるなら、衝突していない
  const float dz0 = b.max.z - a.min.z;
  if (dz0 <= 0) {
    return false;
  }
  // aの手前側面がbの奥側面より奥にあるなら、衝突していない
  const float dz1 = a.max.z - b.min.z;
  if (dz1 <= 0) {
    return false;
  }

  // どちらか、または両方のアクターが「ブロックしない」場合、接触処理を行わない
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

#if 0
  return true;
#endif

  // XYZの各軸について重なっている距離が短い方向を選択する
  glm::vec3 normal;  // 衝突面(アクターBのいずれかの面)の法線
  glm::vec3 penetration; // 重なっている距離と方向
  if (dx0 <= dx1) {
    penetration.x = -dx0;
    normal.x = 1;
  } else {
    penetration.x = dx1;
    normal.x = -1;
  }
  if (dy0 <= dy1) {
    penetration.y = -dy0;
    normal.y = 1;
  } else {
    penetration.y = dy1;
    normal.y = -1;
  }
  if (dz0 <= dz1) {
    penetration.z = -dz0;
    normal.z = 1;
  } else {
    penetration.z = dz1;
    normal.z = -1;
  }

  // 重なっている距離の絶対値
  glm::vec3 absPenetration = abs(penetration);

  // 衝突面になる可能性の高さ
  glm::vec3 score = glm::vec3(0);

  // 浸透距離がより短い方向ほど衝突面である可能性が高いはず
  for (int a = 0; a < 2; ++a) {
    for (int b = a + 1; b < 3; ++b) {
      if (absPenetration[a] < absPenetration[b]) {
        ++score[a];
      } else {
        ++score[b];
      }
    }
  }

#if 1
  // 相対ベロシティを計算する
  glm::vec3 rv = actorA.velocity - actorB.velocity;

  // 浸透が始まった時間tを計算する
  glm::vec3 t(-FLT_MAX);
  for (int i = 0; i < 3; ++i) {
    if (rv[i]) {
      t[i] = penetration[i] / rv[i];
    }
  }

  // 浸透に必要な時間tが長いほど、他の方向より早い時点で衝突したと考えられる
  // ただし、tが0秒未満の場合、衝突面から遠ざかる方向に移動しているため除外する
  // また、tが1/60秒より長い場合、本来なら衝突していないはずなので除外する
  float deltaTime = 1.0f / 60.0f;
  for (int a = 0; a < 2; ++a) {
    for (int b = a + 1; b < 3; ++b) {
      int i = a;
      if (t[a] < t[b]) {
        i = b;
      }
      if (t[i] > 0 && t[i] <= deltaTime) {
        score[i] += 1.5f;
      }
    }
  }
#endif

  // より可能性が低い方向を除外する
  // 値が等しい場合、Z,X,Yの順で優先的に除外する
  if (score.x <= score.y) {
    normal.x = 0;
    penetration.x = 0;
    if (score.z <= score.y) {
      normal.z = 0;
      penetration.z = 0;
    } else {
      normal.y = 0;
      penetration.y = 0;
    }
  } else {
    normal.y = 0;
    penetration.y = 0;
    if (score.z <= score.x) {
      normal.z = 0;
      penetration.z = 0;
    } else {
      normal.x = 0;
      penetration.x = 0;
    }
  }

#if 0
  // XYZ軸のうち、浸透距離が最も短い軸の成分だけを残す
  // NOTE: 浸透方向と接触面の法線は一致しない場合がある
  if (absPenetration.x >= absPenetration.y) {
    penetration.x = 0;
    if (absPenetration.z >= absPenetration.y) {
      penetration.z = 0;
    } else {
      penetration.y = 0;
    }
  } else {
    penetration.y = 0;
    if (absPenetration.x >= absPenetration.z) {
      penetration.x = 0;
    } else {
      penetration.z = 0;
    }
  }
#endif

  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;
  contact.penetration = penetration;
  contact.normal = normal;
  contact.massB = actorB.mass;

  // 衝突面の座標を計算する
  {
    // 基本的にアクターBの座標を使うが、アクターBが静物の場合はアクターAの座標を使う
    Actor* target = &actorB;
    glm::vec3 targetNormal = normal;
    if (actorB.isStatic) {
      target = &actorA;
      targetNormal *= -1; // 法線の向きを反転する
    }
    // コライダーの半径を計算する
    const Box& targetBox = static_cast<Box&>(*target->collider);
    glm::vec3 halfSize = (targetBox.max - targetBox.min) * 0.5f;
    // コライダーの中心座標を計算する
    glm::vec3 center = (targetBox.max + targetBox.min) * 0.5f;
    // 衝突面の座標を計算する
    contact.position = target->position + center - halfSize * targetNormal;
  }

  // 浸透距離の長さを計算する
  contact.penLength = glm::length(penetration);

  // 衝突している
  return true;
}

/**
* 円柱と円柱の衝突
*/
bool CollisionCylinderCylinder(Actor& actorA, Actor& actorB, Contact& contact)
{
  // ワールド座標系の衝突図形を計算する
  Cylinder a = static_cast<Cylinder&>(*actorA.collider);
  a.bottom += actorA.position;

  Cylinder b = static_cast<Cylinder&>(*actorB.collider);
  b.bottom += actorB.position;

  // aの下面がbの上面より上にあるなら、衝突していない
  const float dy0 = (b.bottom.y + b.height) - a.bottom.y;
  if (dy0 <= 0) {
    return false;
  }
  // aの上面がbの下面より下にあるなら、衝突していない
  const float dy1 = (a.bottom.y + a.height) - b.bottom.y;
  if (dy1 <= 0) {
    return false;
  }

  // XZ平面上の距離が半径の合計より遠ければ、衝突していない
  const float dx = b.bottom.x - a.bottom.x;
  const float dz = b.bottom.z - a.bottom.z;
  const float d2 = dx * dx + dz * dz;
  const float r = a.radius + b.radius;
  if ( d2 > r * r) {
    return false;
  }

  // どちらか、または両方のアクターが「ブロックしない」場合、接触処理を行わない
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

  // XZ方向の浸透距離を計算する
  glm::vec3 normal(0);
  glm::vec3 penetration(0);
  const float d = std::sqrt(d2);
  const float invD = 1.0f / d;
  normal.x = -dx * invD;
  normal.z = -dz * invD;
  const float lengthXZ = r - d;
  penetration.x = -lengthXZ * normal.x;
  penetration.z = -lengthXZ * normal.z;

  // Y方向の浸透距離を計算する
  if (dy0 < dy1) {
    penetration.y = -dy0;
    normal.y = 1;
  } else {
    penetration.y = dy1;
    normal.y = -1;
  }

  // 浸透距離の短い方向から衝突したとみなす
  if (std::abs(penetration.y) <= lengthXZ) {
    // Y方向の衝突
    penetration.x = penetration.z = 0;
    normal.x = normal.z = 0;
  } else {
    // XZ方向の衝突
    penetration.y = 0;
    normal.y = 0;
  }

  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;
  contact.penetration = penetration;
  contact.normal = normal;
  contact.massB = actorB.mass;

  // 衝突面の座標を計算する
  {
    // 基本的にアクターBの座標を使うが、アクターBが静物の場合はアクターAの座標を使う
    Actor* target = &actorB;
    Cylinder* targetCollider = &b;
    glm::vec3 targetNormal = normal;
    if (actorB.isStatic) {
      target = &actorA;
      targetCollider = &a;
      targetNormal *= -1; // 法線の向きを反転する
    }

    // 衝突面の座標を計算する
    if (normal.y) {
      // Y方向の衝突の場合・・・
      if (targetNormal.y < 0) {
        contact.position.y = targetCollider->bottom.y;
      } else {
        contact.position.y = targetCollider->bottom.y + targetCollider->height;
      }
      contact.position.x = (a.bottom.x + b.bottom.x) * 0.5f;
      contact.position.z = (a.bottom.z + b.bottom.z) * 0.5f;
    } else {
      // XZ方向の衝突の場合・・・
      contact.position.y = targetCollider->bottom.y + targetCollider->height * 0.5f;
      contact.position.x = targetCollider->bottom.x - targetNormal.x * targetCollider->radius;
      contact.position.z = targetCollider->bottom.z - targetNormal.z * targetCollider->radius;
    }
  }

  // 浸透距離の長さを計算する
  contact.penLength = glm::length(penetration);

  // 衝突している
  return true;
}

/**
* 点に最も近い直方体内の座標(最近接点)を求める
*/
glm::vec3 ClosestPointXZ(const Box& box, const glm::vec3& p)
{
  const float cx = glm::clamp(p.x, box.min.x, box.max.x);
  const float cz = glm::clamp(p.z, box.min.z, box.max.z);
  return glm::vec3(cx, p.y, cz);
}

/**
*
*/
bool CollisionBoxCylinder(Actor& actorA, Actor& actorB, Contact& contact)
{
  // ワールド座標系の衝突図形を計算する
  Box a = static_cast<Box&>(*actorA.collider);
  a.min += actorA.position;
  a.max += actorA.position;

  Cylinder b = static_cast<Cylinder&>(*actorB.collider);
  b.bottom += actorB.position;

  // aの下面がbの上面より上にあるなら、衝突していない
  const float dy0 = (b.bottom.y + b.height) - a.min.y;
  if (dy0 <= 0) {
    return false;
  }
  // aの上面がbの下面より下にあるなら、衝突していない
  const float dy1 = a.max.y - b.bottom.y;
  if (dy1 <= 0) {
    return false;
  }

  // 円柱の中心に最も近い直方体内の座標(最近接点)を求める
  const glm::vec3 closestPoint = ClosestPointXZ(a, b.bottom);

  // 円柱の中心から最近接点までの距離を計算する
  const float dx = closestPoint.x - b.bottom.x;
  const float dz = closestPoint.z - b.bottom.z;
  const float d2 = dx * dx + dz * dz;

  // 最近接点までの距離が円柱の半径より長ければ、衝突していない
  if (d2 > b.radius * b.radius) {
    return false;
  }

  // どちらか、または両方のアクターが「ブロックしない」場合、接触処理を行わない
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

  // Y方向の浸透距離を計算する
  glm::vec3 penetration(0);
  glm::vec3 normal(0);
  if (dy0 < dy1) {
    penetration.y = -dy0;
    normal.y = 1;
  } else {
    penetration.y = dy1;
    normal.y = -1;
  }

  // 最近接点cが直方体のカド(角)にある場合、角を基準に浸透距離を計算する
  bool isCorner = false;
  if ((closestPoint.x == a.min.x || closestPoint.x == a.max.x) &&
    (closestPoint.z == a.min.z || closestPoint.z == a.max.z)) {
    isCorner = true;
    const float d = std::sqrt(d2);
    const float invD = 1.0f / d;
    normal.x = dx * invD;
    normal.z = dz * invD;
    const float lengthXZ = b.radius - d;
    penetration.x = -lengthXZ * normal.x;
    penetration.z = -lengthXZ * normal.z;
  } else {
    // X方向の浸透距離を計算する
    const float dx0 = (b.bottom.x + b.radius) - a.min.x;
    const float dx1 = (b.bottom.x - b.radius) - a.max.x;
    if (dx0 <= -dx1) {
      penetration.x = -dx0;
      normal.x = 1;
    } else {
      penetration.x = -dx1;
      normal.x = -1;
    }

    // Z方向の浸透距離を計算する
    const float dz0 = (b.bottom.z + b.radius) - a.min.z;
    const float dz1 = (b.bottom.z - b.radius) - a.max.z;
    if (dz0 <= -dz1) {
      penetration.z = -dz0;
      normal.z = 1;
    } else {
      penetration.z = -dz1;
      normal.z = -1;
    }
  }

  // 浸透距離が長い方向を除外する
  // コーナーで衝突している場合、XZベクトルの長さとYを比較する
  const glm::vec3 absPenetration = glm::abs(penetration);
  if (isCorner && absPenetration.y > glm::length(glm::vec2(penetration.x, penetration.z))) {
    penetration.y = 0;
    normal.y = 0;
  } else {
    // コーナー以外の場合、最も浸透距離が短い軸を残す
    if (absPenetration.x >= absPenetration.y) {
      normal.x = 0;
      penetration.x = 0;
      if (absPenetration.z >= absPenetration.y) {
        normal.z = 0;
        penetration.z = 0;
      } else {
        normal.y = 0;
        penetration.y = 0;
      }
    } else {
      normal.y = 0;
      penetration.y = 0;
      if (absPenetration.x >= absPenetration.z) {
        normal.x = 0;
        penetration.x = 0;
      } else {
        normal.z = 0;
        penetration.z = 0;
      }
    }
  }

  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;
  contact.penetration = penetration;
  contact.normal = normal;
  contact.massB = actorB.mass;

  // 衝突面の座標を計算する
  {
    // 基本的にアクターBの座標を使うが、アクターBが静物の場合はアクターAの座標を使う
    if (actorB.isStatic) {
      if (normal.y) {
        contact.position.x = closestPoint.x;
        contact.position.y = (a.min.y + a.max.y) * 0.5f - (a.max.y - a.min.y) * 0.5f * normal.y;
        contact.position.z = closestPoint.z;
      } else {
        contact.position.x = (a.min.x + a.max.x) * 0.5f - (a.max.x - a.min.x) * 0.5f * normal.x;
        contact.position.y = (a.min.y + a.max.x) * 0.5f;
        contact.position.z = (a.min.z + a.max.z) * 0.5f - (a.max.z - a.min.z) * 0.5f * normal.z;
      }
    } else {
      // 衝突面の座標を計算する
      if (normal.y) {
        // Y方向の衝突の場合・・・
        contact.position.x = closestPoint.x;
        contact.position.y = b.bottom.y + b.height * (0.5f + 0.5f * normal.y);
        contact.position.z = closestPoint.z;
      } else {
        // XZ方向の衝突の場合・・・
        contact.position.x = b.bottom.x - normal.x * b.radius;
        contact.position.y = a.min.y + dy0 * 0.5f;
        contact.position.z = b.bottom.z - normal.z * b.radius;
      }
    }
  }

  // 浸透距離の長さを計算する
  contact.penLength = glm::length(penetration);

  // 衝突している
  return true;
}

/**
*
*/
bool CollisionCylinderBox(Actor& actorA, Actor& actorB, Contact& contact)
{
  const bool result = CollisionBoxCylinder(actorB, actorA, contact);
  if (result) {
    //std::swap(contact.a, contact.b);
    //std::swap(contact.velocityA, contact.velocityB);
    //std::swap(contact.accelA, contact.accelB);
  }
  return result;
}


/**
* 重なりを解決する
*
* @param contact 衝突情報

@todo contactをアクター毎に生成(つまり1衝突につき2個)
@todo アクター及び法線の等しいcontactを統合(Equal関数の修正)
*/
void SolveContact(Contact& contact)
{
  Actor& actorA = *contact.a;
  Actor& actorB = *contact.b;
  glm::vec3 normal = contact.normal;
  glm::vec3 penetration = contact.penetration;

  // 反発係数の式 e = (Va' - Vb') / (Vb - Va) から e = 1 のとき
  //   Va' - Vb' = Vb - Va ...式(1)
  // 運動量保存則より
  //   Ma*Va + Mb*Vb = Ma*Va' + Mb*Vb' ...式(2)
  // 式(1)をVb'について解くと
  //  -Vb' = Vb - Va - Va'
  //   Vb' = -Vb + Va + Va'
  //   Vb' = Va - Vb + Va' ...式(3)
  // 式(3)を式(2)に代入すると
  //   Ma*Va + Mb*Vb = Ma*Va' + Mb(Va - Vb + Va') ...式(4)
  // 式(4)をVa'について解くと
  //   Ma*Va + Mb*Vb = Ma*Va' + Mb*Va - Mb*Vb + Mb*Va'
  //   Ma*Va + Mb*Vb = (Ma+Mb)Va' + Mb*Va - Mb*Vb
  //   Ma*Va + Mb*Vb - (Ma+Mb)Va' = Mb*Va - Mb*Vb
  //   -(Ma+Mb)Va' = Mb*Va - Mb*Vb - Ma*Va - Mb*Vb
  //   (Ma+Mb)Va' = -Mb*Va + Mb*Vb + Ma*Va + Mb*Vb
  //   (Ma+Mb)Va' = Ma*Va + Mb*Vb + Mb(Vb - Va)
  //   Va' = (Ma*Va + Mb*Vb + Mb(Vb - Va)) / (Ma+Mb) ...式(5)
  //
  // 同様にVb'について解くと
  //   Ma*Va + Mb*Vb = Ma*Vb - Ma*Va + Ma*Vb' + Mb*Vb'
  //   Ma*Va + Mb*Vb - Ma*Vb + Ma*Va = (Ma+Mb)Vb'
  //   Vb' = (Ma*Va + Mb*Vb + Ma(Va - Vb)) / (Ma+Mb)
  //
  // Aが静止物の場合、Bの質量Mbを0として式(5)を解く. すると
  //   Va' = (Ma*Va + 0*Vb + 0(Vb - Va)) / (Ma+0)
  //   Va' = Ma*Va / Ma
  //   Va' = Va
  //
  // NOTE: 摩擦と反発は非常に複雑な物理現象なので、そもそも式を立てることは不可能に近い.
  //       そのため、物理エンジンでは物理的な正確性を無視して適当に計算している.
  //       Bullet3は乗算、UE4、Unityはデフォルトでは平均を使っているようだ.
  //       Box2D-liteでは乗算して平方根を取っている. 重くなるが結果を制御しやすいようだ.
  //       今回は長いものに巻かれることにして、平均を採用する.

  // 反発係数の平均値を計算
  float cor = (actorA.cor + actorB.cor) * 0.5f;

  // 摩擦係数の平均値を計算
  float friction = (actorA.friction + actorB.friction) * 0.5f;

  // アクターBに対するアクターAの相対ベロシティを計算
  glm::vec3 rv = contact.velocityA - contact.velocityB;

  // 衝突面と相対ベロシティに平行なベクトル(タンジェント)を計算
  glm::vec3 tangent = glm::cross(normal, glm::cross(normal, rv));
  if (glm::length(tangent) > 0.000001f) {
    tangent = glm::normalize(tangent);
  } else {
    tangent = glm::vec3(0);
  }
  
  // 摩擦から受けるベロシティを計算
#if 1
  // 摩擦力
  float frictionForce = friction * 9.8f / 60.0f;

  // 摩擦力の最大値を計算
  float maxForce = std::abs(glm::dot(tangent, rv));

  // 摩擦力を最大値に制限
  frictionForce = std::min(frictionForce, maxForce);

  // タンジェント方向の摩擦力を計算
  glm::vec3 frictionVelocity = normal.y * frictionForce * tangent;
#else
  glm::vec3 relAccel = (contact.accelA - contact.accelB);
  float maxf = abs(glm::dot(tangent, rv));
  float tmp = glm::dot(normal, relAccel);
  float ff = std::min(maxf, abs(tmp));
  if (tmp < 0) {
    ff *= -1;
  }
  glm::vec3 frictionVelocity = friction * ff * tangent;
#endif

  // 速度の更新: 法線方向の速度を計算
  float ua = glm::dot(normal, contact.velocityA);
  float ub = glm::dot(normal, contact.velocityB);
  if (actorA.isStatic) {
#if 0
    const float ratio = 1.0f / actorB.contactCount;
    float vb = ua + cor * (ua - ub);     // 衝突後の速度を計算
    actorB.velocity -= normal * ub * ratio;      // 衝突前の速度を0にする
    actorB.velocity += normal * vb * ratio;      // 衝突後の速度を加算する
    actorB.velocity += frictionVelocity * ratio; // 摩擦による速度を加算する

    // 重なりの解消: アクターAは動かないので、アクターBを動かす
    actorB.position += penetration * ratio;
    if (normal.y < 0) {
      //float vy = actorB.velocity.y - actorA.velocity.y;
      //if ( vy < 1.0f) {
        actorB.isOnActor = true;
      //}
    }
#endif
  }
  else if (actorB.isStatic) {
    const float ratio = 1.0f / actorA.contactCount;
    float va = ub + cor * (ub - ua);     // 衝突後の速度を計算
    actorA.velocity -= normal * ua * ratio;      // 衝突前の速度を0にする
    actorA.velocity += normal * va * ratio;      // 衝突後の速度を加算する
    actorA.velocity += frictionVelocity * ratio; // 摩擦による速度を加算する

    // 重なりの解消: アクターBは動かないので、アクターAを動かす
    actorA.position -= penetration * ratio;

    if (normal.y > 0) {
      //float vy = actorA.velocity.y - actorB.velocity.y;
      //if ( vy < 1.0f) {
        actorA.isOnActor = true;
      //}
    }
  }
  else {
    // 速度の更新: 運動エネルギーの分配量を計算
    const float ratioA = 1.0f / actorA.contactCount;
    const float ratioB = 1.0f;// / actorB.contactCount;
    float massA = actorA.mass * ratioA;
    float massB = contact.massB;// actorB.mass* ratioB;
    float massAB = massA + massB;
    float c = massA * ua + massB * ub;
    float va = (c + cor * massB * (ub - ua)) / massAB;
    float vb = (c + cor * massA * (ua - ub)) / massAB;

    // 衝突前の速度を0にする
    actorA.velocity -= normal * ua;
    //actorB.velocity -= normal * ub;

    // 衝突後の速度を加算する
    actorA.velocity += normal * va;
    //actorB.velocity += normal * vb;

    // 摩擦による速度を加算する
    actorA.velocity += frictionVelocity * ratioA;
    //actorB.velocity -= frictionVelocity * ratioB;

    // 重なりの解消: 運動エネルギーの比率で動かす距離を決める
    //              運動エネルギーがゼロの場合は質量の比率で決める
    float rA = abs(va);
    float rB = abs(vb);
    if (rA <= 0.0f && rB <= 0.0f) {
      rA = massA;
      rB = massB;
    }
    glm::vec3 pa = penetration * rA / (rA + rB);
    glm::vec3 pb = penetration * rB / (rA + rB);
    actorA.position -= pa * ratioA;
    //actorB.position += pb * ratioB;

    if (normal.y > 0) {
      //float vy = actorA.velocity.y - actorB.velocity.y;
      //if ( vy < 1.0f) {
        actorA.isOnActor = true;
      //}
    } else if (normal.y < 0) {
      //float vy = actorB.velocity.y - actorB.velocity.y;
      //if ( vy < 1.0f) {
//        actorB.isOnActor = true;
      //}
    }
  }
}

/**
* 2つのコンタクト構造体が似ているか調べる
*
* @param ca 比較するコンタクト構造体その1.
* @param cb 比較するコンタクト構造体その2.
*
* @return true  似ている
* @return false 似ていない
*/
bool Equal(const Contact& ca, const Contact& cb)
{
  // 衝突面の距離が離れている場合は似ていない
  // NOTE: この判定は完全ではない.
  //       DetectCollisionは引数で渡された2つのアクターA, Bのうち、Bの表面に接触点を定義する.
  //       そのため、アクターXとアクターYがあったとき、A=X, B=YとするかA=Y, B=Xとするかで接触点と法線が異なる.
  //       すべての交点に接触点を生成すると、この問題を解決できるかもしれない.
  if (glm::length(ca.position - cb.position) > 0.01f) {
    return false; // 似ていない
  }

#if 0 // 現在の実装では座標が一致するなら法線も一致する
  // 法線の方向が一致しない場合は似ていない
  if (glm::dot(ca.normal, normalB) <= 0) {
    return false; // 似ていない
  }
#endif

  // 静物アクターの有無によって判定を分ける
  glm::vec3 normalB = cb.normal;
  bool hasStaticA = ca.a->isStatic || ca.b->isStatic;
  bool hasStaticB = cb.a->isStatic || cb.b->isStatic;
  switch (hasStaticA + hasStaticB * 2) {
  case 0b00: // A,Bともに動くアクターのみ
    // アクターが両方とも一致したら似ている
    if (ca.a == cb.a && ca.b == cb.b) {
      break;
    }
    if (ca.a == cb.b && ca.b == cb.a) {
      break;
    }
    return false;

  case 0b01: // A=動かないアクターを含む, B=動くアクターのみ
    // 常に似ていないと判定する
    return false;

  case 0b10: // A=動くアクターのみ B=動かないアクターを含む
    // 常に似ていないと判定する
    return false;

  case 0b11: // A,Bともに動かないアクターを含む 
    {
    // 動くアクター同士が一致したら似ている
    Actor* a = ca.a;
    if (ca.a->isStatic) {
      a = ca.b;
    }
    Actor* b = cb.a;
    if (cb.a->isStatic) {
      b = cb.b;
    }
    if (a == b) {
      break;
    }
    }
    return false;
  }

  return true; // 似ている
}

/**
* Contact構造体をA,Bそれぞれに作成する場合の比較関数.
* 
* アクターAと法線が等しい場合にtrue
* つまり、Aに同じ方向から接触した場合は等しいと判定する.
*/
bool Equal2(const Contact& ca, const Contact& cb)
{
  if (ca.a != cb.a) {
    return false;
  }
  if (glm::dot(ca.normal, cb.normal) <= glm::cos(glm::radians(1.0f))) {
    return false;
  }
  return true;
}

