/**
* @file Actor.h
*/
#ifndef ACTOR_H_INCLUDED
#define ACTOR_H_INCLUDED
#include "glad/glad.h"
#include "Global.h"
#include "Texture.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>

/**
* 衝突判定.
*/
struct Collision 
{
  // 円柱のパラメータ.
  float top;    // 円柱の上端.
  float bottom; // 円柱の下端.
  float radius; // 円柱の半径.
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

  std::string name; // アクターの名前.

  const Mesh::Primitive* primitive = nullptr;
  std::shared_ptr<Texture::Image2D> texture;

  glm::vec3 position = glm::vec3(0); // アクターの表示位置.
  glm::vec3 rotation = glm::vec3(0); // アクターの向き.
  glm::vec3 scale = glm::vec3(1); // アクターの大きさ.
  glm::vec3 velocity = glm::vec3(0); // アクターの移動速度.

  // アニメーション用データ.
  std::vector<const Mesh::Primitive*> animation; // アニメーションに使うプリミティブのリスト.
  size_t animationNo = 0; // 表示するプリミティブの番号.
  float animationTimer = 0; // プリミティブ切り替えタイマー(秒).
  float animationInterval = 0.3f; // プリミティブを切り替える間隔(秒).

  // 衝突判定用の変数.
  Collision collision;

  bool hasShadow = true;
};

// アクターの配列.
using ActorPtr = std::shared_ptr<Actor>;
using ActorList = std::vector<ActorPtr>;

void UpdateActorList(ActorList& actorList, float deltaTime);
void RenderActorList(const ActorList& actorList,
  const glm::mat4& matVP, const glm::mat4& matShadow);

bool DetectCollision(const Actor&, const Actor&);
void HandleCollisions(ActorList&);

#endif // ACTOR_H_INCLUDED
