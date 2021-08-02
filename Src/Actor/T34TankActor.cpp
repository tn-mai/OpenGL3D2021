/**
* @file T34TankActor.cpp
*/
#include "T34TankActor.h"
#include "BulletActor.h"
#include "../GameEngine.h"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

/**
* �R���X�g���N�^
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
  Actor(name, prim, tex, position, scale, rotation, adjustment), // ���N���X��������
  target(target)
{
}

/**
* �A�N�^�[�̏�Ԃ��X�V����
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b)
*/
void T34TankActor::OnUpdate(float deltaTime)
{
  // �ǐՑΏۃA�N�^�[���ݒ肳��Ă���ꍇ�̏���
  if (isOnActor && target) {
    // T-34��Ԃ̐��ʕ����̃x�N�g�����v�Z
    glm::mat4 matR = glm::rotate(glm::mat4(1), rotation, glm::vec3(0, 1, 0));
    glm::vec3 t34Front = matR * glm::vec4(0, 0, 1, 1);

    // T-34��Ԃ���^�C�K�[I��Ԃւ̃x�N�g��d���v�Z
    glm::vec3 d = target->position - position;

    // T-34��Ԃ���^�C�K�[I��Ԃւ̋������v�Z
    float length = glm::length(d);

    // �x�N�g��d�𐳋K��
    d = glm::normalize(d);

    // T-34��Ԃ̐��ʃx�N�g���ƁA�^�C�K�[I��Ԃւ̃x�N�g���̓��ς��v�Z
    float r = std::acos(glm::dot(t34Front, d));

    // T-34��Ԃ̐��ʂƃ^�C�K�[I��Ԃ̂�������̊p�x��10�x�����̏ꍇ...
    if (r < glm::radians(10.0f)) {
      // �^�C�K�[I��Ԃ܂ł̋�����10m��艓���ꍇ�͑O�ɉ���
      if (length > 10.0f) {
        velocity += t34Front * 0.3f;
      } else {
        // �x���V�e�B��t34Front�����̒������v�Z
        float v = glm::dot(t34Front, velocity);
        // ������0.2�ȏ�Ȃ�0.2�������A����ȉ��Ȃ璷��������������
        velocity -= t34Front * glm::clamp(v, -0.2f, 0.2f);
      }
    }
    // �p�x��10�x�ȏ�̏ꍇ...
    else {
      // T-34��Ԃ̐��ʃx�N�g���ƁA�^�C�K�[I��Ԃւ̃x�N�g���̊O�ς��v�Z
      glm::vec3 n = glm::cross(t34Front, d);
      // y��0�ȏ�Ȃ甽���v���A0�����Ȃ玞�v���ɉ�]����ق����߂�
      if (n.y >= 0) {
        rotation += glm::radians(90.0f) * deltaTime;
      } else {
        rotation -= glm::radians(90.0f) * deltaTime;
      }
    }

    // �e�𔭎�
    shotTimer -= deltaTime;
    if (shotTimer <= 0) {
      shotTimer = 3;

      // ���ˈʒu��C�̐�[�ɐݒ�
      glm::vec3 position = this->position + t34Front * 2.0f;
      position.y += 2.0f;

      GameEngine& engine = GameEngine::Get();
      std::shared_ptr<Actor> bullet(new BulletActor{
        "EnemyBullet", engine.GetPrimitive(9), engine.GetTexture("Res/Bullet.tga"),
        position, glm::vec3(0.25f), rotation, glm::vec3(0) });

      // 1.5�b��ɒe������
      bullet->lifespan = 1.5f;

      // ��Ԃ̌����Ă�������ɁA30m/s�̑��x�ňړ�������
      bullet->velocity = t34Front * 20.0f;

      // �e�ɏՓ˔����t����
      bullet->collider = Box{ glm::vec3(-0.25f), glm::vec3(0.25f) };
      bullet->mass = 6.8f;
      bullet->friction = 1.0f;

      engine.AddActor(bullet);
    }
  }
}

/**
* �Փ˂���������
*
* @param contact �Փˏ��
*/
void T34TankActor::OnCollision(const struct Contact& contact)
{
  if (contact.b->name == "Bullet") {
    // T-34��Ԃ̑ϋv�l�����炷
    health -= 1;
    if (health <= 0) {
      isDead = true; // T-34��Ԃ���������
    }
    contact.b->isDead = true; // �e����������
  }
}

