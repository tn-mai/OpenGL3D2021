/**
* @file Collision.h
*/
#ifndef COLLISION_H_INCLUDED
#define COLLISION_H_INCLUDED
#include <glm/glm.hpp>
#include <memory>

// ��s�錾
class Actor;
struct Contact;

/**
* �}�`�̎��
*/
enum class ShapeType
{
  box,      // ������
  sphere,   // ��
  cylinder, // �~��
};

/**
* �R���C�_�[�̊��N���X
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
* ������
*/
class Box : public Collider
{
public:
  // �����̃R���C�_�[���쐬����
  static std::shared_ptr<Box> Create(
    const glm::vec3& min, const glm::vec3& max)
  {
    return std::make_shared<Box>(min, max);
  }

  // �R���X�g���N�^�E�f�X�g���N�^
  Box() : Collider(ShapeType::box) {};
  Box(const glm::vec3& min, const glm::vec3& max) :
    Collider(ShapeType::box), min(min), max(max) {}
  virtual ~Box() = default;

  // �N���[�����쐬����
  virtual std::shared_ptr<Collider> Clone() const override
  {
    return std::make_shared<Box>(*this);
  }
  virtual void RotateY(float radians) override;

  glm::vec3 min = glm::vec3(0);
  glm::vec3 max = glm::vec3(0);
};

/**
* �~��
*/
class Cylinder : public Collider
{
public:
  // �����~���R���C�_�[���쐬����
  static std::shared_ptr<Cylinder> Create(
    const glm::vec3& bottom, float radius, float height)
  {
    return std::make_shared<Cylinder>(bottom, radius, height);
  }

  // �R���X�g���N�^�E�f�X�g���N�^
  Cylinder() : Collider(ShapeType::cylinder) {}
  Cylinder(const glm::vec3& bottom, float radius, float height) :
    Collider(ShapeType::cylinder), bottom(bottom), radius(radius), height(height)
  {}
  virtual ~Cylinder() = default;

  // �N���[�����쐬����
  virtual std::shared_ptr<Collider> Clone() const override
  {
    return std::make_shared<Cylinder>(*this);
  }

  glm::vec3 bottom = glm::vec3(0); // ���[�̍��W
  float radius = 1.0f; // ���a
  float height = 1.0f; // ����
};

/**
* ��
*/
class Sphere : public Collider
{
public:
  // ���R���C�_�[���쐬����
  static std::shared_ptr<Sphere> Create(
    const glm::vec3& center, float radius)
  {
    return std::make_shared<Sphere>(center, radius);
  }

  // �R���X�g���N�^�E�f�X�g���N�^
  Sphere() : Collider(ShapeType::sphere) {}
  Sphere(const glm::vec3& center, float radius) :
    Collider(ShapeType::sphere), center(center), radius(radius)
  {}
  virtual ~Sphere() = default;

  // �N���[�����쐬����
  virtual std::shared_ptr<Collider> Clone() const override
  {
    return std::make_shared<Sphere>(*this);
  }

  glm::vec3 center = glm::vec3(0); // ���S���W
  float radius = 1.0f; // ���a
};

/**
* ����
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
