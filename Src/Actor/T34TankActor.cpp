/**
* @file T34TankActor.cpp
*/
#include "T34TankActor.h"
#include "BulletActor.h"
#include "../GameEngine.h"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

/**
* コンストラクタ
*/
T34TankActor::T34TankActor(
  const std::string& name,
  const Primitive& prim,
  std::shared_ptr<Texture> tex,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment,
  const std::shared_ptr<Actor>& target) :
  Actor(name, prim, tex, position, scale, rotation, adjustment), // 基底クラスを初期化
  target(target)
{
}

/**
* アクターの状態を更新する
*
* @param deltaTime 前回の更新からの経過時間(秒)
*/
void T34TankActor::OnUpdate(float deltaTime)
{
  // 追跡対象アクターが設定されている場合の処理
  if (isOnActor && target) {
    // T-34戦車の正面方向のベクトルを計算
    glm::mat4 matR = glm::rotate(glm::mat4(1), rotation, glm::vec3(0, 1, 0));
    glm::vec3 t34Front = matR * glm::vec4(0, 0, 1, 1);

    // T-34戦車からタイガーI戦車へのベクトルdを計算
    glm::vec3 d = target->position - position;

    // T-34戦車からタイガーI戦車への距離を計算
    float length = glm::length(d);

    // ベクトルdを正規化
    d = glm::normalize(d);

    // T-34戦車の正面ベクトルと、タイガーI戦車へのベクトルの内積を計算
    float r = std::acos(glm::dot(t34Front, d));

    // T-34戦車の正面とタイガーI戦車のいる方向の角度が10度未満の場合...
    if (r < glm::radians(10.0f)) {
      // タイガーI戦車までの距離が10mより遠い場合は前に加速
      if (length > 10.0f) {
        velocity += t34Front * 0.3f;
      } else {
        // ベロシティのt34Front方向の長さを計算
        float v = glm::dot(t34Front, velocity);
        // 長さが0.2以上なら0.2を減速、それ以下なら長さ分を減速する
        velocity -= t34Front * glm::clamp(v, -0.2f, 0.2f);
      }
    }
    // 角度が10度以上の場合...
    else {
      // T-34戦車の正面ベクトルと、タイガーI戦車へのベクトルの外積を計算
      glm::vec3 n = glm::cross(t34Front, d);
      // yが0以上なら反時計回り、0未満なら時計回りに回転するほうが近い
      if (n.y >= 0) {
        rotation += glm::radians(90.0f) * deltaTime;
      } else {
        rotation -= glm::radians(90.0f) * deltaTime;
      }
    }

    // 弾を発射
    shotTimer -= deltaTime;
    if (shotTimer <= 0) {
      shotTimer = 3;

      // 発射位置を砲の先端に設定
      glm::vec3 position = this->position + t34Front * 2.0f;
      position.y += 2.0f;

      GameEngine& engine = GameEngine::Get();
      std::shared_ptr<Actor> bullet(new BulletActor{
        "EnemyBullet", engine.GetPrimitive(9), engine.GetTexture("Res/Bullet.tga"),
        position, glm::vec3(0.25f), rotation, glm::vec3(0) });

      // 1.5秒後に弾を消す
      bullet->lifespan = 1.5f;

      // 戦車の向いている方向に、30m/sの速度で移動させる
      bullet->velocity = t34Front * 20.0f;

      // 弾に衝突判定を付ける
      bullet->collider = Box{ glm::vec3(-0.25f), glm::vec3(0.25f) };
      bullet->mass = 6.8f;
      bullet->friction = 1.0f;

      engine.AddActor(bullet);
    }
  }
}

/**
* 衝突を処理する
*
* @param contact 衝突情報
*/
void T34TankActor::OnCollision(const struct Contact& contact)
{
  if (contact.b->name == "Bullet") {
    // T-34戦車の耐久値を減らす
    health -= 1;
    if (health <= 0) {
      isDead = true; // T-34戦車を消去する
    }
    contact.b->isDead = true; // 弾を消去する
  }
}

