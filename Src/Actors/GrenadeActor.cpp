/**
* @file GrenadeActor.cpp
*/
#define NOMINMAX
#include "GrenadeActor.h"
#include "../MainGameScene.h"
#include "../GameData.h"
#include "../Audio.h"
#include "../Audio/MainWorkUnit/SE.h"

/**
* コンストラクタ.
*
* @param pos  アクターを配置する座標.
* @param vel  アクターの移動速度.
* @param rotY アクターの向き.
*/
GrenadeActor::GrenadeActor(const glm::vec3& pos, const glm::vec3& vel, float rotY, MainGameScene* pScene) :
  Actor("grenade",
    &GameData::Get().primitiveBuffer.Get(GameData::PrimNo::m67_grenade),
    std::make_shared<Texture::Image2D>("Res/m67_grenade.tga"),
    pos),
  pMainGameScene(pScene)
{
  rotation.y = rotY;
  velocity = vel;
  gravityScale = 1;
  lifespan = 2; // 点火時間.
  //friction = 0.2f;

  // 衝突形状を設定.
  const float scaleFactor = 1;
  scale = glm::vec3(scaleFactor);
  SetCylinderCollision(0.1f * scaleFactor, -0.1f * scaleFactor, 0.1f * scaleFactor);
}

/**
* 手榴弾の状態を更新する.
*
* @param deltaTime 前回の更新からの経過時間(秒).
*/
void GrenadeActor::OnUpdate(float deltaTime)
{
#if 0
  // 衝突回数に応じて減速.
  if (hitCount > 0) {
    // 衝突した回数だけブレーキがかかる.
    const float brakeSpeed = static_cast<float>(hitCount) * 4 * deltaTime;
    // ブレーキ速度より速ければ減速、ブレーキ速度以下なら速度を0にする.
    if (glm::dot(velocity, velocity) > brakeSpeed * brakeSpeed) {
      // 逆ベクトルを計算.
      const glm::vec3 vecInverse = -glm::normalize(velocity);
      // ブレーキ速度に経過時間を掛けた値を減速.
      velocity += vecInverse * brakeSpeed;
    } else {
      velocity = glm::vec3(0);
    }
    // 衝突回数を0に戻す.
    hitCount = 0;
  }
#endif

  // 速度に応じて回転させる.
  if (glm::dot(velocity, velocity) > 0) {
    const float speed = glm::length(velocity);
    rotation.z -= glm::radians(400.0f) * speed * deltaTime;
  }
}

/**
* 手榴弾が破壊されたときの処理.
*/
void GrenadeActor::OnDestroy()
{
  // 攻撃範囲アクターを追加.
  ActorPtr actor = std::make_shared<Actor>("explosion", nullptr, nullptr, position);
  actor->SetCylinderCollision(3, -1, 3);
  actor->collision.blockOtherActors = false;
  actor->lifespan = 0.00001f;
  actor->OnHit = [](Actor& a, Actor& b) {
    if (b.name == "zombie" || b.name == "grenade") {
      // 吹き飛ばされるアクターの中心座標を計算.
      glm::vec3 p = b.position;
      if (b.name == "zombie") {
        p.y += 1;
      }
      // 吹っ飛ぶ方向を計算.
      const glm::vec3 v = p - a.position;
      // 完全に重なっている場合は上に吹き飛ばす.
      if (glm::dot(v, v) <= 0) {
        b.velocity += glm::vec3(0, 5, 0);
        return;
      }

      // 爆風の最大到達距離を計算.
      const float longY = std::max(a.collision.top, -a.collision.bottom);
      const float maxRange = std::sqrt(
        a.collision.radius * a.collision.radius + longY * longY);
      // 爆発中心に近いほど強く吹き飛ばす.
      const float ratio = 1 - glm::length(v) / maxRange;
      // 吹き飛ばし力はとりあえず最大6m/s、最低2m/sとする.
      const float speed = ratio * 4 + 2;
      b.velocity += glm::normalize(v) * speed;
    }
  };
  pMainGameScene->AddActor(actor);

  Audio::Instance().Play(2, CRI_SE_BANG_2);
}
