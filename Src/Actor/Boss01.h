/**
* @file Boss01.h
*/
#ifndef BOSS01_H_INCLUDED
#define BOSS01_H_INCLUDED
#include "../Actor.h"

/**
* �X�e�[�W1�̃{�X�G
*/
class Boss01 : public Actor
{
public:
  Boss01(const glm::vec3& position, const glm::vec3& scale,
    float rotation, const std::shared_ptr<Actor>& target);
  virtual ~Boss01() = default;
  virtual std::shared_ptr<Actor> Clone() const override {
    return std::shared_ptr<Actor>(new Boss01(*this));
  }

  virtual void OnUpdate(float deltaTime) override;
  virtual void OnCollision(const struct Contact& contact) override;

  void SetTarget(std::shared_ptr<Actor> t) { target = t; }

private:
  // ���샂�[�h
  void Idle(float deltaTime);
  void Danmaku(float deltaTime);
  void Machinegun(float deltaTime);
  void Missile(float deltaTime);

  using ModeFunc = void(Boss01::*)(float); // �����o�֐��|�C���^�^
  ModeFunc mode = &Boss01::Idle; // ���݂̓��샂�[�h

  std::shared_ptr<Actor> target; // �ǂ�������Ώۂ̃A�N�^�[
  float modeTimer = 0;     // ���݂̓��샂�[�h���I������܂ł̕b��
  float shotTimer = 0;     // ���̒e�𔭎˂���܂ł̕b��
  float shotDirection = 0; // �e���̔��˕���
  int ammo = 0;            // �}�V���K���̘A�����ː�
};

#endif // BOSS01_H_INCLUDED
