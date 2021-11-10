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
  static const CollisionFunc funcArray[3][3] = {
    //              box, sphere, cylinder
    /* box      */ { CollisionBoxBox, CollisionBoxSphere, CollisionBoxCylinder },
    /* sphere   */ { CollisionSphereBox, CollisionSphereSphere, DummyCollisionFunc },
    /* cylinder */ { CollisionCylinderBox, DummyCollisionFunc, CollisionCylinderCylinder },
  };
  const int y = static_cast<int>(actorA.collider->GetShapeType());
  const int x = static_cast<int>(actorB.collider->GetShapeType());
  return funcArray[y][x](actorA, actorB, contact);
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
    //const float ratio = 1.0f / actorB.contactCount;
    const float ratio = 1.0f;
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
  }
  else if (actorB.isStatic) {
    //const float ratio = 1.0f / actorA.contactCount;
    const float ratio = 1.0f;
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
    const float ratioA = 1.0f;// / actorA.contactCount;
    const float ratioB = 1.0f;// / actorB.contactCount;
    float massA = actorA.mass * ratioA;
    float massB = actorB.mass * ratioB;
    float massAB = massA + massB;
    float c = massA * ua + massB * ub;
    float va = (c + cor * massB * (ub - ua)) / massAB;
    float vb = (c + cor * massA * (ua - ub)) / massAB;

    // �ՓˑO�̑��x��0�ɂ���
    actorA.velocity -= normal * ua;
    actorB.velocity -= normal * ub;

    // �Փˌ�̑��x�����Z����
    actorA.velocity += normal * va;
    actorB.velocity += normal * vb;

    // ���C�ɂ�鑬�x�����Z����
    actorA.velocity += frictionVelocity * ratioA;
    actorB.velocity -= frictionVelocity * ratioB;

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
    actorB.position += pb * ratioB;

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
  if (!ca.a || !ca.b || !cb.a || !cb.b) {
    return false;
  }

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

