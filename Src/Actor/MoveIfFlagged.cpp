/**
* @file MoveIfFlagged.cpp
*/
#include "MoveIfFlagged.h"
#include "../GameManager.h"

/**
* �R���X�g���N�^
*/
MoveIfFlagged::MoveIfFlagged(
  const std::string& name,
  const Primitive& prim,
  std::shared_ptr<Texture> tex,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment) :
  Actor(name, prim, tex, position, scale, rotation, adjustment)
{
  isStatic = true;
}

/**
* �R���X�g���N�^
*/
MoveIfFlagged::MoveIfFlagged(
  const std::string& name,
  const MeshPtr& mesh,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment) :
  Actor(name, mesh, position, scale, rotation, adjustment)
{
  isStatic = true;
}

/**
* �����̃N���[�����쐬����
*/
std::shared_ptr<Actor> MoveIfFlagged::Clone() const
{
  return std::make_shared<MoveIfFlagged>(*this);
}

/**
* �A�N�^�[�̏�Ԃ��X�V����
*/
void MoveIfFlagged::OnUpdate(float deltaTime)
{
  GameManager& manager = GameManager::Get();
  const bool flag = manager.GetGameFlag(flagNo);
  position = pos[flag];
}

