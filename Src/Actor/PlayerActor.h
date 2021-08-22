/**
* @file PlayerActor.h
*/
#ifndef PLAYERACTOR_H_INCLUDED
#define PLAYERACTOR_H_INCLUDED
#include "../Actor.h"

/**
* プレイヤーが操作する戦車
*/
class PlayerActor : public Actor
{
public:
  PlayerActor(
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation);

  virtual ~PlayerActor() = default;
  virtual void OnUpdate(float deltaTime);
  virtual void OnCollision(const struct Contact& contact);

private:
  int oldShotButton = 0;               // 前回のショットボタンの状態
};

#endif // PLAYERACTOR_H_INCLUDED
