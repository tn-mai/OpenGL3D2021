/**
* @file Actor.h
*/
#ifndef ACTOR_H_INCLUDED
#define ACTOR_H_INCLUDED
#include <glad/glad.h>
#include "Primitive.h"
#include "Texture.h"
#include "ProgramPipeline.h"
#include "Collision.h"
#include "Renderer.h"
#include "Animation.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include <functional>

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

/**
* 表示レイヤー
*/
enum class Layer
{
  Default,
  Sprite,
  UI,
};
static const size_t layerCount = 3; // レイヤー数

/**
* シェーダの種類
*/
enum class Shader
{
  FragmentLighting,
  InstancedMesh,
  GroundMap,
};
static size_t shaderCount = 3; // シェーダの種類数

/**
* 衝突判定の種類
*
* 20bで実装. 20は未実装.
*/
enum class CollisionType
{
  block,       // 衝突判定を行う(通過させない)
  trigger,     // 交差していることを通知するだけ
  noCollision, // 衝突判定を行わない
};

/**
* アクターの種類を識別するタグ
*
* [重要]
* メンバ名の変更・追加・削除、または順番の入れ替えを行った場合、
* ActorTagToString関数の定義も修正すること。
*/
enum class ActorTag
{
  other,     // その他
  player,    // プレイヤーが操作するアクター
  friendly,  // 味方
  enemy,     // 敵
  boss,      // ボス敵
  item,      // アイテム
  destructionTarget, // 破壊対象
};
static const size_t actorTagCount = 7; // タグ数

const char* ActorTagToString(ActorTag tag);
ActorTag StringToActorTag(const char* str);

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

  Actor(
    const std::string& name,
    const MeshPtr& mesh,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment);

  Actor(
    const std::string& name,
    bool isStatic = true,
    const glm::vec3& position = glm::vec3(0),
    const glm::vec3& scale = glm::vec3(1),
    float rotation = 0,
    const glm::vec3& adjustment = glm::vec3(0));

  virtual ~Actor() = default;
  virtual std::shared_ptr<Actor> Clone() const {
    std::shared_ptr<Actor> clone(new Actor(*this));
    if (collider) {
      clone->collider = collider->Clone();
    }
    if (renderer) {
      clone->renderer = renderer->Clone();
    }
    return clone;
  }
  virtual void OnUpdate(float deltaTime);
  virtual void OnCollision(const struct Contact& contact);
  virtual void OnTrigger(std::shared_ptr<Actor> other) {}
  void SetAnimation(AnimationPtr a);
  glm::mat4 GetModelMatrix() const;

  std::string name;                // アクターの名前
  RendererPtr renderer;            // 描画オブジェクト
  glm::vec3 position;              // 物体の位置
  glm::vec3 scale;                 // 物体の拡大縮小率
  float rotation;                  // 物体の回転角度
  glm::vec3 adjustment;            // 物体を原点に移動するための距離

  ActorTag tag = ActorTag::other;

  // 19で追加. 19bは未追加.
  glm::vec4 color = glm::vec4(1);  // テクスチャに合成する色
  float gravityScale = 1.0f;       // 重力の影響度

  glm::vec3 velocity = glm::vec3(0);// 速度(メートル毎秒)
  float lifespan = 0;              // 寿命(秒、0以下なら寿命なし)
  float health = 10;               // 耐久値
  bool isDead = false;             // false=死亡(削除待ち) true=生存中

  CollisionType collisionType = CollisionType::block;
  std::shared_ptr<Collider> collider; // 衝突判定
  //float contactCount = 1;         // 接触点の数
  float mass = 1;                  // 質量(kg)
  float cor = 0.4f;                // 反発係数
  float friction = 0.7f;           // 摩擦係数
  bool isStatic = false;           // false=動かせる物体 true=動かせない物体 

  Layer layer = Layer::Default;    // 表示レイヤー
  Shader shader = Shader::FragmentLighting;

  // [マップエディタ専用] インスタンスグループの管理
  static const int noInstanceGroup = -1; // グループなし
  static const int maxInstanceGroup = 15;// 最大グループ番号
  int instanceGroup = noInstanceGroup;

  // アニメーションを設定するときはSetAnimationを使うこと
  AnimationPtr animation;          // アニメーション

  glm::vec3 oldVelocity = glm::vec3(0); // 以前の速度(メートル毎秒)
  bool isOnActor = false;
};

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
void SolveContact(Contact& contact);
bool Equal(const Contact& ca, const Contact& cb);
bool Equal2(const Contact& ca, const Contact& cb);

#endif // ACTOR_H_INCLUDED
