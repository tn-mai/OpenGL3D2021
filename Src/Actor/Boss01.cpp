/**
* @file Boss01.cpp
*/
#include "Boss01.h"
#include "BulletActor.h"
#include "../GameEngine.h"
#include "../GameManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>

/**
* コンストラクタ
*/
Boss01::Boss01(const glm::vec3& position, const glm::vec3& scale,
  float rotation, const std::shared_ptr<Actor>& target) :
  Actor("Boss01",
    GameEngine::Get().GetPrimitive("Res/Black_Track.obj"),
    GameEngine::Get().LoadTexture("Res/Black_Track.tga"),
    position, scale, rotation, glm::vec3(0)),
  target(target)
{
  health = 50;
  mass = 200'000;
  collider = Box::Create(glm::vec3(-3, 0, -3), glm::vec3(3, 2.5f, 3));
}

/**
* アクターの状態を更新する
*
* @param deltaTime 前回の更新からの経過時間(秒)
*/
void Boss01::OnUpdate(float deltaTime)
{
  (this->*mode)(deltaTime);
}

/**
* 衝突を処理する
*
* @param contact 衝突情報
*/
void Boss01::OnCollision(const struct Contact& contact)
{
  if (contact.b->name == "Bullet") {
    health -= 1;
    if (health <= 0) {
      isDead = true; // T-34戦車を消去する
      GameManager::Get().AddScore(2000);
    }
    contact.b->isDead = true; // 弾を消去する
  }
}

/**
* 待機モードの更新
*/
void Boss01::Idle(float deltaTime)
{
  // 攻撃対象がいなければ待機モードを継続
  if (!target) {
    return;
  }

  // 攻撃対象が一定距離に近づいたら弾幕モードに切り替える
  const float maxViewLength = 40.0f; // 攻撃対象を発見できる距離(m)
  const glm::vec3 d = target->position - position;
  if (glm::dot(d, d) < maxViewLength * maxViewLength) {
    // 弾幕モードの準備
    modeTimer = 10.0f;
    shotTimer = 0.2f;
    shotDirection = 0;
    mode = &Boss01::Danmaku;
  }
}

/**
* 通常弾を発射する
*
* @param engine ゲームエンジン
* @param position 弾の発射位置
* @param velocity 弾の速度
* @param lifespan 弾の寿命(秒)
*/
static void NormalShot(GameEngine& engine,
  const glm::vec3& position,
  const glm::vec3& v,
  float lifespan = 1.5f)
{
  std::shared_ptr<Actor> bullet(new BulletActor{ "EnemyBullet",
    engine.GetPrimitive("Res/Bullet.obj"),
    engine.LoadTexture("Res/Bullet.tga"),
    position, glm::vec3(0.25f), 0, glm::vec3(0) });
  bullet->lifespan = lifespan;
  bullet->velocity = v;
  bullet->mass = 6;
  bullet->friction = 0.0f;
  bullet->collider = Box::Create(glm::vec3(-0.1f), glm::vec3(0.1f));

  engine.AddActor(bullet);
}

/**
* 通常弾を発射する
*
* @param engine ゲームエンジン
* @param position 弾の発射位置
* @param direction 弾の発射方向(度)
*/
static void NormalShot(GameEngine& engine,
  const glm::vec3& position,
  float direction)
{
  // 発射方向のベクトルを計算
  const float radians = glm::radians(direction);
  const glm::mat4 matR = glm::rotate(glm::mat4(1), radians, glm::vec3(0, 1, 0));
  const glm::vec3 v = matR * glm::vec4(0, 0, 1, 1);

  const float speed = 20.0f;
  NormalShot(engine, position + v, v * speed);
}

/**
* 弾幕モードの更新
*/
void Boss01::Danmaku(float deltaTime)
{
  GameEngine& engine = GameEngine::Get();

  // 一定時間ごとに弾を発射
  shotTimer -= deltaTime;
  if (shotTimer <= 0) {
    for (float i = 0; i < 360; i += 60) {
      NormalShot(engine, position + glm::vec3(0, 2.8f, 0), shotDirection + i);
      NormalShot(engine, position + glm::vec3(0, 3.0f, 0), 360 - shotDirection + i + 30);
    }
    shotTimer = 0.2f;
    shotDirection = std::fmod(shotDirection + 5.0f, 360.0f);
  }

  // 一定時間が経過したらマシンガンモードに切り替える
  modeTimer -= deltaTime;
  if (modeTimer <= 0) {
    // マシンガンモードの準備
    // いきなり切り替わらないように、最初の発射タイマーは少し間隔を開ける
    shotTimer = 3.0f;
    ammo = 25; // マシンガンモードで発射する弾の総数
    mode = &Boss01::Machinegun;
  }
}

/**
* マシンガンモードの更新
*/
void Boss01::Machinegun(float deltaTime)
{
  // 弾が残っているなら、一定時間ごとに弾を発射
  if (target && ammo > 0) {
    shotTimer -= deltaTime;
    if (shotTimer <= 0) {
      const glm::vec3 v = glm::normalize(target->position - position);
      const float speed = 20.0f;
      NormalShot(GameEngine::Get(), position + glm::vec3(0, 3, 0), v * speed);

      --ammo;

      // 残弾が5で割り切れるときは3秒待機、それ以外は0.1秒待機
      if (ammo % 5) {
        shotTimer += 0.1f;
      } else {
        shotTimer += 3.0f;
      }
    }
  }

  // 弾がなくなったらミサイルモードに切り替える
  if (ammo <= 0) {
    // ミサイルモードの準備
    ammo = 4; // ミサイルの弾数
    shotTimer = 3.0f;
    mode = &Boss01::Missile;
  }
}

/**
* ミサイルモードの更新
*/
void Boss01::Missile(float deltaTime)
{
  // ミサイルが残っているなら、一定時間ごとにミサイルを発射
  if (target && ammo > 0) {
    shotTimer -= deltaTime;
    if (shotTimer <= 0) {
      const float flightTime = 4.0f;
      const float gravity = 9.8f;
      glm::vec3 v = target->position - position;
      v *= 1.0f / flightTime;
      // 鉛直投げ上げの公式からY方向の速度を計算
      v.y = 0.5f * gravity * flightTime;
      NormalShot(GameEngine::Get(), position + glm::vec3(0, 3, 0), v, flightTime);
      --ammo;
      shotTimer = 4.0f;
    }
  }

  // ミサイルの残り弾数が0になったら弾幕モードに切り替える
  if (ammo <= 0) {
    // 弾幕モードの準備
    modeTimer = 10.0f;
    shotTimer = 0.2f;
    shotDirection = 0;
    mode = &Boss01::Danmaku;
  }
}

