/**
* @file BulletActor.h
*/
#ifndef BULLETACTOR_H_INCLUDED
#define BULLETACTOR_H_INCLUDED
#include "../Actor.h"

/**
* エレベーター
*/
class BulletActor : public Actor
{
public:
  BulletActor(
    const std::string& name,
    const Primitive& prim,
    std::shared_ptr<Texture> tex,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment);

  virtual ~BulletActor() = default;
  virtual std::shared_ptr<Actor> Clone() const override {
    return std::shared_ptr<Actor>(new BulletActor(*this));
  }
  virtual void OnCollision(const struct Contact& contact) override;
};

#endif // BULLETACTOR_H_INCLUDED
