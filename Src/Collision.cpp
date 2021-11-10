/**
* @file Collision.cpp
*/
#include "Collision.h"
#include "Actor.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// ���������_����0�Ƃ݂Ȃ��l
static const float epsilon = FLT_EPSILON * 16;

/**
* �Փ˔������]������
*/
void Box::RotateY(float radians)
{
  const glm::mat3 matRot =
    glm::rotate(glm::mat4(1), radians, glm::vec3(0, 1, 0));
  const glm::vec3 a = matRot * min;
  const glm::vec3 b = matRot * max;
  min = glm::min(a, b);
  max = glm::max(a, b);
}

/**
* �����̂ƒ����̂̏Փ�
*/
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
* ���Ƌ��̏Փ�
*/
bool CollisionSphereSphere(Actor& actorA, Actor& actorB, Contact& contact)
{
  // ���[���h���W�n�̏Փː}�`���v�Z����
  Sphere a = static_cast<Sphere&>(*actorA.collider);
  a.center += actorA.position;

  Sphere b = static_cast<Sphere&>(*actorB.collider);
  b.center += actorB.position;

  // ���S�Ԃ̋��������a�̍��v���傫����΁A�Փ˂��Ă��Ȃ�
  const glm::vec3 v = b.center - a.center;
  const float d2 = glm::dot(v, v);
  const float r = a.radius + b.radius;
  if (d2 > r * r) {
    return false;
  }
  
  // �ǂ��炩�A�܂��͗����̃A�N�^�[���u�u���b�N���Ȃ��v�ꍇ�A�ڐG�������s��Ȃ�
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

  const float distance = std::sqrt(d2);
  const glm::vec3 normal = v * (1.0f / distance);
  const float penetration = (a.radius + b.radius) - distance;

  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;
  contact.penetration = normal * penetration;
  contact.normal = normal;

  // �����������W���Փ˓_�Ƃ���
  const glm::vec3 rv = actorA.velocity - actorB.velocity;
  const float nv = glm::dot(rv, normal);
  contact.position = actorA.position + normal * (a.radius + nv);

  // �Փ˂��Ă���
  return true;
}

/**
* �����̂̒��_���W���擾����
*/
glm::vec3 Corner(const Box& box, int flag)
{
  glm::vec3 c = box.min;
  if (flag & 1) {
    c.x = box.max.x;
  }
  if (flag & 2) {
    c.y = box.max.y;
  }
  if (flag & 4) {
    c.z = box.max.z;
  }
  return c;
}

/**
* ���ƒ����̂̏Փ�
*/
bool CollisionBoxSphere(Actor& actorA, Actor& actorB, Contact& contact)
{
  // ���[���h���W�n�̏Փː}�`���v�Z����
  Box a = static_cast<Box&>(*actorA.collider);
  a.min += actorA.position;
  a.max += actorA.position;

  Sphere b = static_cast<Sphere&>(*actorB.collider);
  b.center += actorB.position;

  // ���̒��S�ɍł��߂������̓��̍��W(�ŋߐړ_)�����߂�
  const glm::vec3 closestPoint = glm::clamp(b.center, a.min, a.max);

  // ���̒��S����ŋߐړ_�܂ł̋������v�Z����
  const glm::vec3 v = closestPoint - b.center;
  const float d2 = glm::dot(v, v);

  // �ŋߐړ_�܂ł̋��������̔��a��蒷����΁A�Փ˂��Ă��Ȃ�
  if (d2 > b.radius * b.radius) {
    return false;
  }

  // �ǂ��炩�A�܂��͗����̃A�N�^�[���u�u���b�N���Ȃ��v�ꍇ�A�ڐG�������s��Ȃ�
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

  // �ŋߐړ_�����_�A�ӁA���̑��̂ǂ��ɂ��邩�ɂ���āA�Փ˖ʂ̌v�Z�𕪂���
  int flagMin = 0;
  int flagMax = 0;
  for (int i = 0; i < 3; ++i) {
    if (closestPoint[i] <= a.min[i]) {
      flagMin |= 1 << i;
    }
    if (closestPoint[i] >= a.max[i]) {
      flagMax |= 1 << i;
    }
  }
  const int flag = flagMin | flagMax;

  glm::vec3 normal;
  if (flag) {
    // �ŋߐړ_�������̂̕\�ʂɂ���ꍇ�A�ŋߐړ_�Ƌ������Ԓ�����@���Ƃ���
    normal = closestPoint - b.center;
    if (glm::dot(normal, normal) < epsilon) {
      // �ŋߐړ_�Ƌ��̒��S���߂�����Ɩ@�����v�Z�ł��Ȃ�
      // �����ɍŋߐړ_���܂܂��ʂ��狁�߂�
      for (int i = 0; i < 3; ++i) {
        if (flagMin & (1 << i)) {
          normal[i] = 1;
        } else if (flagMax & (1 << i)) {
          normal[i] = -1;
        }
      }
    }
    normal = glm::normalize(normal);
  }
  else {
    // �ŋߐړ_�������̂̓����ɂ���ꍇ�A�x���V�e�B����Փ˂̉\��������ʂ̂����A�ł��߂��ʂŏՓ˂����Ƃ݂Ȃ�
    const glm::vec3 rv = actorA.velocity - actorB.velocity;
    const bool noVelocity = glm::dot(rv, rv) < epsilon;
    float dmin = FLT_MAX;
    int face = 0; // �ł��߂���
    for (int i = 0; i < 3; ++i) {
      if (rv[i] < 0 || noVelocity) {
        float d = closestPoint[i] - a.min[i];
        if (d < dmin) {
          dmin = d;
          face = i;
        }
      } else if (rv[i] > 0 || noVelocity) {
        float d = a.max[i] - closestPoint[i];
        if (d < dmin) {
          dmin = d;
          face = i + 3;
        }
      }
    }
    normal = glm::vec3(0);
    if (face < 3) {
      normal[face] = 1;
    } else {
      normal[face - 3] = -1;
    }
  }

  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;

  const float distance = glm::distance(closestPoint, b.center);
  contact.penetration = normal * (distance - b.radius);

  contact.normal = normal;
  contact.position = closestPoint;

  // �Փ˂��Ă���
  return true;
}

bool CollisionSphereBox(Actor& actorA, Actor& actorB, Contact& contact)
{
  return CollisionBoxSphere(actorB, actorA, contact);
}

/**
* ���Ɖ~���̏Փ�
*/
bool CollisionSphereCylinder(Actor& actorA, Actor& actorB, Contact& contact)
{
  return false;
}

bool CollisionCylinderSphere(Actor& actorA, Actor& actorB, Contact& contact)
{
  return CollisionSphereCylinder(actorB, actorA, contact);
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
  if (d2 > r * r) {
    return false;
  }

  // �ǂ��炩�A�܂��͗����̃A�N�^�[���u�u���b�N���Ȃ��v�ꍇ�A�ڐG�������s��Ȃ�
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

  // Y�����̐Z�������ƕ������v�Z����
  glm::vec3 normal(0);
  glm::vec3 penetration(0);
  if (dy0 < dy1) {
    penetration.y = -dy0;
    normal.y = 1;
  } else {
    penetration.y = dy1;
    normal.y = -1;
  }

  // XZ�����̐Z�������ƕ������v�Z����
  float lengthXZ;
  if (d2 >= epsilon) {
    const float d = std::sqrt(d2);
    const float invD = 1.0f / d;
    normal.x = -dx * invD;
    normal.z = -dz * invD;
    lengthXZ = r - d;
  } else {
    // XZ���W���d�Ȃ��Ă���ꍇ�A�@�����v�Z�ł��Ȃ��̂ő��x�ő�p����
    lengthXZ = r;
    normal.x = actorA.velocity.x - actorB.velocity.x;
    normal.z = actorA.velocity.z - actorB.velocity.z;
    if (normal.x || normal.z) {
      const float invD = 1.0f / std::sqrt(normal.x * normal.x + normal.z * normal.z);
      normal.x *= invD;
      normal.z *= invD;
    } else {
      // �x���V�e�B��0�̏ꍇ�͕������m��ł��Ȃ��B�Ƃ肠����+X�����Ƃ���
      normal.x = 1;
    }
  }
  penetration.x = -lengthXZ * normal.x;
  penetration.z = -lengthXZ * normal.z;

  // �Z�������̒������������O����
  if (std::abs(penetration.y) <= lengthXZ) {
    penetration.x = penetration.z = 0;
    normal.x = normal.z = 0;
  } else {
    penetration.y = 0;
    normal.y = 0;
  }

  // �Փˏ���ݒ�
  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;
  contact.penetration = penetration;
  contact.normal = normal;
  contact.penLength = glm::length(penetration);

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
    contact.position = targetCollider->bottom;
    if (normal.y) {
      // Y�����̏Փ˂̏ꍇ�E�E�E
      if (targetNormal.y >= 0) {
        contact.position.y += targetCollider->height;
      }
    } else {
      // XZ�����̏Փ˂̏ꍇ�E�E�E
      contact.position.x -= targetNormal.x * targetCollider->radius;
      contact.position.y += targetCollider->height * 0.5f;
      contact.position.z -= targetNormal.z * targetCollider->radius;
    }
  }

  // �Փ˂��Ă���
  return true;
}

/**
* �����̂Ɖ~���̏Փ�
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

  // �~���̒��S�ɍł��߂������̓���XZ���W(�ŋߐړ_)�����߂�
  const float cx = glm::clamp(b.bottom.x, a.min.x, a.max.x);
  const float cz = glm::clamp(b.bottom.z, a.min.z, a.max.z);
  const glm::vec3 closestPointXZ(cx, 0, cz);

  // �~���̒��S����ŋߐړ_�܂ł̋������v�Z����
  const float dx = closestPointXZ.x - b.bottom.x;
  const float dz = closestPointXZ.z - b.bottom.z;
  const float d2 = dx * dx + dz * dz;

  // �ŋߐړ_�܂ł̋������~���̔��a��蒷����΁A�Փ˂��Ă��Ȃ�
  if (d2 > b.radius * b.radius) {
    return false;
  }

  // �ǂ��炩�A�܂��͗����̃A�N�^�[���u�u���b�N���Ȃ��v�ꍇ�A�ڐG�������s��Ȃ�
  if (!actorA.isBlock || !actorB.isBlock) {
    return true;
  }

  // Y�����̐Z�������ƕ������v�Z����
  glm::vec3 penetration(0);
  glm::vec3 normal(0);
  if (dy0 < dy1) {
    penetration.y = -dy0;
    normal.y = 1;
  } else {
    penetration.y = dy1;
    normal.y = -1;
  }

  // XZ�����̍ŋߐړ_�̈ʒu�ɂ���āA�Փ˖ʂ̌v�Z�𕪂���
  int flagMin = 0;
  int flagMax = 0;
  for (int i = 0; i < 3; i += 2) {
    if (closestPointXZ[i] <= a.min[i]) {
      flagMin |= (1 << i);
    }
    if (closestPointXZ[i] >= a.max[i]) {
      flagMax |= (1 << i);
    }
  }
  const int flag = flagMin | flagMax;

  if (flag) {
    // XZ�ŋߐړ_�������̂̕\�ʂɂ���ꍇ�AXZ�ŋߐړ_�Ɖ~�����Ԓ�����@���Ƃ���
    if (d2 >= epsilon) {
      normal.x = dx;
      normal.z = dz;
    } else {
      // �ŋߐړ_�Ɖ~���̒��S���߂�����Ɩ@�����v�Z�ł��Ȃ�
      // �����ɍŋߐړ_���܂܂��ʂ��狁�߂�
      for (int i = 0; i < 3; i += 2) {
        if (flagMin & (1 << i)) {
          normal[i] = 1;
        } else if (flagMax & (1 << i)) {
          normal[i] = -1;
        }
      }
    }
    // XZ�����̖@���𐳋K��
    const float invD = 1.0f / std::sqrt(normal.x * normal.x + normal.z * normal.z);
    normal.x *= invD;
    normal.z *= invD;
  } else {
    // XZ�ŋߐړ_�������̂̓����ɂ���ꍇ�A
    // �x���V�e�B����Փ˂̉\��������Ɣ��f�����ʂ̂����A�ł��߂��ʂŏՓ˂����Ƃ݂Ȃ�
    // �x���V�e�B��0�̏ꍇ�A���ׂĂ̖ʂɏՓ˂̉\��������Ɣ��f����
    const glm::vec3 rv = actorA.velocity - actorB.velocity; // ���΃x���V�e�B���v�Z
    const bool noVelocity = glm::dot(rv, rv) < epsilon;
    float dmin = FLT_MAX;
    int nearestFace = 0; // �ł��߂���
    for (int i = 0; i < 3; i += 2) {
      if (rv[i] < 0 || noVelocity) {
        float d = closestPointXZ[i] - a.min[i];
        if (d < dmin) {
          dmin = d;
          nearestFace = i;
        }
      }
      if (rv[i] > 0 || noVelocity) {
        float d = a.max[i] - closestPointXZ[i];
        if (d < dmin) {
          dmin = d;
          nearestFace = i + 3;
        }
      }
    }
    // �ł��߂��ʂ̖@����ݒ肷��
    if (nearestFace < 3) {
      normal[nearestFace] = 1;
    } else {
      normal[nearestFace - 3] = -1;
    }
  }

  // XZ�����̐Z���������v�Z
  float distance = b.radius;
  if (d2 >= epsilon) {
    distance -= std::sqrt(d2);
  }
  penetration.x = -normal.x * distance;
  penetration.z = -normal.z * distance;

  // �Z���������������������O����
  // ���ʂ̏Փ˂�����ꍇ�AXZ�x�N�g���̒�����Y���r����
  const glm::vec3 absPenetration = glm::abs(penetration);
  if (flag && absPenetration.y > distance) {
    penetration.y = 0;
    normal.y = 0;
  } else {
    // ���ʏՓˈȊO�̏ꍇ�A�ł��Z���������Z�����������c���A���͏��O����
    float pmin = FLT_MAX;
    int axisMin = 0;
    for (int i = 0; i < 3; ++i) {
      if (absPenetration[i] > 0 && absPenetration[i] < pmin) {
        pmin = absPenetration[i];
        axisMin = i;
      }
    }
    for (int i = 0; i < 3; ++i) {
      if (i != axisMin) {
        penetration[i] = 0;
        normal[i] = 0;
      }
    }
  }

  // �Փˏ���ݒ�
  contact.a = &actorA;
  contact.b = &actorB;
  contact.velocityA = actorA.velocity;
  contact.velocityB = actorB.velocity;
  contact.accelA = actorA.velocity - actorA.oldVelocity;
  contact.accelB = actorB.velocity - actorB.oldVelocity;
  contact.penetration = penetration;
  contact.normal = normal;
  contact.penLength = glm::length(penetration);

  // �Փ˖ʂ̍��W���v�Z����
  {
    // ��{�I�ɃA�N�^�[B�̍��W���g�����A�A�N�^�[B���Õ��̏ꍇ�̓A�N�^�[A�̍��W���g��
    const glm::vec3 center = (a.min + a.max) * 0.5f;
    const glm::vec3 size = (a.max - a.min) * 0.5f;
    if (actorB.isStatic) {
      contact.position = center;
      if (normal.y) {
        // Y�����̏Փ˂̏ꍇ�E�E�E
        contact.position.y -= size.y * normal.y;
      } else {
        // XZ�����̏Փ˂̏ꍇ�E�E�E
        contact.position.x -= size.x * normal.x;
        contact.position.z -= size.z * normal.z;
      }
    } else {
      contact.position = b.bottom;
      if (normal.y) {
        // Y�����̏Փ˂̏ꍇ�E�E�E
        contact.position.y += b.height * (0.5f + 0.5f * normal.y);
      } else {
        // XZ�����̏Փ˂̏ꍇ�E�E�E
        contact.position.x -= normal.x * b.radius;
        contact.position.z -= normal.z * b.radius;
      }
    }
  }

  // �Փ˂��Ă���
  return true;
}

/**
* �~���ƒ����̂̏Փ�
*/
bool CollisionCylinderBox(Actor& actorA, Actor& actorB, Contact& contact)
{
  return CollisionBoxCylinder(actorB, actorA, contact);
}

