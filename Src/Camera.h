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

  glm::vec3 position = glm::vec3(0, 0, 0);
  glm::vec3 up = glm::vec3(0, 1, 0);
  glm::vec3 target = glm::vec3(0, 0, -1);
  float aspectRatio = 1.0f;
  float fovy = glm::radians(45.0f);
  float zNear = 0.1f;
  float zFar = 200.0f;

private:
  glm::mat4 matView = glm::mat4(1);
  glm::mat4 matProj = glm::mat4(1);
};

#endif // CAMERA_H_INCLUDED

