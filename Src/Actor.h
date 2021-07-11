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

/**
* ������.
*/
struct Box
{
  glm::vec3 min = glm::vec3(0);
  glm::vec3 max = glm::vec3(0);
};

/**
* ���̂𐧌䂷��p�����[�^.
*/
struct Actor
{
  std::string name;                // �A�N�^�[�̖��O
  Primitive prim;                  // �`�悷��v���~�e�B�u
  std::shared_ptr<Texture> tex;    // �`��Ɏg���e�N�X�`��
  glm::vec3 position;              // ���̂̈ʒu
  glm::vec3 scale;                 // ���̂̊g��k����
  float rotation;                  // ���̂̉�]�p�x
  glm::vec3 adjustment;            // ���̂����_�Ɉړ����邽�߂̋���

  glm::vec3 velocity = glm::vec3(0);// ���x(���[�g�����b)
  glm::vec3 oldVelocity = glm::vec3(0); // �ȑO�̑��x(���[�g�����b)
  float lifespan = 0;              // ����(�b�A0�ȉ��Ȃ�����Ȃ�)
  float health = 10;               // �ϋv�l
  bool isDead = false;             // false=���S(�폜�҂�) true=������

  Box collider;                    // �Փ˔���
  float mass = 1;                  // ����(kg)
  float cor = 0.4f;                // �����W��
  float friction = 0.7f;           // ���C�W��
  bool isStatic = false;           // false=�������镨�� true=�������Ȃ����� 
  bool isBlock = true;             // false=�ʉ߂ł��� true=�ʉ߂ł��Ȃ�

  bool isOnActor = false;
};

void Draw(
  const Actor& actor,              // ���̂̐���p�����[�^
  const ProgramPipeline& pipeline, // �`��Ɏg���v���O�����p�C�v���C��
  glm::mat4 matProj,               // �`��Ɏg���v���W�F�N�V�����s��
  glm::mat4 matView);              // �`��Ɏg���r���[�s��  

Actor* Find(std::vector<Actor>& actors, const char* name);

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
};

bool DetectCollision(Actor& a, Actor& b, Contact& pContact);
void SolveContact(Contact& contact);
bool Equal(const Contact& ca, const Contact& cb);

#endif // ACTOR_H_INCLUDED
