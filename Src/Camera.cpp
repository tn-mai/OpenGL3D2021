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
* �V�F�[�_�[�ɑ���p�����[�^���擾����
*/
const glm::mat4& Camera::GetShaderParameter() const
{
  return shaderParameter;
}

/**
* �J�����̏�Ԃ��X�V����
*/
void Camera::Update()
{
  // �r���[�s����쐬
  matView = glm::lookAt(position, target, up);

  // �v���W�F�N�V�����s����쐬
  matProj = glm::perspective(fovy, aspectRatio, zNear, zFar);

  // �s���g�������������v�Z
  float distance = focusDistance;
  if (isAutofocus) {
    distance = glm::length(target - position);
  }
  distance *= 1000.0f; // mm(�~�����[�g��)�P�ʂɕϊ�

  // �V�F�[�_�[�ɑ����ʏ���ݒ�
  // [0][0] 1�s�N�Z���̕�(1.0 / �X�N���[���̕�)
  // [0][1] 1�s�N�Z���̍���(1.0 / �X�N���[���̍���)
  // [0][2] �ߕ���(m�P��) 
  // [0][3] ������(m�P��)
  shaderParameter[0] = glm::vec4(glm::vec2(1) / screenSize, zNear, zFar);

  // FOV���Č��ł���C���[�W�Z���T�[�̈ʒu���v�Z(mm�P��)
  const float imageDistance = (sensorWidth * 0.5f) / glm::tan(fovy * aspectRatio * 0.5f);

  // �œ_�������v�Z
  // �ʏ�A�Z���T�[�܂ł̋����ɑ΂��ďŕ��ʂ܂ł̋������\���ɒ���
  // ���̂��߁A�œ_������imageDistance���ق�̂킸�������Z���Ȃ�Ȃ�
  // �Ⴆ��imageDistance=50, distance=4000�̏ꍇ�AfocalLength�͖�49.4�ł���
  // ���̂��߁AfocalLength = imageDistance�Ƃ��Ă����p��̖��͂قڂȂ��Ǝv����
  const float focalLength = 1.0f / ((1.0f / distance) + (1.0f / imageDistance));

  // [1][0] �����Y����s���g�̍����ʒu�܂ł̋���(mm�P��)
  // [1][1] �œ_����(�����Y�������1�_�ɏW�܂�ʒu�܂ł̋���. mm�P��)
  // [1][2] �i��(mm�P��)
  // [1][3] �����󂯂�Z���T�[�̉���(mm�P��)
  shaderParameter[1] = glm::vec4(distance, focalLength, focalLength / fNumber, sensorWidth);

  // [2][0] �J�����̃��[���h���WX
  // [2][1] �J�����̃��[���h���WY
  // [2][2] �J�����̃��[���h���WZ
  shaderParameter[2] = glm::vec4(position, 0);
}

