/**
* @file RandomMovingEnemyActor.h
*/
#ifndef RANDOMMOVINGENEMYACTOR_H_INCLUDED
#define RANDOMMOVINGENEMYACTOR_H_INCLUDED
#include "../Actor.h"
#include "../GameMap.h"
#include <memory>

/**
* ÉâÉìÉ_ÉÄÇ»à⁄ìÆÇÇ∑ÇÈìGêÌé‘
*/
class RandomMovingEnemyActor : public Actor
{
public:
  RandomMovingEnemyActor(
    const std::string& name,
    const Primitive& prim,
    std::shared_ptr<Texture> tex,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment,
    const std::shared_ptr<GameMap>& gamemap);

  virtual ~RandomMovingEnemyActor() = default;
  virtual void OnUpdate(float deltaTime) override;
  virtual void OnCollision(const struct Contact& contact) override;

private:
  std::shared_ptr<GameMap> gamemap;
  glm::vec3 posGoals[2];
  int dirGoals[2] = { 0, 0 };
  bool shouldRotate = false;
  float timer = 0;
  float shotInterval = 5.0f;
};

#endif // RANDOMMOVINGENEMYACTOR_H_INCLUDED
