/**
* @file HumanActor.cpp
*/
#include "HumanActor.h"
#include "../GameEngine.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* �R���X�g���N�^
*/
HumanActor::HumanActor(
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation)
  : Actor(
    "Human",
    GameEngine::Get().LoadMesh("Res/human.obj"),
    position, scale, rotation, glm::vec3(0))
{
  collider = Box::Create(
    glm::vec3(-0.4f, 0, -0.4f), glm::vec3(0.4f, 1.8f, 0.4f));
  mass = 80;

  // ���b�V�����擾
  MeshRenderer& meshRenderer = static_cast<MeshRenderer&>(*renderer);
  MeshPtr mesh = meshRenderer.GetMesh();
  if (mesh) {
    // �e�q�֌W��ݒ�
    Mesh::Group& legR = mesh->groups[groupLegR];
    Mesh::Group& legL = mesh->groups[groupLegL];
    legR.parent = groupBody;
    legL.parent = groupBody;

    // �t�o�C���h�|�[�Y�s���ݒ�
    legR.matInverseBindPose =
      glm::translate(glm::mat4(1), glm::vec3(0.125f, -0.675f, 0.0f));
    legL.matInverseBindPose =
      glm::translate(glm::mat4(1), glm::vec3(-0.125f, -0.675f, 0.0f));

    // �o�C���h�|�[�Y�s���ݒ�
    legR.matBindPose = glm::inverse(legR.matInverseBindPose);
    legL.matBindPose = glm::inverse(legL.matInverseBindPose);

    // �E���̃A�j���[�V������ݒ�
    AnimationCurvePtr curveLegR = AnimationCurve::Create();
    curveLegR->SetKey(KeyFrame{ 0.00f, glm::radians(0.0f) });
    curveLegR->SetKey(KeyFrame{ 0.25f, glm::radians(45.0f) });
    curveLegR->SetKey(KeyFrame{ 0.75f, glm::radians(-45.0f) });
    curveLegR->SetKey(KeyFrame{ 1.00f, glm::radians(0.0f) });
    AnimationClipPtr clip = AnimationClip::Create();
    clip->SetCurve(
      [](Actor& a, float v) {
        // �A�j���[�V�����̒lv��X����]�p�x�Ƃ݂Ȃ��ĉ�]�s����쐬
        const glm::mat4 matRot = glm::rotate(glm::mat4(1), v, glm::vec3(1, 0, 0));
        // ��]�s����E���̃O���[�v�s��ɐݒ�
        MeshRenderer& meshRenderer = static_cast<MeshRenderer&>(*a.renderer);
        meshRenderer.SetGroupMatrix(groupLegR, matRot);
      },
      curveLegR);

    // �����̃A�j���[�V������ݒ�
    AnimationCurvePtr curveLegL = AnimationCurve::Create();
    curveLegL->SetKey(KeyFrame{ 0.00f, glm::radians(0.0f) });
    curveLegL->SetKey(KeyFrame{ 0.25f, glm::radians(-45.0f) });
    curveLegL->SetKey(KeyFrame{ 0.75f, glm::radians(45.0f) });
    curveLegL->SetKey(KeyFrame{ 1.00f, glm::radians(0.0f) });
    clip->SetCurve(
      [](Actor& a, float v) {
        const glm::mat4 matRot = glm::rotate(glm::mat4(1), v, glm::vec3(1, 0, 0));
        MeshRenderer& meshRenderer = static_cast<MeshRenderer&>(*a.renderer);
        meshRenderer.SetGroupMatrix(groupLegL, matRot);
      },
      curveLegL);

    // �A�j���[�V���������[�v�Đ�
    AnimationPtr animation = Animation::Create();
    animation->SetClip(clip);
    animation->SetWrapMode(Animation::WrapMode::loop);
    animation->Play();
    SetAnimation(animation);
  }
}

/**
* �A�N�^�[�̏�Ԃ��X�V����
*
* @param deltaTime �O��̍X�V����̌o�ߎ���(�b)
*/
void HumanActor::OnUpdate(float deltaTime)
{
  Actor::OnUpdate(deltaTime);
}

