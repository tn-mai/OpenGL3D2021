/**
* @file T34TankActor.h
*/
#ifndef T34TANKACTOR_H_INCLUDED
#define T34TANKACTOR_H_INCLUDED
#include "../Actor.h"

/**
* T-34戦車
*/
class T34TankActor : public Actor
{
public:
  T34TankActor(
    const std::string& name,
    const Primitive& prim,
    std::shared_ptr<Texture> tex,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment,
    const std::shared_ptr<Actor>& target);

  virtual ~T34TankActor() = default;
  virtual void OnUpdate(float deltaTime) override;
  virtual void OnCollision(const struct Contact& contact) override;

private:
  std::shared_ptr<Actor> target; // 追いかける対象のアクター
  float shotTimer = 3;
};

#endif // T34TANKACTOR_H_INCLUDED
