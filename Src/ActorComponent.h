/**
* @file ActorComponent.h
*/
#ifndef ACTORCOMOPNENT_H_INCLUDED
#define ACTORCOMOPNENT_H_INCLUDED
#include <memory>
#include <string>
#include <type_traits>

class Actor;
struct Contact;

#define DEFINE_ACTORCOMPONENT_ID(name, base) \
  public:\
  static size_t GetId() { \
    static const size_t id = std::hash<std::string>()(#name); \
    return id; } \
  virtual bool IsBaseOf(size_t id) const { \
    if (id == GetId()) { return true; } \
    return base::IsBaseOf(id); }

/**
* コンポーネントの基本クラス
*
* NOTE: このクラス、作ってはみたが、いまのところ使用していない.
*/
class ActorComponent
{
public:
  explicit ActorComponent(Actor* owner) : owner(owner) {}
  virtual ~ActorComponent() = 0 {}

  virtual void OnUpdate(float deltaTime) {}
  virtual void OnCollision(const Contact& contact) {}
  virtual void OnTrigger(std::shared_ptr<Actor> other) {}

  static size_t GetId() {
    static const size_t id = std::hash<std::string>()("ActorComponent");
    return id;
  }
  virtual bool IsBaseOf(size_t id) const { return id == GetId(); }

  template<typename T>
  bool IsA() const { return IsBaseOf(T::GetId()); }

private:
  Actor* owner = nullptr;
};

using ActorComponentPtr = std::shared_ptr<ActorComponent>;

class TestComponentA : public ActorComponent
{
  DEFINE_ACTORCOMPONENT_ID(TestComponentA, ActorComponent)

public:
  explicit TestComponentA(Actor* owner) : ActorComponent(owner) {}
  virtual ~TestComponentA() {}
};

#endif // ACTORCOMOPNENT_H_INCLUDED
