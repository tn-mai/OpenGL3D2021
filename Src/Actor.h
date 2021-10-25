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
#include <memory>

/**
* 第11回テキスト実装方針:
* エンジンクラスへの統合はコストが高いため見送る.
* 1. actors 配列を shared_ptr 配列化する.
* 2. OnHit仮想関数を実装.
* 3. EnemyActorクラスを作成し、OnHitをオーバーライド.
* 4. BulletActorクラスを作成し、OnHitをオーバーライド.
* 5. Tick仮想関数を実装.
* 7. ElevetorActorクラスを作成し、Tickをオーバーライド.
* 8. EnemyActorクラスのTickをオーバーライド.
* 9. PlayerActorクラスを作成し、Tickをオーバーライド.
*/

// 図形の種類
enum class ShapeType
{
  box,      // 直方体
  cylinder, // 円柱
};

struct Collider
{
public:
  explicit Collider(ShapeType type) : shapeType(type) {}
  virtual ~Collider() = default;

  ShapeType shapeType;
};

/**
* 直方体
*/
struct Box : public Collider
{
  Box() : Collider(ShapeType::box) {};
  Box(const glm::vec3& min, const glm::vec3& max) :
    Collider(ShapeType::box), min(min), max(max)
  {}
  virtual ~Box() = default;

  glm::vec3 min = glm::vec3(0);
  glm::vec3 max = glm::vec3(0);
};

inline std::shared_ptr<Box> CreateBoxShape(
  const glm::vec3& min, const glm::vec3& max)
{
  return std::make_shared<Box>(min, max);
}

/**
* 円柱
*/
struct Cylinder : public Collider
{
  Cylinder(const glm::vec3& bottom, float radius, float height) :
    Collider(ShapeType::cylinder), bottom(bottom), radius(radius), height(height)
  {}
  virtual ~Cylinder() = default;

  glm::vec3 bottom = glm::vec3(0); // 下端の座標
  float radius = 0.5f; // 半径
  float height = 1.0f; // 高さ
};

inline std::shared_ptr<Cylinder> CreateCylinderShape(
  const glm::vec3& bottom, float radius, float height)
{
  return std::make_shared<Cylinder>(bottom, radius, height);
}

/**
* 表示レイヤー
*/
enum class Layer
{
  Default,
  UI,
};
static const size_t layerCount = 2; // レイヤー数

/**
* シェーダの種類
*/
enum class Shader
{
  FragmentLighting,
  Ground,
};

/**
* 物体を制御するパラメータ.
*/
class Actor
{
public:
  Actor(
    const std::string& name,
    const Primitive& prim,
    std::shared_ptr<Texture> tex,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment);

  virtual ~Actor() = default;
  virtual std::shared_ptr<Actor> Clone() const {
    return std::shared_ptr<Actor>(new Actor(*this));
  }
  virtual void OnUpdate(float deltaTime);
  virtual void OnCollision(const struct Contact& contact);

  std::string name;                // アクターの名前
  Primitive prim;                  // 描画するプリミティブ
  std::shared_ptr<Texture> tex;    // 描画に使うテクスチャ
  glm::vec3 position;              // 物体の位置
  glm::vec3 scale;                 // 物体の拡大縮小率
  float rotation;                  // 物体の回転角度
  glm::vec3 adjustment;            // 物体を原点に移動するための距離

  glm::vec3 velocity = glm::vec3(0);// 速度(メートル毎秒)
  float lifespan = 0;              // 寿命(秒、0以下なら寿命なし)
  float health = 10;               // 耐久値
  bool isDead = false;             // false=死亡(削除待ち) true=生存中

  std::shared_ptr<Collider> collider; // 衝突判定
  float contactCount = 1;         // 接触点の数
  float mass = 1;                  // 質量(kg)
  float cor = 0.4f;                // 反発係数
  float friction = 0.7f;           // 摩擦係数
  bool isStatic = false;           // false=動かせる物体 true=動かせない物体 

  Layer layer = Layer::Default;    // 表示レイヤー
  Shader shader = Shader::FragmentLighting;

  // TODO: テキスト未追加
  glm::vec3 oldVelocity = glm::vec3(0); // 以前の速度(メートル毎秒)
  bool isBlock = true;             // false=通過できる true=通過できない
  float gravityScale = 1.0f;       // 重力係数
  glm::vec4 color = glm::vec4(1);
  bool isOnActor = false;
};

void Draw(
  const Actor& actor,              // 物体の制御パラメータ
  const ProgramPipeline& pipeline, // 描画に使うプログラムパイプライン
  glm::mat4 matProj,               // 描画に使うプロジェクション行列
  glm::mat4 matView);              // 描画に使うビュー行列  

std::shared_ptr<Actor> Find(std::vector<std::shared_ptr<Actor>>& actors, const char* name);

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

  float massB;
};

Contact Reverse(const Contact& contact);

bool DetectCollision(Actor& a, Actor& b, Contact& contact);
bool CollisionBoxBox(Actor& a, Actor& b, Contact& contact);
bool CollisionCylinderCylinder(Actor& a, Actor& b, Contact& contact);
bool CollisionBoxCylinder(Actor& a, Actor& b, Contact& contact);
bool CollisionCylinderBox(Actor& a, Actor& b, Contact& contact);
void SolveContact(Contact& contact);
bool Equal(const Contact& ca, const Contact& cb);
bool Equal2(const Contact& ca, const Contact& cb);

#endif // ACTOR_H_INCLUDED
