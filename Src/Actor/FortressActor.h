/**
* @file FortressActor.h
*/
#ifndef FORTRESSACTOR_H_INCLUDED
#define FORTRESSACTOR_H_INCLUDED
#include "../Actor.h"

/**
* 破壊可能なアクター
*/
class FortressActor : public Actor
{
public:
  FortressActor(
    const std::string& name,
    const Primitive& prim,
    std::shared_ptr<Texture> tex,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment);

  FortressActor(
    const std::string& name,
    const MeshPtr& mesh,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment);

  virtual ~FortressActor() = default;
  virtual std::shared_ptr<Actor> Clone() const override {
    return std::make_shared<FortressActor>(*this);
  }
  virtual void OnCollision(const Contact& c) override;
};


#endif // FORTRESSACTOR_H_INCLUDED
