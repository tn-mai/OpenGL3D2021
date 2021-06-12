/**
* @file Actor.h
*/
#ifndef ACTOR_H_INCLUDED
#define ACTOR_H_INCLUDED
#include <glad/glad.h>
#include "Primitive.h"
#include "Texture.h"
#include "ProgramPipeline.h"
#include <glm/glm.hpp>

/**
* ���̂𐧌䂷��p�����[�^.
*/
struct Actor
{
  Primitive prim;                  // �`�悷��v���~�e�B�u
  std::shared_ptr<Texture> tex;    // �`��Ɏg���e�N�X�`��
  glm::vec3 position;              // ���̂̈ʒu
  glm::vec3 scale;                 // ���̂̊g��k����
  float rotation;                  // ���̂̉�]�p�x
  glm::vec3 adjustment;            // ���̂����_�Ɉړ����邽�߂̋���
};

void Draw(
  const Actor& actor,              // ���̂̐���p�����[�^
  const ProgramPipeline& pipeline, // �`��Ɏg���v���O�����p�C�v���C��
  glm::mat4 matProj,               // �`��Ɏg���v���W�F�N�V�����s��
  glm::mat4 matView);              // �`��Ɏg���r���[�s��  

#endif // ACTOR_H_INCLUDED
