/**
* @file ZombieActor.h
*/
#ifndef ZOMBIEACTOR_H_INCLUDED
#define ZOMBIEACTOR_H_INCLUDED
#include "../Actor.h"

class MainGameScene;

/**
* ゾンビアクター.
*/
class ZombieActor : public Actor
{
public:
  ZombieActor(const glm::vec3& pos, float rotY, MainGameScene* p);
  virtual ~ZombieActor() = default;

  virtual void OnUpdate(float) override;

private:
  void AddBloodActor();

  float moveSpeed = 2;
  MainGameScene* pMainGameScene = nullptr;
};

#endif // ZOMBIEACTOR_H_INCLUDED
