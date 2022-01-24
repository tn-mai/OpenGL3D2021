/**
* @file PlayerActor.h
*/
#ifndef PLAYERACTOR_H_INCLUDED
#define PLAYERACTOR_H_INCLUDED
#include "../Actor.h"

/**
* �v���C���[�����삷����
*/
class PlayerActor : public Actor
{
public:
  PlayerActor(
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation);

  virtual ~PlayerActor() = default;
  virtual std::shared_ptr<Actor> Clone() const override {
    return std::shared_ptr<Actor>(new PlayerActor(*this));
  }
  virtual void OnUpdate(float deltaTime);
  virtual void OnCollision(const struct Contact& contact);

  // ���[�U�[����\�t���O�̐ݒ�
  void SetControlFlag(bool flag) { isControlable = flag; }
  bool GetControlFlag() const { return isControlable; }

private:
  int oldShotButton = 0;     // �O��̃V���b�g�{�^���̏��
  bool isControlable = true; // ���[�U�[����\�t���O

  // 21�Ŏ���. 21b�͖�����.
  float rotTurret = 0;              // �C���̉�]�p�x
  static const int gunGroup    = 0; // �C�g�̃O���[�v�ԍ�
  static const int turretGroup = 1; // �C���̃O���[�v�ԍ�
};

#endif // PLAYERACTOR_H_INCLUDED
