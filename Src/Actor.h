/**
* @file Actor.h
*/
#ifndef ACTOR_H_INCLUDED
#define ACTOR_H_INCLUDED
#include <glad/glad.h>
#include "Primitive.h"
#include "Texture.h"
#include "ProgramPipeline.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

/**
* 直方体.
*/
struct Box
{
  glm::vec3 min = glm::vec3(0);
  glm::vec3 max = glm::vec3(0);
};

/**
* 物体を制御するパラメータ.
*/
struct Actor
{
  std::string name;                // アクターの名前
  Primitive prim;                  // 描画するプリミティブ
  std::shared_ptr<Texture> tex;    // 描画に使うテクスチャ
  glm::vec3 position;              // 物体の位置
  glm::vec3 scale;                 // 物体の拡大縮小率
  float rotation;                  // 物体の回転角度
  glm::vec3 adjustment;            // 物体を原点に移動するための距離

  glm::vec3 velocity = glm::vec3(0);// 速度(メートル毎秒)
  glm::vec3 oldVelocity = glm::vec3(0); // 以前の速度(メートル毎秒)
  float lifespan = 0;              // 寿命(秒、0以下なら寿命なし)
  float health = 10;               // 耐久値
  bool isDead = false;             // false=死亡(削除待ち) true=生存中

  Box collider;                    // 衝突判定
  float mass = 1;                  // 質量(kg)
  float cor = 0.4f;                // 反発係数
  float friction = 0.7f;           // 摩擦係数
  bool isStatic = false;           // false=動かせる物体 true=動かせない物体 
  bool isBlock = true;             // false=通過できる true=通過できない

  bool isOnActor = false;
};

void Draw(
  const Actor& actor,              // 物体の制御パラメータ
  const ProgramPipeline& pipeline, // 描画に使うプログラムパイプライン
  glm::mat4 matProj,               // 描画に使うプロジェクション行列
  glm::mat4 matView);              // 描画に使うビュー行列  

Actor* Find(std::vector<Actor>& actors, const char* name);

/**
* 衝突情報
*/
struct Contact
{
  Actor* a = nullptr;
  Actor* b = nullptr;
  glm::vec3 velocityA;   // 衝突時点でのアクターAのベロシティ
  glm::vec3 velocityB;   // 衝突時点でのアクターBのベロシティ
  glm::vec3 accelA;      // 衝突時点でのアクターAの加速度
  glm::vec3 accelB;      // 衝突時点でのアクターBの加速度
  glm::vec3 penetration; // 浸透距離
  glm::vec3 normal;      // 衝突面の法線
  glm::vec3 position;    // 衝突面の座標
  float penLength;       // 浸透距離の長さ
};

bool DetectCollision(Actor& a, Actor& b, Contact& pContact);
void SolveContact(Contact& contact);
bool Equal(const Contact& ca, const Contact& cb);

#endif // ACTOR_H_INCLUDED
