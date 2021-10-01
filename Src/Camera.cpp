/**
* @file Camera.cpp
*/
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* ビュー行列を取得する
*/
const glm::mat4& Camera::GetViewMatrix() const
{
  return matView;
}

/**
* プロジェクション行列を取得する
*/
const glm::mat4& Camera::GetProjectionMatrix() const
{
  return matProj;
}

/**
* カメラの状態を更新する
*/
void Camera::Update()
{
  // ビュー行列を作成.
  matView = glm::lookAt(position, target, up);

  // プロジェクション行列を作成.
  matProj = glm::perspective(fovy, aspectRatio, zNear, zFar);
}

