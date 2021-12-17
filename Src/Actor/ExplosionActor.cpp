/**
* @file ExplosionActor.cpp
*/
#include "ExplosionActor.h"
#include "../GameEngine.h"

/**
* コンストラクタ
*/
ExplosionActor::ExplosionActor(const glm::vec3& position, float scale) :
  Actor("Explosion",
    GameEngine::Get().GetPrimitive("Res/Explosion.obj"),
    GameEngine::Get().LoadTexture("Res/ExplosionActor.tga"),
    position, glm::vec3(0.1f), 0, glm::vec3(0)),
  scale(scale)
{
  isStatic = true;
  collisionType = CollisionType::noCollision;
}

/**
* アクターの状態を更新する
*/
void ExplosionActor::OnUpdate(float deltaTime)
{
  rotation += glm::radians(180.0f) * deltaTime;

  timer += deltaTime;
  const float totalTime = 0.5f; // 爆発表示時間(秒)
  const float t = timer / totalTime; // 0-1に変換
  const float baseScale = scale * 0.1f;
  Actor::scale = glm::vec3(baseScale + scale * 0.9f * t);

  // 時間を過ぎたら自分自身を削除
  if (timer >= totalTime) {
    isDead = true;
  }
}
