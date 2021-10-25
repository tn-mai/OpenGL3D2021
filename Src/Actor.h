/**
* @file Actor.h
*/
#ifndef ACTOR_H_INCLUDED
#define ACTOR_H_INCLUDED
#include <glad/glad.h>
#include "Primitive.h"
#include "Texture.h"
#include "ProgramPipeline.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <memory>

/**
* ��11��e�L�X�g�������j:
* �G���W���N���X�ւ̓����̓R�X�g���������ߌ�����.
* 1. actors �z��� shared_ptr �z�񉻂���.
* 2. OnHit���z�֐�������.
* 3. EnemyActor�N���X���쐬���AOnHit���I�[�o�[���C�h.
* 4. BulletActor�N���X���쐬���AOnHit���I�[�o�[���C�h.
* 5. Tick���z�֐�������.
* 7. ElevetorActor�N���X���쐬���ATick���I�[�o�[���C�h.
* 8. EnemyActor�N���X��Tick���I�[�o�[���C�h.
* 9. PlayerActor�N���X���쐬���ATick���I�[�o�[���C�h.
*/

// �}�`�̎��
enum class ShapeType
{
  box,      // ������
  cylinder, // �~��
};

struct Collider
{
public:
  explicit Collider(ShapeType type) : shapeType(type) {}
  virtual ~Collider() = default;

  ShapeType shapeType;
};

/**
* ������
*/
struct Box : public Collider
{
  Box() : Collider(ShapeType::box) {};
  Box(const glm::vec3& min, const glm::vec3& max) :
    Collider(ShapeType::box), min(min), max(max)
  {}
  virtual ~Box() = default;

  glm::vec3 min = glm::vec3(0);
  glm::vec3 max = glm::vec3(0);
};

inline std::shared_ptr<Box> CreateBoxShape(
  const glm::vec3& min, const glm::vec3& max)
{
  return std::make_shared<Box>(min, max);
}

/**
* �~��
*/
struct Cylinder : public Collider
{
  Cylinder(const glm::vec3& bottom, float radius, float height) :
    Collider(ShapeType::cylinder), bottom(bottom), radius(radius), height(height)
  {}
  virtual ~Cylinder() = default;

  glm::vec3 bottom = glm::vec3(0); // ���[�̍��W
  float radius = 0.5f; // ���a
  float height = 1.0f; // ����
};

inline std::shared_ptr<Cylinder> CreateCylinderShape(
  const glm::vec3& bottom, float radius, float height)
{
  return std::make_shared<Cylinder>(bottom, radius, height);
}

/**
* �\�����C���[
*/
enum class Layer
{
  Default,
  UI,
};
static const size_t layerCount = 2; // ���C���[��

/**
* �V�F�[�_�̎��
*/
enum class Shader
{
  FragmentLighting,
  Ground,
};

/**
* ���̂𐧌䂷��p�����[�^.
*/
class Actor
{
public:
  Actor(
    const std::string& name,
    const Primitive& prim,
    std::shared_ptr<Texture> tex,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment);

  virtual ~Actor() = default;
  virtual std::shared_ptr<Actor> Clone() const {
    return std::shared_ptr<Actor>(new Actor(*this));
  }
  virtual void OnUpdate(float deltaTime);
  virtual void OnCollision(const struct Contact& contact);

  std::string name;                // �A�N�^�[�̖��O
  Primitive prim;                  // �`�悷��v���~�e�B�u
  std::shared_ptr<Texture> tex;    // �`��Ɏg���e�N�X�`��
  glm::vec3 position;              // ���̂̈ʒu
  glm::vec3 scale;                 // ���̂̊g��k����
  float rotation;                  // ���̂̉�]�p�x
  glm::vec3 adjustment;            // ���̂����_�Ɉړ����邽�߂̋���

  glm::vec3 velocity = glm::vec3(0);// ���x(���[�g�����b)
  float lifespan = 0;              // ����(�b�A0�ȉ��Ȃ�����Ȃ�)
  float health = 10;               // �ϋv�l
  bool isDead = false;             // false=���S(�폜�҂�) true=������

  std::shared_ptr<Collider> collider; // �Փ˔���
  float contactCount = 1;         // �ڐG�_�̐�
  float mass = 1;                  // ����(kg)
  float cor = 0.4f;                // �����W��
  float friction = 0.7f;           // ���C�W��
  bool isStatic = false;           // false=�������镨�� true=�������Ȃ����� 

  Layer layer = Layer::Default;    // �\�����C���[
  Shader shader = Shader::FragmentLighting;

  // TODO: �e�L�X�g���ǉ�
  glm::vec3 oldVelocity = glm::vec3(0); // �ȑO�̑��x(���[�g�����b)
  bool isBlock = true;             // false=�ʉ߂ł��� true=�ʉ߂ł��Ȃ�
  float gravityScale = 1.0f;       // �d�͌W��
  glm::vec4 color = glm::vec4(1);
  bool isOnActor = false;
};

void Draw(
  const Actor& actor,              // ���̂̐���p�����[�^
  const ProgramPipeline& pipeline, // �`��Ɏg���v���O�����p�C�v���C��
  glm::mat4 matProj,               // �`��Ɏg���v���W�F�N�V�����s��
  glm::mat4 matView);              // �`��Ɏg���r���[�s��  

std::shared_ptr<Actor> Find(std::vector<std::shared_ptr<Actor>>& actors, const char* name);

/**
* �Փˏ��
*/
struct Contact
{
  Actor* a = nullptr;
  Actor* b = nullptr;
  glm::vec3 velocityA;   // �Փˎ��_�ł̃A�N�^�[A�̃x���V�e�B
  glm::vec3 velocityB;   // �Փˎ��_�ł̃A�N�^�[B�̃x���V�e�B
  glm::vec3 accelA;      // �Փˎ��_�ł̃A�N�^�[A�̉����x
  glm::vec3 accelB;      // �Փˎ��_�ł̃A�N�^�[B�̉����x
  glm::vec3 penetration; // �Z������
  glm::vec3 normal;      // �Փ˖ʂ̖@��
  glm::vec3 position;    // �Փ˖ʂ̍��W
  float penLength;       // �Z�������̒���

  float massB;
};

Contact Reverse(const Contact& contact);

bool DetectCollision(Actor& a, Actor& b, Contact& contact);
bool CollisionBoxBox(Actor& a, Actor& b, Contact& contact);
bool CollisionCylinderCylinder(Actor& a, Actor& b, Contact& contact);
bool CollisionBoxCylinder(Actor& a, Actor& b, Contact& contact);
bool CollisionCylinderBox(Actor& a, Actor& b, Contact& contact);
void SolveContact(Contact& contact);
bool Equal(const Contact& ca, const Contact& cb);
bool Equal2(const Contact& ca, const Contact& cb);

#endif // ACTOR_H_INCLUDED
