/**
* @file ExplosionActor.cpp
*/
#include "ExplosionActor.h"
#include "../GameEngine.h"

/**
* �R���X�g���N�^
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
* �A�N�^�[�̏�Ԃ��X�V����
*/
void ExplosionActor::OnUpdate(float deltaTime)
{
  rotation += glm::radians(180.0f) * deltaTime;

  timer += deltaTime;
  const float totalTime = 0.5f; // �����\������(�b)
  const float t = timer / totalTime; // 0-1�ɕϊ�
  const float baseScale = scale * 0.1f;
  Actor::scale = glm::vec3(baseScale + scale * 0.9f * t);

  // ���Ԃ��߂����玩�����g���폜
  if (timer >= totalTime) {
    isDead = true;
  }
}
