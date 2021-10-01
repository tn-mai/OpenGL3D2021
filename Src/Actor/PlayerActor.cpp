/**
* @file PlayerActor.cpp
*/
#include "PlayerActor.h"
#include "../GameEngine.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* �R���X�g���N�^
*/
PlayerActor::PlayerActor(
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation) :
  Actor(
    "Tiger-I",
    GameEngine::Get().GetPrimitive("Res/tank/Tiger_I.obj"),
    GameEngine::Get().LoadTexture("Res/tank/PzVl_Tiger_I.tga"),
    position, scale, rotation, glm::vec3(0))
{
  health = 10;
  //collider = CreateBoxCollider(glm::vec3(-1.8f, 0, -1.8f), glm::vec3(1.8f, 2.8f, 1.8f));
  collider = CreateCylinderShape(glm::vec3(0), 1.8f, 2.8f);
  mass = 57'000;
  //cor = 0.1f;
  //friction = 1.0f;
}

/**
* �A�N�^�[�̏�Ԃ��X�V����
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b)
*/
void PlayerActor::OnUpdate(float deltaTime)
{
  GameEngine& engine = GameEngine::Get();

  if (isOnActor) {
    if (engine.GetKey(GLFW_KEY_A)) {
      rotation += glm::radians(90.0f) * deltaTime;
    } else if (engine.GetKey(GLFW_KEY_D)) {
      rotation -= glm::radians(90.0f) * deltaTime;
    }
  }

  // rotation��0�̂Ƃ��̐�Ԃ̌����x�N�g��
  glm::vec3 tankFront(0, 0, 1);
  // rotation���W�A��������]�������]�s������
  const glm::mat4 matRot = glm::rotate(glm::mat4(1), rotation, glm::vec3(0, 1, 0));
  // �����x�N�g����tank.rotation������]������
  tankFront = matRot * glm::vec4(tankFront, 1);

  if (isOnActor) {
    float speed2 = glm::dot(velocity, velocity);
    //if (speed2 < 10.0f * 10.0f) {
      float tankAccel = 0.2f; // ��Ԃ̉����x
      if (engine.GetKey(GLFW_KEY_W)) {
        velocity += tankFront * tankAccel;
      } else if (engine.GetKey(GLFW_KEY_S)) {
        velocity -= tankFront * tankAccel;
      } else {
        float v = glm::dot(tankFront, velocity);
        velocity -= tankFront * glm::clamp(v, -0.1f, 0.1f);
      }
      glm::vec3 tankRight = glm::normalize(glm::cross(tankFront, glm::vec3(0, 1, 0)));
      float rightSpeed = glm::dot(tankRight, velocity);
      velocity -= tankRight * glm::clamp(rightSpeed, -0.2f, 0.2f);
    //}
  }

  // �}�E�X���{�^���̏�Ԃ��擾����
  int shotButton = engine.GetMouseButton(GLFW_MOUSE_BUTTON_LEFT);

  // �}�E�X���{�^���������ꂽ�u�Ԃɒe�A�N�^�[�𔭎˂���
  static int shotInterval = 5;
  bool isShot = false;
  if (shotButton != 0) {
    if (oldShotButton == 0 || --shotInterval <= 0) {
      isShot = true;
      shotInterval = 5;
    }
  }
  if (isShot) {
    // ���ˈʒu��C�̐�[�ɐݒ�
    glm::vec3 position = this->position + tankFront * 6.0f;
    position.y += 2.0f;

    std::shared_ptr<Actor> bullet(new Actor{
      "Bullet",
      engine.GetPrimitive("Res/Bullet.obj"),
      engine.LoadTexture("Res/Bullet.tga"),
      position, glm::vec3(0.25f), rotation, glm::vec3(0) });

    // 1.5�b��ɒe������
    bullet->lifespan = 1.5f;

    // ��Ԃ̌����Ă�������ɁA30m/s�̑��x�ňړ�������
    bullet->velocity = tankFront * 30.0f;

    // �e�ɏՓ˔����t����
    bullet->collider = CreateBoxShape(glm::vec3(-0.25f), glm::vec3(0.25f));
    bullet->mass = 6.8f;
    bullet->friction = 1.0f;

    engine.AddActor(bullet);
  }

  // �u�O��̃V���b�g�{�^���̏�ԁv���X�V����
  oldShotButton = shotButton;
}

/**
* �Փ˂���������
*
* @param contact �Փˏ��
*/
void PlayerActor::OnCollision(const struct Contact& contact)
{
  if (contact.b->name == "EnemyBullet") {
    --health;
    if (health <= 0) {
      isDead = true;
    }
    contact.b->isDead = true;
  }
}

