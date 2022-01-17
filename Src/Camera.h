/**
* @file Camera.h
*/
#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED
#include <glm/glm.hpp>

/**
* �J�����f�[�^
*/
class Camera
{
public:
  Camera() = default;
  ~Camera() = default;

  void Update();
  const glm::mat4& GetViewMatrix() const;
  const glm::mat4& GetProjectionMatrix() const;
  const glm::mat4& GetShaderParameter() const;

  glm::vec3 position = glm::vec3(0, 0, 0);
  glm::vec3 up = glm::vec3(0, 1, 0);
  glm::vec3 target = glm::vec3(0, 0, -1);
  float aspectRatio = 1.0f;
  float fovy = glm::radians(45.0f);
  float zNear = 1.0f; // 21b��0.1f����ύX.
  float zFar = 400.0f; // 21b��200.0f����ύX.

  // true:  focusDistance�𖳎�����target�܂ł̋������g��
  // false: fucusDistance�ɐݒ肵���������g��
  bool isAutofocus = true;
  float focusDistance = 10; // �����Y����s���g�̍����ʒu�܂ł̋���(m�P��)
  float fNumber = 1.4f;     // F�l(����������ƃ{�P�������Ȃ�)
  float sensorWidth = 36;   // �����󂯂�Z���T�[�̉���(mm�P��)
  glm::vec2 screenSize = glm::vec2(1280, 720); // ��ʂ̃s�N�Z����

private:
  glm::mat4 matView = glm::mat4(1);
  glm::mat4 matProj = glm::mat4(1);
  glm::mat4 shaderParameter = glm::mat4(0);
};

#endif // CAMERA_H_INCLUDED

