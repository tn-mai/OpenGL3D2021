/**
* @file RandomMovingEnemyActor.cpp
*/
#include "RandomMovingEnemyActor.h"
#include "BulletActor.h"
#include "../GameEngine.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

/**
* �R���X�g���N�^
*/
RandomMovingEnemyActor::RandomMovingEnemyActor(
  const std::string& name,
  const Primitive& prim,
  std::shared_ptr<Texture> tex,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment,
  const std::shared_ptr<GameMap>& gamemap) :
  Actor(name, prim, tex, position, scale, rotation, adjustment), // ���N���X��������
  gamemap(gamemap),
  posGoals{ position, position },
  shotInterval(static_cast<float>(GameEngine::Get().GetRandom() % 9 + 1))
{
}

/**
* �A�N�^�[�̏�Ԃ��X�V����
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b)
*/
void RandomMovingEnemyActor::OnUpdate(float deltaTime)
{
  /*
    �u���݂̈ړ���v�Ɓu���̈ړ���v��ێ�.
    �u���̈ړ���v���u���݂̈ړ���v�̉�������Ɂc
      �Ȃ�: �u���݂̈ړ���v�ňꎞ��~.
      ����: ���̈ړ����ݒ�(�ꎞ��~���Ȃ�).
  */
  if (timer > 0) {
    timer -= deltaTime;
  }

  GameEngine& engine = GameEngine::Get();

  // �ړ���0�܂ł̋������v�Z
  const float length = glm::length(posGoals[0] - position);

  // ������2m�����Ȃ�ړ���0�ɓ��������Ƃ݂Ȃ�
  bool shouldSearchNextGoal = false;
  if (length < 2.0f) {
    // �ړ���0�ƈړ���1�̈ړ��������������ꍇ�A�ړ���1���ړ���0�ɃR�s�[���A�V�����ړ���1���쐬����
    if (dirGoals[0] == dirGoals[1]) {
      shouldSearchNextGoal = true;
    } else {
      // ���x��1m/s�����ɂȂ������~�����Ƃ݂Ȃ��A���̈ړ����ݒ肷��
      if (glm::length(velocity) < 1) {
        shouldSearchNextGoal = true;
      }
    }
  }

  if (shouldSearchNextGoal || shouldRotate) {
    shouldSearchNextGoal = false;
    posGoals[0] = posGoals[1];
    dirGoals[0] = dirGoals[1];

    // �}�b�v���Q�Ƃ��Ĉړ��\�ȕ����𒲂ׂ�
    glm::vec3 pass[4] = {};
    bool hasPass[4] = {};
    const glm::vec3 dirList[] = { { 0, 0, 4}, { 4, 0, 0}, { 0, 0,-4}, {-4, 0, 0} };
    glm::vec3 endGoal = shouldRotate ? position : posGoals[1];
    endGoal.x = std::floor((endGoal.x + 2.0f) / 4.0f) * 4.0f;
    endGoal.z = std::floor((endGoal.z + 2.0f) / 4.0f) * 4.0f;
    for (int i = 0; i < 4; ++i) {
      if (shouldRotate && i == dirGoals[0]) {
        continue;
      }
      const float x = endGoal.x + dirList[i].x;
      const float z = endGoal.z + dirList[i].z;
      const int objectNo = gamemap->GetObjectNo(x, z);
      if (objectNo == 0) {
        pass[i] = glm::vec3(x, endGoal.y, z);
        hasPass[i] = true;
      }
    }

    unsigned int maxScore = 0;
    int nextDir = -1;
    const unsigned int basicScore[] = { 50, 1, 50, 90, 50, 1, 50 };
    for (int i = 0; i < 4; ++i) {
      if (hasPass[i]) {
        unsigned int score = engine.GetRandom() % 100 + basicScore[i + 3 - dirGoals[1]];
        if (score > maxScore) {
          maxScore = score;
          nextDir = i;
        }
      }
    }

    if (nextDir >= 0) {
      posGoals[1] = pass[nextDir];
      dirGoals[1] = nextDir;
    }

    if (shouldRotate) {
      posGoals[0] = posGoals[1];
      dirGoals[0] = dirGoals[1];
      shouldRotate = false;
      timer = 2.0f;
    }
  }

  // �ړI�n�����ʂɗ���悤�ɐ��񂷂�
  const glm::mat4 matRot = glm::rotate(glm::mat4(1), rotation, glm::vec3(0, 1, 0));
  const glm::vec3 front = matRot * glm::vec4(0, 0, 1, 1);
  const glm::vec3 dirGoal = glm::normalize(posGoals[0] - position);
  const float r = std::acos(glm::dot(front, dirGoal));
  const float speed = glm::min(glm::radians(90.0f) * deltaTime, r);
  if (length >= 2.0f) {
    const glm::vec3 n = glm::cross(front, dirGoal);
    if (n.y >= 0) {
      rotation += speed;
    } else {
      rotation -= speed;
    }
  }
  
  if (r <= speed * 4) {
    // �ړ���܂ł̋����ɉ����ĉ���/��������
    glm::vec3 dist = posGoals[0] - position;
    dist.y = 0;
    glm::vec3 targetVelocity = glm::clamp(dist * 4.0f, -8.0f, 8.0f);
    glm::vec3 error = targetVelocity - velocity;
    glm::vec3 force = glm::clamp(error * 4.0f, -20.0f, 20.0f);
    velocity += force * deltaTime;
  }

  shotInterval -= deltaTime;
  if (shotInterval <= 0) {
    shotInterval = static_cast<float>(engine.GetRandom() % 7 + 1);

    glm::vec3 position = this->position + front * 3.0f;
    position.y += 2.0f;

    std::shared_ptr<Actor> bullet(new BulletActor{
      "EnemyBullet", engine.GetPrimitive(9), engine.LoadTexture("Res/Bullet.tga"),
      position, glm::vec3(0.25f), rotation, glm::vec3(0) });
    bullet->lifespan = 1.5f;
    bullet->velocity = front * 20.0f;
    bullet->collider = Box::Create(glm::vec3(-0.25f), glm::vec3(0.25f));
    bullet->mass = 6.8f;
    bullet->friction = 1.0f;

    engine.AddActor(bullet);
  }
}

/**
* �Փ˂���������
*
* @param contact �Փˏ��
*/
void RandomMovingEnemyActor::OnCollision(const struct Contact& contact)
{
  if (contact.b->name == "EnemyBullet") {
    return;
  }

  if (contact.b->name == "Bullet") {
    // T-34��Ԃ̑ϋv�l�����炷
    health -= 1;
    if (health <= 0) {
      isDead = true; // T-34��Ԃ���������
    }
    contact.b->isDead = true; // �e����������
  } else {
    if (timer <= 0) {
      if (std::abs(contact.normal.y) < 0.0001f) {
        glm::vec3 v = contact.velocityA - contact.velocityB;
        v.y = 0;
        const float l = glm::length(v);
        if (l > 0.0001f) {
          const float r = std::acos(glm::dot(-v * (1.0f / l), contact.normal));
          if (r < glm::radians(45.0f)) {
            shouldRotate = true;
            timer = 2.0f;
          }
        }
      }
    }
  }
}

