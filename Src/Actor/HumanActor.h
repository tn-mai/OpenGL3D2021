/**
* @file HumanActor.h
*/
#ifndef HUMANACTOR_H_INCLUDED
#define HUMANACTOR_H_INCLUDED
#include "../Actor.h"

/**
* 人間アクター
*/
class HumanActor : public Actor
{
public:
  HumanActor(
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation);

  virtual ~HumanActor() = default;
  virtual std::shared_ptr<Actor> Clone() const override {
    return std::make_shared<HumanActor>(*this);
  }
  virtual void OnUpdate(float deltaTime);

private:
  static const int groupBody = 0;
  static const int groupHead = 1;
  static const int groupArmR = 2;
  static const int groupArmL = 3;
  static const int groupLegR = 4;
  static const int groupLegL = 5;
};

#endif // HUMANACTOR_H_INCLUDED
