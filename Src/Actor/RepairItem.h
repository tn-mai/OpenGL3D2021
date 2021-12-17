/**
* @file RepairItem.h
*/
#ifndef REPAIRITEM_H_INCLUDED
#define REPAIRITEM_H_INCLUDED
#include "../Actor.h"

/**
* HPèCóùÉAÉCÉeÉÄ
*/
class RepairItem : public Actor
{
public:
  explicit RepairItem(const glm::vec3& position);
  virtual ~RepairItem() = default;
  virtual std::shared_ptr<Actor> Clone() const {
    std::shared_ptr<RepairItem> clone(new RepairItem(*this));
    return clone;
  }

  virtual void OnUpdate(float) override;
  virtual void OnTrigger(std::shared_ptr<Actor> other) override;
};

#endif // REPAIRITEM_H_INCLUDED
