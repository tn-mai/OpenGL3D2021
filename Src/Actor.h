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
* 物体を制御するパラメータ.
*/
struct Actor
{
  Primitive prim;                  // 描画するプリミティブ
  std::shared_ptr<Texture> tex;    // 描画に使うテクスチャ
  glm::vec3 position;              // 物体の位置
  glm::vec3 scale;                 // 物体の拡大縮小率
  float rotation;                  // 物体の回転角度
  glm::vec3 adjustment;            // 物体を原点に移動するための距離
};

void Draw(
  const Actor& actor,              // 物体の制御パラメータ
  const ProgramPipeline& pipeline, // 描画に使うプログラムパイプライン
  glm::mat4 matProj,               // 描画に使うプロジェクション行列
  glm::mat4 matView);              // 描画に使うビュー行列  

#endif // ACTOR_H_INCLUDED
