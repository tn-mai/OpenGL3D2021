/**
* @file Collision.cpp
*/
#include "Collision.h"
#include "Actor.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// 浮動小数点数の0とみなす値
static const float epsilon = FLT_EPSILON * 16;

/**
* 衝突判定を回転させる
*/
void Box::RotateY(float radians)
{
  const glm::mat3 matRot =
    glm::rotate(glm::mat4(1), radians, glm::vec3(0, 1, 0));
  const glm::vec3 a = matRot * min;
  const glm::vec3 b = matRot * max;
  min = glm::min(a, b);
  max = glm::max(a, b);
}

/**
* 直方体と直方体の衝突
*/
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
* 球と球の衝突
*/
bool CollisionSphereSphere(Actor& actorA, Actor& actorB, Contact& contact)
{
  // ワールド座標系の衝突図形を計算する
  Sphere a = static_cast<Sphere&>(*actorA.collider);
  a.center += actorA.position;

  Sphere b = static_cast<Sphere&>(*actorB.collider);
  b.center += actorB.position;

  // 中心間の距離が半径の合計より大きければ、衝突していない
  const glm::vec3 v = b.center - a.center;
  const float d2 = glm::dot(v, v);
  const float r = a.radius + b.radius;
  if (d2 > r * r) {
    return false;
  }
  
  // どちらか、または両方のアクターが「ブロックしない」場合、接触処理を行わない
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

  const float distance = std::sqrt(d2);
  const glm::vec3 normal = v * (1.0f / distance);
  const float penetration = (a.radius + b.radius) - distance;

  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;
  contact.penetration = normal * penetration;
  contact.normal = normal;

  // 交差した座標を衝突点とする
  const glm::vec3 rv = actorA.velocity - actorB.velocity;
  const float nv = glm::dot(rv, normal);
  contact.position = actorA.position + normal * (a.radius + nv);

  // 衝突している
  return true;
}

/**
* 直方体の頂点座標を取得する
*/
glm::vec3 Corner(const Box& box, int flag)
{
  glm::vec3 c = box.min;
  if (flag & 1) {
    c.x = box.max.x;
  }
  if (flag & 2) {
    c.y = box.max.y;
  }
  if (flag & 4) {
    c.z = box.max.z;
  }
  return c;
}

/**
* 球と直方体の衝突
*/
bool CollisionBoxSphere(Actor& actorA, Actor& actorB, Contact& contact)
{
  // ワールド座標系の衝突図形を計算する
  Box a = static_cast<Box&>(*actorA.collider);
  a.min += actorA.position;
  a.max += actorA.position;

  Sphere b = static_cast<Sphere&>(*actorB.collider);
  b.center += actorB.position;

  // 球の中心に最も近い直方体内の座標(最近接点)を求める
  const glm::vec3 closestPoint = glm::clamp(b.center, a.min, a.max);

  // 球の中心から最近接点までの距離を計算する
  const glm::vec3 v = closestPoint - b.center;
  const float d2 = glm::dot(v, v);

  // 最近接点までの距離が球の半径より長ければ、衝突していない
  if (d2 > b.radius * b.radius) {
    return false;
  }

  // どちらか、または両方のアクターが「ブロックしない」場合、接触処理を行わない
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

  // 最近接点が頂点、辺、その他のどこにあるかによって、衝突面の計算を分ける
  int flagMin = 0;
  int flagMax = 0;
  for (int i = 0; i < 3; ++i) {
    if (closestPoint[i] <= a.min[i]) {
      flagMin |= 1 << i;
    }
    if (closestPoint[i] >= a.max[i]) {
      flagMax |= 1 << i;
    }
  }
  const int flag = flagMin | flagMax;

  glm::vec3 normal;
  if (flag) {
    // 最近接点が直方体の表面にある場合、最近接点と球を結ぶ直線を法線とする
    normal = closestPoint - b.center;
    if (glm::dot(normal, normal) < epsilon) {
      // 最近接点と球の中心が近すぎると法線を計算できない
      // かわりに最近接点が含まれる面から求める
      for (int i = 0; i < 3; ++i) {
        if (flagMin & (1 << i)) {
          normal[i] = 1;
        } else if (flagMax & (1 << i)) {
          normal[i] = -1;
        }
      }
    }
    normal = glm::normalize(normal);
  }
  else {
    // 最近接点が直方体の内側にある場合、ベロシティから衝突の可能性がある面のうち、最も近い面で衝突したとみなす
    const glm::vec3 rv = actorA.velocity - actorB.velocity;
    const bool noVelocity = glm::dot(rv, rv) < epsilon;
    float dmin = FLT_MAX;
    int face = 0; // 最も近い面
    for (int i = 0; i < 3; ++i) {
      if (rv[i] < 0 || noVelocity) {
        float d = closestPoint[i] - a.min[i];
        if (d < dmin) {
          dmin = d;
          face = i;
        }
      } else if (rv[i] > 0 || noVelocity) {
        float d = a.max[i] - closestPoint[i];
        if (d < dmin) {
          dmin = d;
          face = i + 3;
        }
      }
    }
    normal = glm::vec3(0);
    if (face < 3) {
      normal[face] = 1;
    } else {
      normal[face - 3] = -1;
    }
  }

  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;

  const float distance = glm::distance(closestPoint, b.center);
  contact.penetration = normal * (distance - b.radius);

  contact.normal = normal;
  contact.position = closestPoint;

  // 衝突している
  return true;
}

bool CollisionSphereBox(Actor& actorA, Actor& actorB, Contact& contact)
{
  return CollisionBoxSphere(actorB, actorA, contact);
}

/**
* 球と円柱の衝突
*/
bool CollisionSphereCylinder(Actor& actorA, Actor& actorB, Contact& contact)
{
  return false;
}

bool CollisionCylinderSphere(Actor& actorA, Actor& actorB, Contact& contact)
{
  return CollisionSphereCylinder(actorB, actorA, contact);
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
  if (d2 > r * r) {
    return false;
  }

  // どちらか、または両方のアクターが「ブロックしない」場合、接触処理を行わない
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

  // Y方向の浸透距離と方向を計算する
  glm::vec3 normal(0);
  glm::vec3 penetration(0);
  if (dy0 < dy1) {
    penetration.y = -dy0;
    normal.y = 1;
  } else {
    penetration.y = dy1;
    normal.y = -1;
  }

  // XZ方向の浸透距離と方向を計算する
  float lengthXZ;
  if (d2 >= epsilon) {
    const float d = std::sqrt(d2);
    const float invD = 1.0f / d;
    normal.x = -dx * invD;
    normal.z = -dz * invD;
    lengthXZ = r - d;
  } else {
    // XZ座標が重なっている場合、法線を計算できないので速度で代用する
    lengthXZ = r;
    normal.x = actorA.velocity.x - actorB.velocity.x;
    normal.z = actorA.velocity.z - actorB.velocity.z;
    if (normal.x || normal.z) {
      const float invD = 1.0f / std::sqrt(normal.x * normal.x + normal.z * normal.z);
      normal.x *= invD;
      normal.z *= invD;
    } else {
      // ベロシティが0の場合は方向を確定できない。とりあえず+X方向とする
      normal.x = 1;
    }
  }
  penetration.x = -lengthXZ * normal.x;
  penetration.z = -lengthXZ * normal.z;

  // 浸透距離の長い方向を除外する
  if (std::abs(penetration.y) <= lengthXZ) {
    penetration.x = penetration.z = 0;
    normal.x = normal.z = 0;
  } else {
    penetration.y = 0;
    normal.y = 0;
  }

  // 衝突情報を設定
  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;
  contact.penetration = penetration;
  contact.normal = normal;
  contact.penLength = glm::length(penetration);

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
    contact.position = targetCollider->bottom;
    if (normal.y) {
      // Y方向の衝突の場合・・・
      if (targetNormal.y >= 0) {
        contact.position.y += targetCollider->height;
      }
    } else {
      // XZ方向の衝突の場合・・・
      contact.position.x -= targetNormal.x * targetCollider->radius;
      contact.position.y += targetCollider->height * 0.5f;
      contact.position.z -= targetNormal.z * targetCollider->radius;
    }
  }

  // 衝突している
  return true;
}

/**
* 直方体と円柱の衝突
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

  // 円柱の中心に最も近い直方体内のXZ座標(最近接点)を求める
  const float cx = glm::clamp(b.bottom.x, a.min.x, a.max.x);
  const float cz = glm::clamp(b.bottom.z, a.min.z, a.max.z);
  const glm::vec3 closestPointXZ(cx, 0, cz);

  // 円柱の中心から最近接点までの距離を計算する
  const float dx = closestPointXZ.x - b.bottom.x;
  const float dz = closestPointXZ.z - b.bottom.z;
  const float d2 = dx * dx + dz * dz;

  // 最近接点までの距離が円柱の半径より長ければ、衝突していない
  if (d2 > b.radius * b.radius) {
    return false;
  }

  // どちらか、または両方のアクターが「ブロックしない」場合、接触処理を行わない
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

  // Y方向の浸透距離と方向を計算する
  glm::vec3 penetration(0);
  glm::vec3 normal(0);
  if (dy0 < dy1) {
    penetration.y = -dy0;
    normal.y = 1;
  } else {
    penetration.y = dy1;
    normal.y = -1;
  }

  // XZ方向の最近接点の位置によって、衝突面の計算を分ける
  int flagMin = 0;
  int flagMax = 0;
  for (int i = 0; i < 3; i += 2) {
    if (closestPointXZ[i] <= a.min[i]) {
      flagMin |= (1 << i);
    }
    if (closestPointXZ[i] >= a.max[i]) {
      flagMax |= (1 << i);
    }
  }
  const int flag = flagMin | flagMax;

  if (flag) {
    // XZ最近接点が直方体の表面にある場合、XZ最近接点と円を結ぶ直線を法線とする
    if (d2 >= epsilon) {
      normal.x = dx;
      normal.z = dz;
    } else {
      // 最近接点と円柱の中心が近すぎると法線を計算できない
      // かわりに最近接点が含まれる面から求める
      for (int i = 0; i < 3; i += 2) {
        if (flagMin & (1 << i)) {
          normal[i] = 1;
        } else if (flagMax & (1 << i)) {
          normal[i] = -1;
        }
      }
    }
    // XZ方向の法線を正規化
    const float invD = 1.0f / std::sqrt(normal.x * normal.x + normal.z * normal.z);
    normal.x *= invD;
    normal.z *= invD;
  } else {
    // XZ最近接点が直方体の内側にある場合、
    // ベロシティから衝突の可能性があると判断される面のうち、最も近い面で衝突したとみなす
    // ベロシティが0の場合、すべての面に衝突の可能性があると判断する
    const glm::vec3 rv = actorA.velocity - actorB.velocity; // 相対ベロシティを計算
    const bool noVelocity = glm::dot(rv, rv) < epsilon;
    float dmin = FLT_MAX;
    int nearestFace = 0; // 最も近い面
    for (int i = 0; i < 3; i += 2) {
      if (rv[i] < 0 || noVelocity) {
        float d = closestPointXZ[i] - a.min[i];
        if (d < dmin) {
          dmin = d;
          nearestFace = i;
        }
      }
      if (rv[i] > 0 || noVelocity) {
        float d = a.max[i] - closestPointXZ[i];
        if (d < dmin) {
          dmin = d;
          nearestFace = i + 3;
        }
      }
    }
    // 最も近い面の法線を設定する
    if (nearestFace < 3) {
      normal[nearestFace] = 1;
    } else {
      normal[nearestFace - 3] = -1;
    }
  }

  // XZ方向の浸透距離を計算
  float distance = b.radius;
  if (d2 >= epsilon) {
    distance -= std::sqrt(d2);
  }
  penetration.x = -normal.x * distance;
  penetration.z = -normal.z * distance;

  // 浸透距離が長い方向を除外する
  // 側面の衝突がある場合、XZベクトルの長さとYを比較する
  const glm::vec3 absPenetration = glm::abs(penetration);
  if (flag && absPenetration.y > distance) {
    penetration.y = 0;
    normal.y = 0;
  } else {
    // 側面衝突以外の場合、最も浸透距離が短い軸だけを残し、他は除外する
    float pmin = FLT_MAX;
    int axisMin = 0;
    for (int i = 0; i < 3; ++i) {
      if (absPenetration[i] > 0 && absPenetration[i] < pmin) {
        pmin = absPenetration[i];
        axisMin = i;
      }
    }
    for (int i = 0; i < 3; ++i) {
      if (i != axisMin) {
        penetration[i] = 0;
        normal[i] = 0;
      }
    }
  }

  // 衝突情報を設定
  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;
  contact.penetration = penetration;
  contact.normal = normal;
  contact.penLength = glm::length(penetration);

  // 衝突面の座標を計算する
  {
    // 基本的にアクターBの座標を使うが、アクターBが静物の場合はアクターAの座標を使う
    const glm::vec3 center = (a.min + a.max) * 0.5f;
    const glm::vec3 size = (a.max - a.min) * 0.5f;
    if (actorB.isStatic) {
      contact.position = center;
      if (normal.y) {
        // Y方向の衝突の場合・・・
        contact.position.y -= size.y * normal.y;
      } else {
        // XZ方向の衝突の場合・・・
        contact.position.x -= size.x * normal.x;
        contact.position.z -= size.z * normal.z;
      }
    } else {
      contact.position = b.bottom;
      if (normal.y) {
        // Y方向の衝突の場合・・・
        contact.position.y += b.height * (0.5f + 0.5f * normal.y);
      } else {
        // XZ方向の衝突の場合・・・
        contact.position.x -= normal.x * b.radius;
        contact.position.z -= normal.z * b.radius;
      }
    }
  }

  // 衝突している
  return true;
}

/**
* 円柱と直方体の衝突
*/
bool CollisionCylinderBox(Actor& actorA, Actor& actorB, Contact& contact)
{
  return CollisionBoxCylinder(actorB, actorA, contact);
}

