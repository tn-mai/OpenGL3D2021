/**
* @file Camera.cpp
*/
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* �r���[�s����擾����
*/
const glm::mat4& Camera::GetViewMatrix() const
{
  return matView;
}

/**
* �v���W�F�N�V�����s����擾����
*/
const glm::mat4& Camera::GetProjectionMatrix() const
{
  return matProj;
}

/**
* �J�����̏�Ԃ��X�V����
*/
void Camera::Update()
{
  // �r���[�s����쐬.
  matView = glm::lookAt(position, target, up);

  // �v���W�F�N�V�����s����쐬.
  matProj = glm::perspective(fovy, aspectRatio, zNear, zFar);
}

