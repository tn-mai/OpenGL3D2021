/**
* @file Collision.h
*/
#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED
#include <glm/glm.hpp>
#include <memory>

// 先行宣言
class Actor;
struct Contact;

/**
* 図形の種類
*/
enum class ShapeType
{
  box,      // 直方体
  sphere,   // 球
  cylinder, // 円柱
};

/**
* コライダーの基底クラス
*/
class Collider
{
public:
  Collider(ShapeType type) : shapeType(type) {}
  virtual ~Collider() = default;

  virtual std::shared_ptr<Collider> Clone() const = 0;
  ShapeType GetShapeType() const { return shapeType; }
  virtual void RotateY(float radians) {}

private:
  ShapeType shapeType;
};

/**
* 直方体
*/
class Box : public Collider
{
public:
  // 直方体コライダーを作成する
  static std::shared_ptr<Box> Create(
    const glm::vec3& min, const glm::vec3& max)
  {
    return std::make_shared<Box>(min, max);
  }

  // コンストラクタ・デストラクタ
  Box() : Collider(ShapeType::box) {};
  Box(const glm::vec3& min, const glm::vec3& max) :
    Collider(ShapeType::box), min(min), max(max) {}
  virtual ~Box() = default;

  // クローンを作成する
  virtual std::shared_ptr<Collider> Clone() const override
  {
    return std::make_shared<Box>(*this);
  }
  virtual void RotateY(float radians) override;

  glm::vec3 min = glm::vec3(0);
  glm::vec3 max = glm::vec3(0);
};

/**
* 円柱
*/
class Cylinder : public Collider
{
public:
  // 垂直円柱コライダーを作成する
  static std::shared_ptr<Cylinder> Create(
    const glm::vec3& bottom, float radius, float height)
  {
    return std::make_shared<Cylinder>(bottom, radius, height);
  }

  // コンストラクタ・デストラクタ
  Cylinder() : Collider(ShapeType::cylinder) {}
  Cylinder(const glm::vec3& bottom, float radius, float height) :
    Collider(ShapeType::cylinder), bottom(bottom), radius(radius), height(height)
  {}
  virtual ~Cylinder() = default;

  // クローンを作成する
  virtual std::shared_ptr<Collider> Clone() const override
  {
    return std::make_shared<Cylinder>(*this);
  }

  glm::vec3 bottom = glm::vec3(0); // 下端の座標
  float radius = 1.0f; // 半径
  float height = 1.0f; // 高さ
};

/**
* 球
*/
class Sphere : public Collider
{
public:
  // 球コライダーを作成する
  static std::shared_ptr<Sphere> Create(
    const glm::vec3& center, float radius)
  {
    return std::make_shared<Sphere>(center, radius);
  }

  // コンストラクタ・デストラクタ
  Sphere() : Collider(ShapeType::sphere) {}
  Sphere(const glm::vec3& center, float radius) :
    Collider(ShapeType::sphere), center(center), radius(radius)
  {}
  virtual ~Sphere() = default;

  // クローンを作成する
  virtual std::shared_ptr<Collider> Clone() const override
  {
    return std::make_shared<Sphere>(*this);
  }

  glm::vec3 center = glm::vec3(0); // 中心座標
  float radius = 1.0f; // 半径
};

/**
* 線分
*/
struct Segment
{
  glm::vec3 start;
  glm::vec3 end;
};

bool CollisionBoxBox(Actor& a, Actor& b, Contact& contact);
bool CollisionSphereSphere(Actor& a, Actor& b, Contact& contact);
bool CollisionCylinderCylinder(Actor& a, Actor& b, Contact& contact);
bool CollisionBoxSphere(Actor& a, Actor& b, Contact& contact);
bool CollisionBoxCylinder(Actor& a, Actor& b, Contact& contact);
bool CollisionSphereBox(Actor& a, Actor& b, Contact& contact);
bool CollisionSphereCylinder(Actor& a, Actor& b, Contact& contact);
bool CollisionCylinderBox(Actor& a, Actor& b, Contact& contact);
bool CollisionCylinderSphere(Actor& a, Actor& b, Contact& contact);

#endif // COLLISION_H_INCLUDED
