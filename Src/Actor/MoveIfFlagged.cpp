/**
* @file MoveIfFlagged.cpp
*/
#include "MoveIfFlagged.h"
#include "../GameManager.h"

/**
* コンストラクタ
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
* コンストラクタ
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
* 自分のクローンを作成する
*/
std::shared_ptr<Actor> MoveIfFlagged::Clone() const
{
  return std::make_shared<MoveIfFlagged>(*this);
}

/**
* アクターの状態を更新する
*/
void MoveIfFlagged::OnUpdate(float deltaTime)
{
  GameManager& manager = GameManager::Get();
  const bool flag = manager.GetGameFlag(flagNo);
  position = pos[flag];
}

