/**
* @file PlayerActor.cpp
*/
#include "PlayerActor.h"
#include "../GameEngine.h"
#include "../Audio.h"
#include "../Audio/MainWorkUnit/SE.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* コンストラクタ
*/
PlayerActor::PlayerActor(
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation) :
  Actor(
    "Tiger-I",
    GameEngine::Get().GetPrimitive("Res/tank/Tiger_I.obj"),
    GameEngine::Get().LoadTexture("Res/tank/PzVl_Tiger_I.tga"),
    position, scale, rotation, glm::vec3(0))
{
  health = 10;
  //collider = Box::Create(glm::vec3(-1.8f, 0, -1.8f), glm::vec3(1.8f, 2.8f, 1.8f));
  collider = Cylinder::Create(glm::vec3(0), 1.8f, 2.5f);
  mass = 57'000;
  //cor = 0.1f;
  //friction = 1.0f;
}

/**
* アクターの状態を更新する
*
* @param deltaTime 前回の更新からの経過時間(秒)
*/
void PlayerActor::OnUpdate(float deltaTime)
{
  GameEngine& engine = GameEngine::Get();

  bool playTankTruck = false;

  if (isOnActor) {
    if (engine.GetKey(GLFW_KEY_A)) {
      rotation += glm::radians(90.0f) * deltaTime;
      playTankTruck = true;
    } else if (engine.GetKey(GLFW_KEY_D)) {
      rotation -= glm::radians(90.0f) * deltaTime;
      playTankTruck = true;
    }
  }

  // rotationが0のときの戦車の向きベクトル
  glm::vec3 tankFront(0, 0, 1);
  // rotationラジアンだけ回転させる回転行列を作る
  const glm::mat4 matRot = glm::rotate(glm::mat4(1), rotation, glm::vec3(0, 1, 0));
  // 向きベクトルをtank.rotationだけ回転させる
  tankFront = matRot * glm::vec4(tankFront, 1);

  if (isOnActor) {
    float speed2 = glm::dot(velocity, velocity);
    //if (speed2 < 10.0f * 10.0f) {
      float tankAccel = 0.2f; // 戦車の加速度
      if (engine.GetKey(GLFW_KEY_W)) {
        velocity += tankFront * tankAccel;
        playTankTruck = true;
      } else if (engine.GetKey(GLFW_KEY_S)) {
        velocity -= tankFront * tankAccel;
        playTankTruck = true;
      } else {
        float v = glm::dot(tankFront, velocity);
        velocity -= tankFront * glm::clamp(v, -0.1f, 0.1f);
      }
      //glm::vec3 tankRight = glm::normalize(glm::cross(tankFront, glm::vec3(0, 1, 0)));
      //float rightSpeed = glm::dot(tankRight, velocity);
      //velocity -= tankRight * glm::clamp(rightSpeed, -0.2f, 0.2f);
    //}
      if (engine.GetKey(GLFW_KEY_SPACE)) {
        velocity.y = 12;
      }
  }

  if (playTankTruck) {
    Audio::Get().Play(2, CRI_SE_TANK_MOVE, 0.1f);
  } else {
    Audio::Get().Stop(2);
  }

  // マウス左ボタンの状態を取得する
  int shotButton = engine.GetMouseButton(GLFW_MOUSE_BUTTON_LEFT);

  // マウス左ボタンが押された瞬間に弾アクターを発射する
  static int shotInterval = 5;
  bool isShot = false;
  if (shotButton != 0) {
    if (oldShotButton == 0 || --shotInterval <= 0) {
      isShot = true;
      shotInterval = 5;
    }
  }
  if (isShot) {
    // 発射位置を砲の先端に設定
    glm::vec3 position = this->position + tankFront * 6.0f;
    position.y += 2.0f;

    std::shared_ptr<Actor> bullet(new Actor{
      "Bullet",
      engine.GetPrimitive("Res/Bullet.obj"),
      engine.LoadTexture("Res/Bullet.tga"),
      position, glm::vec3(0.25f), rotation, glm::vec3(0) });

    // 1.5秒後に弾を消す
    bullet->lifespan = 1.5f;

    // 戦車の向いている方向に、30m/sの速度で移動させる
    bullet->velocity = tankFront * 30.0f;

    // 弾に衝突判定を付ける
    //bullet->collider = Box::Create(glm::vec3(-0.25f), glm::vec3(0.25f));
    //bullet->collider = Sphere::Create(glm::vec3(0), 1.0f);
    bullet->collider = Cylinder::Create(glm::vec3(0, -0.3f, 0), 0.3f, 0.6f);
    bullet->mass = 6.8f;
    bullet->friction = 1.0f;

    engine.AddActor(bullet);

    Audio::Get().Play(1, CRI_SE_SHOT);
  }

  // 「前回のショットボタンの状態」を更新する
  oldShotButton = shotButton;
}

/**
* 衝突を処理する
*
* @param contact 衝突情報
*/
void PlayerActor::OnCollision(const struct Contact& contact)
{
  if (contact.b->name == "EnemyBullet") {
    --health;
    if (health <= 0) {
      Audio::Get().Play(1, CRI_SE_EXPLOSION);
      isDead = true;
    }
    contact.b->isDead = true;
  }
}

