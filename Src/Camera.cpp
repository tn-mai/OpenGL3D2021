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
* シェーダーに送るパラメータを取得する
*/
const glm::mat4& Camera::GetShaderParameter() const
{
  return shaderParameter;
}

/**
* カメラの状態を更新する
*/
void Camera::Update()
{
  // ビュー行列を作成
  matView = glm::lookAt(position, target, up);

  // プロジェクション行列を作成
  matProj = glm::perspective(fovy, aspectRatio, zNear, zFar);

  // ピントが合う距離を計算
  float distance = focusDistance;
  if (isAutofocus) {
    distance = glm::length(target - position);
  }
  distance *= 1000.0f; // mm(ミリメートル)単位に変換

  // シェーダーに送る画面情報を設定
  // [0][0] 1ピクセルの幅(1.0 / スクリーンの幅)
  // [0][1] 1ピクセルの高さ(1.0 / スクリーンの高さ)
  // [0][2] 近平面(m単位) 
  // [0][3] 遠平面(m単位)
  shaderParameter[0] = glm::vec4(glm::vec2(1) / screenSize, zNear, zFar);

  // FOVを再現できるイメージセンサーの位置を計算(mm単位)
  const float imageDistance = (sensorWidth * 0.5f) / glm::tan(fovy * aspectRatio * 0.5f);

  // 焦点距離を計算
  // 通常、センサーまでの距離に対して焦平面までの距離が十分に長い
  // そのため、焦点距離はimageDistanceよりほんのわずかしか短くならない
  // 例えばimageDistance=50, distance=4000の場合、focalLengthは約49.4である
  // このため、focalLength = imageDistanceとしても実用上の問題はほぼないと思われる
  const float focalLength = 1.0f / ((1.0f / distance) + (1.0f / imageDistance));

  // [1][0] レンズからピントの合う位置までの距離(mm単位)
  // [1][1] 焦点距離(レンズから光が1点に集まる位置までの距離. mm単位)
  // [1][2] 絞り(mm単位)
  // [1][3] 光を受けるセンサーの横幅(mm単位)
  shaderParameter[1] = glm::vec4(distance, focalLength, focalLength / fNumber, sensorWidth);

  // [2][0] カメラのワールド座標X
  // [2][1] カメラのワールド座標Y
  // [2][2] カメラのワールド座標Z
  shaderParameter[2] = glm::vec4(position, 0);
}

