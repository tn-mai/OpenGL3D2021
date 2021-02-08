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
  bool blockOtherActors = true; // 通り抜けられないならtrue、抜けられるならfalse.

  // 円柱のパラメータ.
  float top;    // 円柱の上端.
  float bottom; // 円柱の下端.
  float radius; // 円柱の半径.

  // 直方体のパラメータ.
  glm::vec3 boxMin;
  glm::vec3 boxMax;
};

/**
* 軸並行境界ボックス.
*/
struct AABB
{
  glm::vec3 c = glm::vec3(0); // 中心座標.
  glm::vec3 r = glm::vec3(0); // 各軸の半径.
};

/**
* アクター.
*/
class Actor
{
public:
  Actor() = default;
  virtual ~Actor() = default;
  Actor(std::string actorName, const Mesh::Primitive* prim,
    std::shared_ptr<Texture::Image2D> tex, const glm::vec3& pos);

  void Update(float deltTIme);
  virtual void OnUpdate(float) {}
  virtual void OnDestroy() {}

  // 描画の種類.
  enum DrawType {
    color,  // 通常描画.
    shadow, // 影描画.
  };
  void Draw(const Shader::Pipeline& pipeline, const glm::mat4& matVP, DrawType) const;

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
    dead, // 死んでいる.
    blow_off, // 吹き飛んでいる.
  };
  State state = State::idle; // 現在の動作状態.
  float health = 0; // 耐久力.
  float lifespan = 0; // 寿命(秒). 0以下は無限扱い.
  float timer = 0;  // 汎用タイマー.

  const Mesh::Primitive* primitive = nullptr;
  const Mesh::Primitive* morphTarget = nullptr;
  const Mesh::Primitive* prevBaseMesh = nullptr;
  const Mesh::Primitive* prevMorphTarget = nullptr;
  float prevMorphWeight = 0;
  float morphTransitionTimer = 0;
  std::shared_ptr<Texture::Image2D> texture;
  std::shared_ptr<Texture::Image2D> texNormal;
  std::shared_ptr<Texture::Image2D> texMetallicSmoothness;
  std::shared_ptr<Texture::Sampler> samplers[3];

  glm::vec3 position = glm::vec3(0); // アクターの表示位置.
  glm::vec3 rotation = glm::vec3(0); // アクターの向き.
  glm::vec3 scale = glm::vec3(1); // アクターの大きさ.
  glm::vec3 velocity = glm::vec3(0); // アクターの移動速度.
  glm::vec4 baseColor = glm::vec4(1); // アクターの色.

  // アニメーション用データ.
  std::shared_ptr<Animation> animation; // アニメーションデータ.
  size_t animationNo = 0; // 表示するプリミティブの番号.
  float animationTimer = 0; // プリミティブ切り替えタイマー(秒).

  // 衝突判定用の変数.
  Collision collision;
  AABB boundingBox;

  // 衝突解決関数へのポインタ.
  void (*OnHit)(Actor&, Actor&) = [](Actor&, Actor&) {};

  std::shared_ptr<Actor> attackActor; // 攻撃の衝突判定用アクター.

  float gravityScale = 0; // 重力影響率.
  float friction = 0.7f;  // 摩擦係数.
  float drag = 0;         // 発生した摩擦力.

  bool isDead = false; // 死亡フラグ.
  bool isShadowCaster = true;
};

using ActorPtr = std::shared_ptr<Actor>; // アクターポインタ型.
using ActorList = std::vector<ActorPtr>; // アクター配列型.

void UpdateActorList(ActorList& actorList, float deltaTime);
void RenderActorList(const ActorList& actorList,
  const glm::mat4& matVP, Actor::DrawType drawType);

bool DetectCollision(Actor&, Actor&, bool block);

/**
* 線分.
*/
struct Segment
{
  glm::vec3 start; // 線分の始点.
  glm::vec3 end;   // 線分の終点.
};

/**
* 球.
*/
struct Sphere
{
  glm::vec3 center; // 球の中心座標.
  float radius;     // 球の半径.
};

/**
* 円錐.
*/
struct Cone
{
  glm::vec3 tip; // 円錐の頂点座標.
  float height;  // 円錐の高さ.
  glm::vec3 direction; // 円錐の向き.
  float radius;  // 円錐の底面の半径.
};

/**
* 平面.
*/
struct Plane
{
  glm::vec3 point;  // 平面上の任意の座標.
  glm::vec3 normal; // 平面の法線.
};

bool Intersect(const Segment& seg, const Plane& plane, glm::vec3* p);
bool SphereInsidePlane(const Sphere& sphere, const Plane& plane);
bool ConeInsidePlane(const Cone& cone, const Plane& plane);

#endif // ACTOR_H_INCLUDED
