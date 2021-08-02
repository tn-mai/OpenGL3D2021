/**
* @file BulletActor.cpp
*/
#include "BulletActor.h"

/**
* コンストラクタ
*/
BulletActor::BulletActor(
  const std::string& name,
  const Primitive& prim,
  std::shared_ptr<Texture> tex,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment) :
  Actor(name, prim, tex, position, scale, rotation, adjustment) // 基底クラスを初期化
{
}

/**
* 衝突を処理する
*
* @param contact 衝突情報
*/
void BulletActor::OnCollision(const struct Contact& contact)
{
  if (!contact.b->isStatic) {
    isDead = true;
  }
}

