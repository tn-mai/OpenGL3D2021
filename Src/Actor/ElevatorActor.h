/**
* @file ElevatorActor.h
*/
#ifndef ELEVATORACTOR_H_INCLUDED
#define ELEVATORACTOR_H_INCLUDED
#include "../Actor.h"

/**
* エレベーター
*/
class ElevatorActor : public Actor
{
public:
  ElevatorActor(
    const std::string& name,
    const Primitive& prim,
    std::shared_ptr<Texture> tex,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment);

  virtual ~ElevatorActor() = default;
  virtual std::shared_ptr<Actor> Clone() const override {
    return std::shared_ptr<Actor>(new ElevatorActor(*this));
  }
  virtual void OnUpdate(float deltaTime) override;

private:
  int elevetorState = 0;
};

#endif // ELEVATORACTOR_H_INCLUDED
