/**
* @file Actor.cpp
*/
#include "Actor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

/**
* �R���X�g���N�^
*/
Actor::Actor(
  const std::string& name,
  const Primitive& prim,
  std::shared_ptr<Texture> tex,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment) :
  name(name),
  prim(prim),
  tex(tex),
  position(position),
  scale(scale),
  rotation(rotation),
  adjustment(adjustment)
{
}

/**
* �A�N�^�[�̏�Ԃ��X�V����
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b)
*/
void Actor::OnUpdate(float deltaTime)
{
  // �������Ȃ�
}

/**
* �Փ˂���������
*
* @param contact �Փˏ��
*/
void Actor::OnCollision(const struct Contact& contact)
{
  // �������Ȃ�
}

/**
* ���̂�`�悷��.
*/
void Draw(
  const Actor& actor,              // ���̂̐���p�����[�^
  const ProgramPipeline& pipeline, // �`��Ɏg���v���O�����p�C�v���C��
  glm::mat4 matProj,               // �`��Ɏg���v���W�F�N�V�����s��
  glm::mat4 matView)               // �`��Ɏg���r���[�s��  
{
  // ���f���s����v�Z����
  glm::mat4 matT = glm::translate(glm::mat4(1), actor.position);
  glm::mat4 matR = glm::rotate(glm::mat4(1), actor.rotation, glm::vec3(0, 1, 0));
  glm::mat4 matS = glm::scale(glm::mat4(1), actor.scale);
  glm::mat4 matA = glm::translate(glm::mat4(1), actor.adjustment);
  glm::mat4 matModel = matT * matR * matS * matA;

  // MVP�s����v�Z����
  glm::mat4 matMVP = matProj * matView * matModel;

  // ���f���s���MVP�s���GPU�������ɃR�s�[����
  const GLint locMatTRS = 0;
  const GLint locMatModel = 1;
  pipeline.SetUniform(locMatTRS, matMVP);
  if (actor.layer == Layer::Default) {
    pipeline.SetUniform(locMatModel, matModel);
  }

  // TODO: �e�L�X�g���ǉ�
  const GLint locColor = 200;
  pipeline.SetUniform(locColor, actor.color);

  if (actor.tex) {
    actor.tex->Bind(0); // �e�N�X�`�������蓖�Ă�
  } else {
  }
  actor.prim.Draw();  // �v���~�e�B�u��`�悷��
}

/**
* ���O�̈�v����A�N�^�[����������.
*
* @param actors �����Ώۂ̔z��.
* @param name   ��������A�N�^�[�̖��O.
*
* @retval nullptr�ȊO �ŏ���name�Ɩ��O�̈�v�����A�N�^�[.
* @retval nullptr     actors�̒��ɖ��O�̈�v����A�N�^�[���Ȃ�.
*/
std::shared_ptr<Actor> Find(std::vector<std::shared_ptr<Actor>>& actors, const char* name)
{
  for (int i = 0; i < actors.size(); ++i) {
    if (actors[i]->name == name) {
      return actors[i];
    }
  }
  return nullptr;
}

/**
* A��B�����ւ���.
*/
Contact Reverse(const Contact& contact)
{
  Contact result;

  result.a = contact.b;
  result.b = contact.a;
  result.velocityA = contact.velocityB;
  result.velocityB = contact.velocityA;
  result.accelA = contact.accelB;
  result.accelB = contact.accelA;
  result.penetration = -contact.penetration;
  result.normal = -contact.normal;
  result.position = contact.position;
  result.penLength = contact.penLength;
  result.massB = contact.a->mass;

  return result;
}

bool DummyCollisionFunc(Actor&, Actor&, Contact&)
{
  return false;
}

/**
* �Փ˂����o����
*
* @param actorA  �Փ˂��Ă��邩���ׂ�A�N�^�[
* @param actorB  �Փ˂��Ă��邩���ׂ�A�N�^�[
* @param contact �Փˏ��
*
* @retval true  �Փ˂��Ă���
* @retval false �Փ˂��Ă��Ȃ�
*/
bool DetectCollision(Actor& actorA, Actor& actorB, Contact& contact)
{
  // ���ʂ������Ȃ����̓��m�͏Փ˂��Ȃ�
  if (actorA.mass <= 0 && actorB.mass <= 0) {
    return false;
  }

  // �������Ȃ����̓��m�͏Փ˂��Ȃ�
  if (actorA.isStatic && actorB.isStatic) {
    return false;
  }

  // �R���C�_�[���ݒ肳��Ă��Ȃ����̂͏Փ˂��Ȃ�
  if (!actorA.collider || !actorB.collider) {
    return false;
  }

  using CollisionFunc = bool(*)(Actor&, Actor&, Contact&);
  static const CollisionFunc funcArray[2][2] = {
    //              box, cylinder
    /* box      */ { CollisionBoxBox, CollisionBoxCylinder },
    /* cylinder */ { CollisionCylinderBox, CollisionCylinderCylinder },
  };
  const int y = static_cast<int>(actorA.collider->shapeType);
  const int x = static_cast<int>(actorB.collider->shapeType);
  return funcArray[y][x](actorA, actorB, contact);
}

bool CollisionBoxBox(Actor& actorA, Actor& actorB, Contact& contact)
{
  // ���[���h���W�n�̏Փː}�`���v�Z����
  Box a = static_cast<Box&>(*actorA.collider);
  a.min += actorA.position;
  a.max += actorA.position;

  Box b = static_cast<Box&>(*actorB.collider);
  b.min += actorB.position;
  b.max += actorB.position;

  // a�̍����ʂ�b�̉E���ʂ��E�ɂ���Ȃ�A�Փ˂��Ă��Ȃ�
  const float dx0 = b.max.x - a.min.x;
  if (dx0 <= 0) {
    return false;
  }
  // a�̉E���ʂ�b�̍����ʂ�荶�ɂ���Ȃ�A�Փ˂��Ă��Ȃ�
  const float dx1 = a.max.x - b.min.x;
  if (dx1 <= 0) {
    return false;
  }

  // a�̉��ʂ�b�̏�ʂ���ɂ���Ȃ�A�Փ˂��Ă��Ȃ�
  const float dy0 = b.max.y - a.min.y;
  if (dy0 <= 0) {
    return false;
  }
  // a�̏�ʂ�b�̉��ʂ�艺�ɂ���Ȃ�A�Փ˂��Ă��Ȃ�
  const float dy1 = a.max.y - b.min.y;
  if (dy1 <= 0) {
    return false;
  }

  // a�̉����ʂ�b�̎�O���ʂ���O�ɂ���Ȃ�A�Փ˂��Ă��Ȃ�
  const float dz0 = b.max.z - a.min.z;
  if (dz0 <= 0) {
    return false;
  }
  // a�̎�O���ʂ�b�̉����ʂ�艜�ɂ���Ȃ�A�Փ˂��Ă��Ȃ�
  const float dz1 = a.max.z - b.min.z;
  if (dz1 <= 0) {
    return false;
  }

  // �ǂ��炩�A�܂��͗����̃A�N�^�[���u�u���b�N���Ȃ��v�ꍇ�A�ڐG�������s��Ȃ�
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

#if 0
  return true;
#endif

  // XYZ�̊e���ɂ��ďd�Ȃ��Ă��鋗�����Z��������I������
  glm::vec3 normal;  // �Փ˖�(�A�N�^�[B�̂����ꂩ�̖�)�̖@��
  glm::vec3 penetration; // �d�Ȃ��Ă��鋗���ƕ���
  if (dx0 <= dx1) {
    penetration.x = -dx0;
    normal.x = 1;
  } else {
    penetration.x = dx1;
    normal.x = -1;
  }
  if (dy0 <= dy1) {
    penetration.y = -dy0;
    normal.y = 1;
  } else {
    penetration.y = dy1;
    normal.y = -1;
  }
  if (dz0 <= dz1) {
    penetration.z = -dz0;
    normal.z = 1;
  } else {
    penetration.z = dz1;
    normal.z = -1;
  }

  // �d�Ȃ��Ă��鋗���̐�Βl
  glm::vec3 absPenetration = abs(penetration);

  // �Փ˖ʂɂȂ�\���̍���
  glm::vec3 score = glm::vec3(0);

  // �Z�����������Z�������قǏՓ˖ʂł���\���������͂�
  for (int a = 0; a < 2; ++a) {
    for (int b = a + 1; b < 3; ++b) {
      if (absPenetration[a] < absPenetration[b]) {
        ++score[a];
      } else {
        ++score[b];
      }
    }
  }

#if 1
  // ���΃x���V�e�B���v�Z����
  glm::vec3 rv = actorA.velocity - actorB.velocity;

  // �Z�����n�܂�������t���v�Z����
  glm::vec3 t(-FLT_MAX);
  for (int i = 0; i < 3; ++i) {
    if (rv[i]) {
      t[i] = penetration[i] / rv[i];
    }
  }

  // �Z���ɕK�v�Ȏ���t�������قǁA���̕�����葁�����_�ŏՓ˂����ƍl������
  // �������At��0�b�����̏ꍇ�A�Փ˖ʂ��牓����������Ɉړ����Ă��邽�ߏ��O����
  // �܂��At��1/60�b��蒷���ꍇ�A�{���Ȃ�Փ˂��Ă��Ȃ��͂��Ȃ̂ŏ��O����
  float deltaTime = 1.0f / 60.0f;
  for (int a = 0; a < 2; ++a) {
    for (int b = a + 1; b < 3; ++b) {
      int i = a;
      if (t[a] < t[b]) {
        i = b;
      }
      if (t[i] > 0 && t[i] <= deltaTime) {
        score[i] += 1.5f;
      }
    }
  }
#endif

  // ���\�����Ⴂ���������O����
  // �l���������ꍇ�AZ,X,Y�̏��ŗD��I�ɏ��O����
  if (score.x <= score.y) {
    normal.x = 0;
    penetration.x = 0;
    if (score.z <= score.y) {
      normal.z = 0;
      penetration.z = 0;
    } else {
      normal.y = 0;
      penetration.y = 0;
    }
  } else {
    normal.y = 0;
    penetration.y = 0;
    if (score.z <= score.x) {
      normal.z = 0;
      penetration.z = 0;
    } else {
      normal.x = 0;
      penetration.x = 0;
    }
  }

#if 0
  // XYZ���̂����A�Z���������ł��Z�����̐����������c��
  // NOTE: �Z�������ƐڐG�ʂ̖@���͈�v���Ȃ��ꍇ������
  if (absPenetration.x >= absPenetration.y) {
    penetration.x = 0;
    if (absPenetration.z >= absPenetration.y) {
      penetration.z = 0;
    } else {
      penetration.y = 0;
    }
  } else {
    penetration.y = 0;
    if (absPenetration.x >= absPenetration.z) {
      penetration.x = 0;
    } else {
      penetration.z = 0;
    }
  }
#endif

  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;
  contact.penetration = penetration;
  contact.normal = normal;
  contact.massB = actorB.mass;

  // �Փ˖ʂ̍��W���v�Z����
  {
    // ��{�I�ɃA�N�^�[B�̍��W���g�����A�A�N�^�[B���Õ��̏ꍇ�̓A�N�^�[A�̍��W���g��
    Actor* target = &actorB;
    glm::vec3 targetNormal = normal;
    if (actorB.isStatic) {
      target = &actorA;
      targetNormal *= -1; // �@���̌����𔽓]����
    }
    // �R���C�_�[�̔��a���v�Z����
    const Box& targetBox = static_cast<Box&>(*target->collider);
    glm::vec3 halfSize = (targetBox.max - targetBox.min) * 0.5f;
    // �R���C�_�[�̒��S���W���v�Z����
    glm::vec3 center = (targetBox.max + targetBox.min) * 0.5f;
    // �Փ˖ʂ̍��W���v�Z����
    contact.position = target->position + center - halfSize * targetNormal;
  }

  // �Z�������̒������v�Z����
  contact.penLength = glm::length(penetration);

  // �Փ˂��Ă���
  return true;
}

/**
* �~���Ɖ~���̏Փ�
*/
bool CollisionCylinderCylinder(Actor& actorA, Actor& actorB, Contact& contact)
{
  // ���[���h���W�n�̏Փː}�`���v�Z����
  Cylinder a = static_cast<Cylinder&>(*actorA.collider);
  a.bottom += actorA.position;

  Cylinder b = static_cast<Cylinder&>(*actorB.collider);
  b.bottom += actorB.position;

  // a�̉��ʂ�b�̏�ʂ���ɂ���Ȃ�A�Փ˂��Ă��Ȃ�
  const float dy0 = (b.bottom.y + b.height) - a.bottom.y;
  if (dy0 <= 0) {
    return false;
  }
  // a�̏�ʂ�b�̉��ʂ�艺�ɂ���Ȃ�A�Փ˂��Ă��Ȃ�
  const float dy1 = (a.bottom.y + a.height) - b.bottom.y;
  if (dy1 <= 0) {
    return false;
  }

  // XZ���ʏ�̋��������a�̍��v��艓����΁A�Փ˂��Ă��Ȃ�
  const float dx = b.bottom.x - a.bottom.x;
  const float dz = b.bottom.z - a.bottom.z;
  const float d2 = dx * dx + dz * dz;
  const float r = a.radius + b.radius;
  if ( d2 > r * r) {
    return false;
  }

  // �ǂ��炩�A�܂��͗����̃A�N�^�[���u�u���b�N���Ȃ��v�ꍇ�A�ڐG�������s��Ȃ�
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

  // XZ�����̐Z���������v�Z����
  glm::vec3 normal(0);
  glm::vec3 penetration(0);
  const float d = std::sqrt(d2);
  const float invD = 1.0f / d;
  normal.x = -dx * invD;
  normal.z = -dz * invD;
  const float lengthXZ = r - d;
  penetration.x = -lengthXZ * normal.x;
  penetration.z = -lengthXZ * normal.z;

  // Y�����̐Z���������v�Z����
  if (dy0 < dy1) {
    penetration.y = -dy0;
    normal.y = 1;
  } else {
    penetration.y = dy1;
    normal.y = -1;
  }

  // �Z�������̒Z����������Փ˂����Ƃ݂Ȃ�
  if (std::abs(penetration.y) <= lengthXZ) {
    // Y�����̏Փ�
    penetration.x = penetration.z = 0;
    normal.x = normal.z = 0;
  } else {
    // XZ�����̏Փ�
    penetration.y = 0;
    normal.y = 0;
  }

  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;
  contact.penetration = penetration;
  contact.normal = normal;
  contact.massB = actorB.mass;

  // �Փ˖ʂ̍��W���v�Z����
  {
    // ��{�I�ɃA�N�^�[B�̍��W���g�����A�A�N�^�[B���Õ��̏ꍇ�̓A�N�^�[A�̍��W���g��
    Actor* target = &actorB;
    Cylinder* targetCollider = &b;
    glm::vec3 targetNormal = normal;
    if (actorB.isStatic) {
      target = &actorA;
      targetCollider = &a;
      targetNormal *= -1; // �@���̌����𔽓]����
    }

    // �Փ˖ʂ̍��W���v�Z����
    if (normal.y) {
      // Y�����̏Փ˂̏ꍇ�E�E�E
      if (targetNormal.y < 0) {
        contact.position.y = targetCollider->bottom.y;
      } else {
        contact.position.y = targetCollider->bottom.y + targetCollider->height;
      }
      contact.position.x = (a.bottom.x + b.bottom.x) * 0.5f;
      contact.position.z = (a.bottom.z + b.bottom.z) * 0.5f;
    } else {
      // XZ�����̏Փ˂̏ꍇ�E�E�E
      contact.position.y = targetCollider->bottom.y + targetCollider->height * 0.5f;
      contact.position.x = targetCollider->bottom.x - targetNormal.x * targetCollider->radius;
      contact.position.z = targetCollider->bottom.z - targetNormal.z * targetCollider->radius;
    }
  }

  // �Z�������̒������v�Z����
  contact.penLength = glm::length(penetration);

  // �Փ˂��Ă���
  return true;
}

/**
* �_�ɍł��߂������̓��̍��W(�ŋߐړ_)�����߂�
*/
glm::vec3 ClosestPointXZ(const Box& box, const glm::vec3& p)
{
  const float cx = glm::clamp(p.x, box.min.x, box.max.x);
  const float cz = glm::clamp(p.z, box.min.z, box.max.z);
  return glm::vec3(cx, p.y, cz);
}

/**
*
*/
bool CollisionBoxCylinder(Actor& actorA, Actor& actorB, Contact& contact)
{
  // ���[���h���W�n�̏Փː}�`���v�Z����
  Box a = static_cast<Box&>(*actorA.collider);
  a.min += actorA.position;
  a.max += actorA.position;

  Cylinder b = static_cast<Cylinder&>(*actorB.collider);
  b.bottom += actorB.position;

  // a�̉��ʂ�b�̏�ʂ���ɂ���Ȃ�A�Փ˂��Ă��Ȃ�
  const float dy0 = (b.bottom.y + b.height) - a.min.y;
  if (dy0 <= 0) {
    return false;
  }
  // a�̏�ʂ�b�̉��ʂ�艺�ɂ���Ȃ�A�Փ˂��Ă��Ȃ�
  const float dy1 = a.max.y - b.bottom.y;
  if (dy1 <= 0) {
    return false;
  }

  // �~���̒��S�ɍł��߂������̓��̍��W(�ŋߐړ_)�����߂�
  const glm::vec3 closestPoint = ClosestPointXZ(a, b.bottom);

  // �~���̒��S����ŋߐړ_�܂ł̋������v�Z����
  const float dx = closestPoint.x - b.bottom.x;
  const float dz = closestPoint.z - b.bottom.z;
  const float d2 = dx * dx + dz * dz;

  // �ŋߐړ_�܂ł̋������~���̔��a��蒷����΁A�Փ˂��Ă��Ȃ�
  if (d2 > b.radius * b.radius) {
    return false;
  }

  // �ǂ��炩�A�܂��͗����̃A�N�^�[���u�u���b�N���Ȃ��v�ꍇ�A�ڐG�������s��Ȃ�
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

  // Y�����̐Z���������v�Z����
  glm::vec3 penetration(0);
  glm::vec3 normal(0);
  if (dy0 < dy1) {
    penetration.y = -dy0;
    normal.y = 1;
  } else {
    penetration.y = dy1;
    normal.y = -1;
  }

  // �ŋߐړ_c�������̂̃J�h(�p)�ɂ���ꍇ�A�p����ɐZ���������v�Z����
  bool isCorner = false;
  if ((closestPoint.x == a.min.x || closestPoint.x == a.max.x) &&
    (closestPoint.z == a.min.z || closestPoint.z == a.max.z)) {
    isCorner = true;
    const float d = std::sqrt(d2);
    const float invD = 1.0f / d;
    normal.x = dx * invD;
    normal.z = dz * invD;
    const float lengthXZ = b.radius - d;
    penetration.x = -lengthXZ * normal.x;
    penetration.z = -lengthXZ * normal.z;
  } else {
    // X�����̐Z���������v�Z����
    const float dx0 = (b.bottom.x + b.radius) - a.min.x;
    const float dx1 = (b.bottom.x - b.radius) - a.max.x;
    if (dx0 <= -dx1) {
      penetration.x = -dx0;
      normal.x = 1;
    } else {
      penetration.x = -dx1;
      normal.x = -1;
    }

    // Z�����̐Z���������v�Z����
    const float dz0 = (b.bottom.z + b.radius) - a.min.z;
    const float dz1 = (b.bottom.z - b.radius) - a.max.z;
    if (dz0 <= -dz1) {
      penetration.z = -dz0;
      normal.z = 1;
    } else {
      penetration.z = -dz1;
      normal.z = -1;
    }
  }

  // �Z���������������������O����
  // �R�[�i�[�ŏՓ˂��Ă���ꍇ�AXZ�x�N�g���̒�����Y���r����
  const glm::vec3 absPenetration = glm::abs(penetration);
  if (isCorner && absPenetration.y > glm::length(glm::vec2(penetration.x, penetration.z))) {
    penetration.y = 0;
    normal.y = 0;
  } else {
    // �R�[�i�[�ȊO�̏ꍇ�A�ł��Z���������Z�������c��
    if (absPenetration.x >= absPenetration.y) {
      normal.x = 0;
      penetration.x = 0;
      if (absPenetration.z >= absPenetration.y) {
        normal.z = 0;
        penetration.z = 0;
      } else {
        normal.y = 0;
        penetration.y = 0;
      }
    } else {
      normal.y = 0;
      penetration.y = 0;
      if (absPenetration.x >= absPenetration.z) {
        normal.x = 0;
        penetration.x = 0;
      } else {
        normal.z = 0;
        penetration.z = 0;
      }
    }
  }

  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;
  contact.penetration = penetration;
  contact.normal = normal;
  contact.massB = actorB.mass;

  // �Փ˖ʂ̍��W���v�Z����
  {
    // ��{�I�ɃA�N�^�[B�̍��W���g�����A�A�N�^�[B���Õ��̏ꍇ�̓A�N�^�[A�̍��W���g��
    if (actorB.isStatic) {
      if (normal.y) {
        contact.position.x = closestPoint.x;
        contact.position.y = (a.min.y + a.max.y) * 0.5f - (a.max.y - a.min.y) * 0.5f * normal.y;
        contact.position.z = closestPoint.z;
      } else {
        contact.position.x = (a.min.x + a.max.x) * 0.5f - (a.max.x - a.min.x) * 0.5f * normal.x;
        contact.position.y = (a.min.y + a.max.x) * 0.5f;
        contact.position.z = (a.min.z + a.max.z) * 0.5f - (a.max.z - a.min.z) * 0.5f * normal.z;
      }
    } else {
      // �Փ˖ʂ̍��W���v�Z����
      if (normal.y) {
        // Y�����̏Փ˂̏ꍇ�E�E�E
        contact.position.x = closestPoint.x;
        contact.position.y = b.bottom.y + b.height * (0.5f + 0.5f * normal.y);
        contact.position.z = closestPoint.z;
      } else {
        // XZ�����̏Փ˂̏ꍇ�E�E�E
        contact.position.x = b.bottom.x - normal.x * b.radius;
        contact.position.y = a.min.y + dy0 * 0.5f;
        contact.position.z = b.bottom.z - normal.z * b.radius;
      }
    }
  }

  // �Z�������̒������v�Z����
  contact.penLength = glm::length(penetration);

  // �Փ˂��Ă���
  return true;
}

/**
*
*/
bool CollisionCylinderBox(Actor& actorA, Actor& actorB, Contact& contact)
{
  const bool result = CollisionBoxCylinder(actorB, actorA, contact);
  if (result) {
    //std::swap(contact.a, contact.b);
    //std::swap(contact.velocityA, contact.velocityB);
    //std::swap(contact.accelA, contact.accelB);
  }
  return result;
}


/**
* �d�Ȃ����������
*
* @param contact �Փˏ��

@todo contact���A�N�^�[���ɐ���(�܂�1�Փ˂ɂ�2��)
@todo �A�N�^�[�y�і@���̓�����contact�𓝍�(Equal�֐��̏C��)
*/
void SolveContact(Contact& contact)
{
  Actor& actorA = *contact.a;
  Actor& actorB = *contact.b;
  glm::vec3 normal = contact.normal;
  glm::vec3 penetration = contact.penetration;

  // �����W���̎� e = (Va' - Vb') / (Vb - Va) ���� e = 1 �̂Ƃ�
  //   Va' - Vb' = Vb - Va ...��(1)
  // �^���ʕۑ������
  //   Ma*Va + Mb*Vb = Ma*Va' + Mb*Vb' ...��(2)
  // ��(1)��Vb'�ɂ��ĉ�����
  //  -Vb' = Vb - Va - Va'
  //   Vb' = -Vb + Va + Va'
  //   Vb' = Va - Vb + Va' ...��(3)
  // ��(3)����(2)�ɑ�������
  //   Ma*Va + Mb*Vb = Ma*Va' + Mb(Va - Vb + Va') ...��(4)
  // ��(4)��Va'�ɂ��ĉ�����
  //   Ma*Va + Mb*Vb = Ma*Va' + Mb*Va - Mb*Vb + Mb*Va'
  //   Ma*Va + Mb*Vb = (Ma+Mb)Va' + Mb*Va - Mb*Vb
  //   Ma*Va + Mb*Vb - (Ma+Mb)Va' = Mb*Va - Mb*Vb
  //   -(Ma+Mb)Va' = Mb*Va - Mb*Vb - Ma*Va - Mb*Vb
  //   (Ma+Mb)Va' = -Mb*Va + Mb*Vb + Ma*Va + Mb*Vb
  //   (Ma+Mb)Va' = Ma*Va + Mb*Vb + Mb(Vb - Va)
  //   Va' = (Ma*Va + Mb*Vb + Mb(Vb - Va)) / (Ma+Mb) ...��(5)
  //
  // ���l��Vb'�ɂ��ĉ�����
  //   Ma*Va + Mb*Vb = Ma*Vb - Ma*Va + Ma*Vb' + Mb*Vb'
  //   Ma*Va + Mb*Vb - Ma*Vb + Ma*Va = (Ma+Mb)Vb'
  //   Vb' = (Ma*Va + Mb*Vb + Ma(Va - Vb)) / (Ma+Mb)
  //
  // A���Î~���̏ꍇ�AB�̎���Mb��0�Ƃ��Ď�(5)������. �����
  //   Va' = (Ma*Va + 0*Vb + 0(Vb - Va)) / (Ma+0)
  //   Va' = Ma*Va / Ma
  //   Va' = Va
  //
  // NOTE: ���C�Ɣ����͔��ɕ��G�ȕ������ۂȂ̂ŁA�����������𗧂Ă邱�Ƃ͕s�\�ɋ߂�.
  //       ���̂��߁A�����G���W���ł͕����I�Ȑ��m���𖳎����ēK���Ɍv�Z���Ă���.
  //       Bullet3�͏�Z�AUE4�AUnity�̓f�t�H���g�ł͕��ς��g���Ă���悤��.
  //       Box2D-lite�ł͏�Z���ĕ�����������Ă���. �d���Ȃ邪���ʂ𐧌䂵�₷���悤��.
  //       ����͒������̂Ɋ�����邱�Ƃɂ��āA���ς��̗p����.

  // �����W���̕��ϒl���v�Z
  float cor = (actorA.cor + actorB.cor) * 0.5f;

  // ���C�W���̕��ϒl���v�Z
  float friction = (actorA.friction + actorB.friction) * 0.5f;

  // �A�N�^�[B�ɑ΂���A�N�^�[A�̑��΃x���V�e�B���v�Z
  glm::vec3 rv = contact.velocityA - contact.velocityB;

  // �Փ˖ʂƑ��΃x���V�e�B�ɕ��s�ȃx�N�g��(�^���W�F���g)���v�Z
  glm::vec3 tangent = glm::cross(normal, glm::cross(normal, rv));
  if (glm::length(tangent) > 0.000001f) {
    tangent = glm::normalize(tangent);
  } else {
    tangent = glm::vec3(0);
  }
  
  // ���C����󂯂�x���V�e�B���v�Z
#if 1
  // ���C��
  float frictionForce = friction * 9.8f / 60.0f;

  // ���C�͂̍ő�l���v�Z
  float maxForce = std::abs(glm::dot(tangent, rv));

  // ���C�͂��ő�l�ɐ���
  frictionForce = std::min(frictionForce, maxForce);

  // �^���W�F���g�����̖��C�͂��v�Z
  glm::vec3 frictionVelocity = normal.y * frictionForce * tangent;
#else
  glm::vec3 relAccel = (contact.accelA - contact.accelB);
  float maxf = abs(glm::dot(tangent, rv));
  float tmp = glm::dot(normal, relAccel);
  float ff = std::min(maxf, abs(tmp));
  if (tmp < 0) {
    ff *= -1;
  }
  glm::vec3 frictionVelocity = friction * ff * tangent;
#endif

  // ���x�̍X�V: �@�������̑��x���v�Z
  float ua = glm::dot(normal, contact.velocityA);
  float ub = glm::dot(normal, contact.velocityB);
  if (actorA.isStatic) {
#if 0
    const float ratio = 1.0f / actorB.contactCount;
    float vb = ua + cor * (ua - ub);     // �Փˌ�̑��x���v�Z
    actorB.velocity -= normal * ub * ratio;      // �ՓˑO�̑��x��0�ɂ���
    actorB.velocity += normal * vb * ratio;      // �Փˌ�̑��x�����Z����
    actorB.velocity += frictionVelocity * ratio; // ���C�ɂ�鑬�x�����Z����

    // �d�Ȃ�̉���: �A�N�^�[A�͓����Ȃ��̂ŁA�A�N�^�[B�𓮂���
    actorB.position += penetration * ratio;
    if (normal.y < 0) {
      //float vy = actorB.velocity.y - actorA.velocity.y;
      //if ( vy < 1.0f) {
        actorB.isOnActor = true;
      //}
    }
#endif
  }
  else if (actorB.isStatic) {
    const float ratio = 1.0f / actorA.contactCount;
    float va = ub + cor * (ub - ua);     // �Փˌ�̑��x���v�Z
    actorA.velocity -= normal * ua * ratio;      // �ՓˑO�̑��x��0�ɂ���
    actorA.velocity += normal * va * ratio;      // �Փˌ�̑��x�����Z����
    actorA.velocity += frictionVelocity * ratio; // ���C�ɂ�鑬�x�����Z����

    // �d�Ȃ�̉���: �A�N�^�[B�͓����Ȃ��̂ŁA�A�N�^�[A�𓮂���
    actorA.position -= penetration * ratio;

    if (normal.y > 0) {
      //float vy = actorA.velocity.y - actorB.velocity.y;
      //if ( vy < 1.0f) {
        actorA.isOnActor = true;
      //}
    }
  }
  else {
    // ���x�̍X�V: �^���G�l���M�[�̕��z�ʂ��v�Z
    const float ratioA = 1.0f / actorA.contactCount;
    const float ratioB = 1.0f;// / actorB.contactCount;
    float massA = actorA.mass * ratioA;
    float massB = contact.massB;// actorB.mass* ratioB;
    float massAB = massA + massB;
    float c = massA * ua + massB * ub;
    float va = (c + cor * massB * (ub - ua)) / massAB;
    float vb = (c + cor * massA * (ua - ub)) / massAB;

    // �ՓˑO�̑��x��0�ɂ���
    actorA.velocity -= normal * ua;
    //actorB.velocity -= normal * ub;

    // �Փˌ�̑��x�����Z����
    actorA.velocity += normal * va;
    //actorB.velocity += normal * vb;

    // ���C�ɂ�鑬�x�����Z����
    actorA.velocity += frictionVelocity * ratioA;
    //actorB.velocity -= frictionVelocity * ratioB;

    // �d�Ȃ�̉���: �^���G�l���M�[�̔䗦�œ��������������߂�
    //              �^���G�l���M�[���[���̏ꍇ�͎��ʂ̔䗦�Ō��߂�
    float rA = abs(va);
    float rB = abs(vb);
    if (rA <= 0.0f && rB <= 0.0f) {
      rA = massA;
      rB = massB;
    }
    glm::vec3 pa = penetration * rA / (rA + rB);
    glm::vec3 pb = penetration * rB / (rA + rB);
    actorA.position -= pa * ratioA;
    //actorB.position += pb * ratioB;

    if (normal.y > 0) {
      //float vy = actorA.velocity.y - actorB.velocity.y;
      //if ( vy < 1.0f) {
        actorA.isOnActor = true;
      //}
    } else if (normal.y < 0) {
      //float vy = actorB.velocity.y - actorB.velocity.y;
      //if ( vy < 1.0f) {
//        actorB.isOnActor = true;
      //}
    }
  }
}

/**
* 2�̃R���^�N�g�\���̂����Ă��邩���ׂ�
*
* @param ca ��r����R���^�N�g�\���̂���1.
* @param cb ��r����R���^�N�g�\���̂���2.
*
* @return true  ���Ă���
* @return false ���Ă��Ȃ�
*/
bool Equal(const Contact& ca, const Contact& cb)
{
  // �Փ˖ʂ̋���������Ă���ꍇ�͎��Ă��Ȃ�
  // NOTE: ���̔���͊��S�ł͂Ȃ�.
  //       DetectCollision�͈����œn���ꂽ2�̃A�N�^�[A, B�̂����AB�̕\�ʂɐڐG�_���`����.
  //       ���̂��߁A�A�N�^�[X�ƃA�N�^�[Y���������Ƃ��AA=X, B=Y�Ƃ��邩A=Y, B=X�Ƃ��邩�ŐڐG�_�Ɩ@�����قȂ�.
  //       ���ׂĂ̌�_�ɐڐG�_�𐶐�����ƁA���̖��������ł��邩������Ȃ�.
  if (glm::length(ca.position - cb.position) > 0.01f) {
    return false; // ���Ă��Ȃ�
  }

#if 0 // ���݂̎����ł͍��W����v����Ȃ�@������v����
  // �@���̕�������v���Ȃ��ꍇ�͎��Ă��Ȃ�
  if (glm::dot(ca.normal, normalB) <= 0) {
    return false; // ���Ă��Ȃ�
  }
#endif

  // �Õ��A�N�^�[�̗L���ɂ���Ĕ���𕪂���
  glm::vec3 normalB = cb.normal;
  bool hasStaticA = ca.a->isStatic || ca.b->isStatic;
  bool hasStaticB = cb.a->isStatic || cb.b->isStatic;
  switch (hasStaticA + hasStaticB * 2) {
  case 0b00: // A,B�Ƃ��ɓ����A�N�^�[�̂�
    // �A�N�^�[�������Ƃ���v�����玗�Ă���
    if (ca.a == cb.a && ca.b == cb.b) {
      break;
    }
    if (ca.a == cb.b && ca.b == cb.a) {
      break;
    }
    return false;

  case 0b01: // A=�����Ȃ��A�N�^�[���܂�, B=�����A�N�^�[�̂�
    // ��Ɏ��Ă��Ȃ��Ɣ��肷��
    return false;

  case 0b10: // A=�����A�N�^�[�̂� B=�����Ȃ��A�N�^�[���܂�
    // ��Ɏ��Ă��Ȃ��Ɣ��肷��
    return false;

  case 0b11: // A,B�Ƃ��ɓ����Ȃ��A�N�^�[���܂� 
    {
    // �����A�N�^�[���m����v�����玗�Ă���
    Actor* a = ca.a;
    if (ca.a->isStatic) {
      a = ca.b;
    }
    Actor* b = cb.a;
    if (cb.a->isStatic) {
      b = cb.b;
    }
    if (a == b) {
      break;
    }
    }
    return false;
  }

  return true; // ���Ă���
}

/**
* Contact�\���̂�A,B���ꂼ��ɍ쐬����ꍇ�̔�r�֐�.
* 
* �A�N�^�[A�Ɩ@�����������ꍇ��true
* �܂�AA�ɓ�����������ڐG�����ꍇ�͓������Ɣ��肷��.
*/
bool Equal2(const Contact& ca, const Contact& cb)
{
  if (ca.a != cb.a) {
    return false;
  }
  if (glm::dot(ca.normal, cb.normal) <= glm::cos(glm::radians(1.0f))) {
    return false;
  }
  return true;
}

