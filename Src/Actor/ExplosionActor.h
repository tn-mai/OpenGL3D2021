/**
* @file ExplosionActor.h
*/
#ifndef EXPLOSIONACTOR_H_INCLUDED
#define EXPLOSIONACTOR_H_INCLUDED
#include "../Actor.h"

/**
* ”š”­
*/
class ExplosionActor : public Actor
{
public:
  ExplosionActor(const glm::vec3& position, float scale);
  virtual ~ExplosionActor() = default;

  virtual void OnUpdate(float) override;

private:
  float scale = 1;
  float timer = 0;
};
#endif // EXPLOSIONACTOR_H_INCLUDED
