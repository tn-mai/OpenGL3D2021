/**
* @file Actor.h
*/
#ifndef ACTOR_H_INCLUDED
#define ACTOR_H_INCLUDED
#include "glad/glad.h"
#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>

/**
* アニメーションデータ.
*/
struct Animation
{
  std::vector<const Mesh::Primitive*> list; // アニメーションに使うプリミティブのリスト.
  float interval = 0.3f; // プリミティブを切り替える間隔(秒).
  bool isLoop = true; // ループフラグ.
};

/**
* 衝突判定.
*/
struct Collision
{
  // 衝突形状の種類.
  enum Shape {
    none,     // 衝突判定なし.
    cylinder, // 円柱.
    box,      // 直方体.
  };
  Shape shape = Shape::none;
  bool isBlock = true; // 通り抜けられないならtrue、抜けられるならfalse.

  // 円柱のパラメータ.
  float top;    // 円柱の上端.
  float bottom; // 円柱の下端.
  float radius; // 円柱の半径.

  // 直方体のパラメータ.
  glm::vec3 boxMin;
  glm::vec3 boxMax;
};

/**
* アクター.
*/
class Actor
{
public:
  Actor() = default;
  ~Actor() = default;
  Actor(std::string actorName, const Mesh::Primitive* prim,
    std::shared_ptr<Texture::Image2D> tex, const glm::vec3& pos);

  void Update(float deltTIme);
  void Draw(const Shader::Pipeline& pipeline, const glm::mat4& matVP, const glm::mat4& matShadow) const;
  void SetCylinderCollision(float top, float bottom, float radius);
  void SetBoxCollision(const glm::vec3& min, const glm::vec3& max);
  void SetAnimation(
    std::shared_ptr<Animation> animation);

  std::string name; // アクターの名前.

  // アクターの動作状態.
  enum State {
    idle, // 何もしていない(待機中).
    run,  // 走っている.
    search, // 索敵している.
    attack, // 攻撃している.
    damage, // ダメージを受けている.
    down, // 倒れている.
  };
  State state = State::idle; // 現在の動作状態.


  const Mesh::Primitive* primitive = nullptr;
  std::shared_ptr<Texture::Image2D> texture;

  glm::vec3 position = glm::vec3(0); // アクターの表示位置.
  glm::vec3 rotation = glm::vec3(0); // アクターの向き.
  glm::vec3 scale = glm::vec3(1); // アクターの大きさ.
  glm::vec3 velocity = glm::vec3(0); // アクターの移動速度.

  // アニメーション用データ.
  std::shared_ptr<Animation> animation; // アニメーションデータ.
  size_t animationNo = 0; // 表示するプリミティブの番号.
  float animationTimer = 0; // プリミティブ切り替えタイマー(秒).

  // 衝突判定用の変数.
  Collision collision;
  void (*OnHit)(Actor&, Actor&) = [](Actor&, Actor&) {};

  bool isDead = false;
  bool hasShadow = true;
};

using ActorPtr = std::shared_ptr<Actor>; // アクターポインタ型.
using ActorList = std::vector<ActorPtr>; // アクター配列型.

void UpdateActorList(ActorList& actorList, float deltaTime);
void RenderActorList(const ActorList& actorList,
  const glm::mat4& matVP, const glm::mat4& matShadow);

bool DetectCollision(Actor&, Actor&);

#endif // ACTOR_H_INCLUDED
