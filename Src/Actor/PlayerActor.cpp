/**
* @file PlayerActor.cpp
*/
#include "PlayerActor.h"
#include "../GameEngine.h"
#include "../Audio.h"
#ifdef USE_EASY_AUDIO
#include "../EasyAudioSettings.h"
#else
#include "../Audio/MainWorkUnit/SE.h"
#endif // USE_EASY_AUDIO
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
    GameEngine::Get().LoadMesh("Res/tank/Tiger_I.obj"),
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

#if 0 // ターレット(と砲身)だけを回転させる実験
  static float rotTurret = 0;
  MeshPtr mesh = GetMesh();
  if (mesh) {
    for (int i = 0; i < mesh->groups.size(); ++i) {
      if (mesh->groups[i].name == "Turret_Object_1.002" ||
        mesh->groups[i].name == "Gun_Object_1.003") {
        this->SetMatrix(i, glm::rotate(glm::mat4(1), rotTurret, glm::vec3(0, 1, 0)));
      }
    }
    rotTurret += glm::radians(30.0f) * deltaTime;
    rotTurret = fmod(rotTurret, glm::radians(360.0f));
  }
#endif

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

#ifdef USE_EASY_AUDIO
  if (playTankTruck) {
    Audio::Play(1, SE_TANK_MOVE, 0.1f, true);
  } else {
    Audio::Stop(1);
  }
#else
  if (playTankTruck) {
    Audio::Get().Play(2, CRI_SE_TANK_MOVE, 0.1f);
  } else {
    Audio::Get().Stop(2);
  }
#endif

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

#ifdef USE_EASY_AUDIO
    Audio::PlayOneShot(SE_PLAYER_SHOT);
#else
    Audio::Get().Play(1, CRI_SE_SHOT);
#endif // USE_EASY_AUDIO
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
#ifdef USE_EASY_AUDIO
      Audio::PlayOneShot(SE_EXPLOSION);
#else
      Audio::Get().Play(1, CRI_SE_EXPLOSION);
#endif // USE_EASY_AUDIO
      isDead = true;
    }
    contact.b->isDead = true;
  }
}

