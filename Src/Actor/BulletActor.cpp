/**
* @file BulletActor.cpp
*/
#include "BulletActor.h"

/**
* �R���X�g���N�^
*/
BulletActor::BulletActor(
  const std::string& name,
  const Primitive& prim,
  std::shared_ptr<Texture> tex,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment) :
  Actor(name, prim, tex, position, scale, rotation, adjustment) // ���N���X��������
{
}

/**
* �Փ˂���������
*
* @param contact �Փˏ��
*/
void BulletActor::OnCollision(const struct Contact& contact)
{
  if (!contact.b->isStatic) {
    isDead = true;
  }
}

