/**
* @file Camera.h
*/
#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED
#include <glm/glm.hpp>

/**
* カメラデータ
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
  float zNear = 1.0f; // 21bで0.1fから変更.
  float zFar = 400.0f; // 21bで200.0fから変更.

  // true:  focusDistanceを無視してtargetまでの距離を使う
  // false: fucusDistanceに設定した距離を使う
  bool isAutofocus = true;
  float focusDistance = 10; // レンズからピントの合う位置までの距離(m単位)
  float fNumber = 1.4f;     // F値(小さくするとボケが強くなる)
  float sensorWidth = 36;   // 光を受けるセンサーの横幅(mm単位)
  glm::vec2 screenSize = glm::vec2(1280, 720); // 画面のピクセル数

private:
  glm::mat4 matView = glm::mat4(1);
  glm::mat4 matProj = glm::mat4(1);
  glm::mat4 shaderParameter = glm::mat4(0);
};

#endif // CAMERA_H_INCLUDED

