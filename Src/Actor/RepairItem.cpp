/**
* @file RepairItem.cpp
*/
#include "RepairItem.h"
#include "../GameEngine.h"

/**
* �R���X�g���N�^
*/
RepairItem::RepairItem(const glm::vec3& position) :
  Actor("Repair",
    GameEngine::Get().GetPrimitive("Res/Cube.obj"),
    GameEngine::Get().LoadTexture("Res/RepairItem.tga"),
    position, glm::vec3(1), 0, glm::vec3(-1, 0, -1))
{
  isStatic = true;
  collisionType = CollisionType::trigger;
  collider = Box::Create(glm::vec3(-1, 0, -1), glm::vec3(1, 2, 1));
}

/**
* �A�N�^�[�̏�Ԃ��X�V����
*/
void RepairItem::OnUpdate(float deltaTime)
{
  rotation += deltaTime;
  if (rotation >= glm::radians(360.0f)) {
    rotation -= glm::radians(360.0f);
  }
}

/**
* �g���K�[�n���h��
*/
void RepairItem::OnTrigger(std::shared_ptr<Actor> other)
{
  if (other->name == "Tiger-I") {
    other->health += 5;
    if (other->health >= 10) {
      other->health = 10;
    }
    isDead = true;
  }
}

