/**
* @file HumanActor.cpp
*/
#include "HumanActor.h"
#include "../GameEngine.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* コンストラクタ
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

  // メッシュを取得
  MeshRenderer& meshRenderer = static_cast<MeshRenderer&>(*renderer);
  MeshPtr mesh = meshRenderer.GetMesh();
  if (mesh) {
    // 親子関係を設定
    Mesh::Group& legR = mesh->groups[groupLegR];
    Mesh::Group& legL = mesh->groups[groupLegL];
    legR.parent = groupBody;
    legL.parent = groupBody;

    // 逆バインドポーズ行列を設定
    legR.matInverseBindPose =
      glm::translate(glm::mat4(1), glm::vec3(0.125f, -0.675f, 0.0f));
    legL.matInverseBindPose =
      glm::translate(glm::mat4(1), glm::vec3(-0.125f, -0.675f, 0.0f));

    // バインドポーズ行列を設定
    legR.matBindPose = glm::inverse(legR.matInverseBindPose);
    legL.matBindPose = glm::inverse(legL.matInverseBindPose);

    // 右足のアニメーションを設定
    AnimationCurvePtr curveLegR = AnimationCurve::Create();
    curveLegR->SetKey(KeyFrame{ 0.00f, glm::radians(0.0f) });
    curveLegR->SetKey(KeyFrame{ 0.25f, glm::radians(45.0f) });
    curveLegR->SetKey(KeyFrame{ 0.75f, glm::radians(-45.0f) });
    curveLegR->SetKey(KeyFrame{ 1.00f, glm::radians(0.0f) });
    AnimationClipPtr clip = AnimationClip::Create();
    clip->SetCurve(
      [](Actor& a, float v) {
        // アニメーションの値vをX軸回転角度とみなして回転行列を作成
        const glm::mat4 matRot = glm::rotate(glm::mat4(1), v, glm::vec3(1, 0, 0));
        // 回転行列を右足のグループ行列に設定
        MeshRenderer& meshRenderer = static_cast<MeshRenderer&>(*a.renderer);
        meshRenderer.SetGroupMatrix(groupLegR, matRot);
      },
      curveLegR);

    // 左足のアニメーションを設定
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

    // アニメーションをループ再生
    AnimationPtr animation = Animation::Create();
    animation->SetClip(clip);
    animation->SetWrapMode(Animation::WrapMode::loop);
    animation->Play();
    SetAnimation(animation);
  }
}

/**
* アクターの状態を更新する
*
* @param deltaTime 前回の更新からの経過時間(秒)
*/
void HumanActor::OnUpdate(float deltaTime)
{
  Actor::OnUpdate(deltaTime);
}

