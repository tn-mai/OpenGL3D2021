/**
* @file Boss01.cpp
*/
#include "Boss01.h"
#include "BulletActor.h"
#include "../GameEngine.h"
#include "../GameManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>

/**
* �R���X�g���N�^
*/
Boss01::Boss01(const glm::vec3& position, const glm::vec3& scale,
  float rotation, const std::shared_ptr<Actor>& target) :
  Actor("Boss01",
    GameEngine::Get().GetPrimitive("Res/Black_Track.obj"),
    GameEngine::Get().LoadTexture("Res/Black_Track.tga"),
    position, scale, rotation, glm::vec3(0)),
  target(target)
{
  health = 50;
  mass = 200'000;
  collider = Box::Create(glm::vec3(-3, 0, -3), glm::vec3(3, 2.5f, 3));
}

/**
* �A�N�^�[�̏�Ԃ��X�V����
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b)
*/
void Boss01::OnUpdate(float deltaTime)
{
  (this->*mode)(deltaTime);
}

/**
* �Փ˂���������
*
* @param contact �Փˏ��
*/
void Boss01::OnCollision(const struct Contact& contact)
{
  if (contact.b->name == "Bullet") {
    health -= 1;
    if (health <= 0) {
      isDead = true; // T-34��Ԃ���������
      GameManager::Get().AddScore(2000);
    }
    contact.b->isDead = true; // �e����������
  }
}

/**
* �ҋ@���[�h�̍X�V
*/
void Boss01::Idle(float deltaTime)
{
  // �U���Ώۂ����Ȃ���Αҋ@���[�h���p��
  if (!target) {
    return;
  }

  // �U���Ώۂ���苗���ɋ߂Â�����e�����[�h�ɐ؂�ւ���
  const float maxViewLength = 40.0f; // �U���Ώۂ𔭌��ł��鋗��(m)
  const glm::vec3 d = target->position - position;
  if (glm::dot(d, d) < maxViewLength * maxViewLength) {
    // �e�����[�h�̏���
    modeTimer = 10.0f;
    shotTimer = 0.2f;
    shotDirection = 0;
    mode = &Boss01::Danmaku;
  }
}

/**
* �ʏ�e�𔭎˂���
*
* @param engine �Q�[���G���W��
* @param position �e�̔��ˈʒu
* @param velocity �e�̑��x
* @param lifespan �e�̎���(�b)
*/
static void NormalShot(GameEngine& engine,
  const glm::vec3& position,
  const glm::vec3& v,
  float lifespan = 1.5f)
{
  std::shared_ptr<Actor> bullet(new BulletActor{ "EnemyBullet",
    engine.GetPrimitive("Res/Bullet.obj"),
    engine.LoadTexture("Res/Bullet.tga"),
    position, glm::vec3(0.25f), 0, glm::vec3(0) });
  bullet->lifespan = lifespan;
  bullet->velocity = v;
  bullet->mass = 6;
  bullet->friction = 0.0f;
  bullet->collider = Box::Create(glm::vec3(-0.1f), glm::vec3(0.1f));

  engine.AddActor(bullet);
}

/**
* �ʏ�e�𔭎˂���
*
* @param engine �Q�[���G���W��
* @param position �e�̔��ˈʒu
* @param direction �e�̔��˕���(�x)
*/
static void NormalShot(GameEngine& engine,
  const glm::vec3& position,
  float direction)
{
  // ���˕����̃x�N�g�����v�Z
  const float radians = glm::radians(direction);
  const glm::mat4 matR = glm::rotate(glm::mat4(1), radians, glm::vec3(0, 1, 0));
  const glm::vec3 v = matR * glm::vec4(0, 0, 1, 1);

  const float speed = 20.0f;
  NormalShot(engine, position + v, v * speed);
}

/**
* �e�����[�h�̍X�V
*/
void Boss01::Danmaku(float deltaTime)
{
  GameEngine& engine = GameEngine::Get();

  // ��莞�Ԃ��Ƃɒe�𔭎�
  shotTimer -= deltaTime;
  if (shotTimer <= 0) {
    for (float i = 0; i < 360; i += 60) {
      NormalShot(engine, position + glm::vec3(0, 2.8f, 0), shotDirection + i);
      NormalShot(engine, position + glm::vec3(0, 3.0f, 0), 360 - shotDirection + i + 30);
    }
    shotTimer = 0.2f;
    shotDirection = std::fmod(shotDirection + 5.0f, 360.0f);
  }

  // ��莞�Ԃ��o�߂�����}�V���K�����[�h�ɐ؂�ւ���
  modeTimer -= deltaTime;
  if (modeTimer <= 0) {
    // �}�V���K�����[�h�̏���
    // �����Ȃ�؂�ւ��Ȃ��悤�ɁA�ŏ��̔��˃^�C�}�[�͏����Ԋu���J����
    shotTimer = 3.0f;
    ammo = 25; // �}�V���K�����[�h�Ŕ��˂���e�̑���
    mode = &Boss01::Machinegun;
  }
}

/**
* �}�V���K�����[�h�̍X�V
*/
void Boss01::Machinegun(float deltaTime)
{
  // �e���c���Ă���Ȃ�A��莞�Ԃ��Ƃɒe�𔭎�
  if (target && ammo > 0) {
    shotTimer -= deltaTime;
    if (shotTimer <= 0) {
      const glm::vec3 v = glm::normalize(target->position - position);
      const float speed = 20.0f;
      NormalShot(GameEngine::Get(), position + glm::vec3(0, 3, 0), v * speed);

      --ammo;

      // �c�e��5�Ŋ���؂��Ƃ���3�b�ҋ@�A����ȊO��0.1�b�ҋ@
      if (ammo % 5) {
        shotTimer += 0.1f;
      } else {
        shotTimer += 3.0f;
      }
    }
  }

  // �e���Ȃ��Ȃ�����~�T�C�����[�h�ɐ؂�ւ���
  if (ammo <= 0) {
    // �~�T�C�����[�h�̏���
    ammo = 4; // �~�T�C���̒e��
    shotTimer = 3.0f;
    mode = &Boss01::Missile;
  }
}

/**
* �~�T�C�����[�h�̍X�V
*/
void Boss01::Missile(float deltaTime)
{
  // �~�T�C�����c���Ă���Ȃ�A��莞�Ԃ��ƂɃ~�T�C���𔭎�
  if (target && ammo > 0) {
    shotTimer -= deltaTime;
    if (shotTimer <= 0) {
      const float flightTime = 4.0f;
      const float gravity = 9.8f;
      glm::vec3 v = target->position - position;
      v *= 1.0f / flightTime;
      // ���������グ�̌�������Y�����̑��x���v�Z
      v.y = 0.5f * gravity * flightTime;
      NormalShot(GameEngine::Get(), position + glm::vec3(0, 3, 0), v, flightTime);
      --ammo;
      shotTimer = 4.0f;
    }
  }

  // �~�T�C���̎c��e����0�ɂȂ�����e�����[�h�ɐ؂�ւ���
  if (ammo <= 0) {
    // �e�����[�h�̏���
    modeTimer = 10.0f;
    shotTimer = 0.2f;
    shotDirection = 0;
    mode = &Boss01::Danmaku;
  }
}

