/**
* @file ElevatorActor.cpp
*/
#include "ElevatorActor.h"

/**
* コンストラクタ
*/
ElevatorActor::ElevatorActor(
  const std::string& name,
  const Primitive& prim,
  std::shared_ptr<Texture> tex,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment) :
  Actor(name, prim, tex, position, scale, rotation, adjustment) // 基底クラスを初期化
{
}

/**
* アクターの状態を更新する
*
* @param deltaTime 前回の更新からの経過時間(秒)
*/
void ElevatorActor::OnUpdate(float deltaTime)
{
  // エレベーターの移動方向を切り替える
  switch (elevetorState) {
  case 0:
    if (position.y >= 4) {
      position.y = 4;
      velocity.y = 0;
      velocity.z = 1;
      elevetorState = 1;
    }
    break;
  case 1:
    if (position.z >= 0) {
      position.z = 0;
      velocity.z = -1;
      elevetorState = 2;
    }
    break;
  case 2:
    if (position.z <= -4) {
      position.z = -4;
      velocity.y = -1;
      velocity.z = 0;
      elevetorState = 3;
    }
    break;
  case 3:
    if (position.y <= -1) {
      position.y = -1;
      velocity.y = 1;
      elevetorState = 0;
    }
    break;
  }
}
