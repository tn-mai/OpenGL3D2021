/**
* @file T34TankActor.cpp
*/
#include "T34TankActor.h"
#include "BulletActor.h"
#include "ExplosionActor.h"
#include "../GameEngine.h"
#include "../GameManager.h"
#include "../Audio.h"
#ifdef USE_EASY_AUDIO
#include "../EasyAudioSettings.h"
#else
#include "../Audio/MainWorkUnit/SE.h"
#endif // USE_EASY_AUDIO
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

/**
* コンストラクタ
*/
T34TankActor::T34TankActor(
  const std::string& name,
  const Primitive& prim,
  std::shared_ptr<Texture> tex,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment,
  const std::shared_ptr<Actor>& target) :
  Actor(name, prim, tex, position, scale, rotation, adjustment), // 基底クラスを初期化
  target(target)
{
}

/**
* アクターの状態を更新する
*
* @param deltaTime 前回の更新からの経過時間(秒)
*/
void T34TankActor::OnUpdate(float deltaTime)
{
  // 追跡対象アクターが設定されている場合の処理
  if (isOnActor && target) {
    // T-34戦車の正面方向のベクトルを計算
    glm::mat4 matR = glm::rotate(glm::mat4(1), rotation, glm::vec3(0, 1, 0));
    glm::vec3 t34Front = matR * glm::vec4(0, 0, 1, 1);

    // T-34戦車からタイガーI戦車へのベクトルdを計算
    glm::vec3 d = target->position - position;

    // T-34戦車からタイガーI戦車への距離を計算
    float length = glm::length(d);

    // プレイヤーが視認範囲外にいるときは何もしない
    const float maxViewLength = 40.0f; // プレイヤーを発見できる距離(m)
    if (length > maxViewLength) {
      return;
    }

    // ベクトルdを正規化
    d = glm::normalize(d);

    // T-34戦車の正面ベクトルと、タイガーI戦車へのベクトルの内積を計算
    float r = std::acos(glm::dot(t34Front, d));

    // T-34戦車の正面とタイガーI戦車のいる方向の角度が10度未満の場合...
    if (r < glm::radians(10.0f)) {
      // タイガーI戦車までの距離が10mより遠い場合は前に加速
      if (length > 10.0f) {
        // 進行方向への速度が最高速度以下なら加速
        const float maxSpeed = (40.0f/* km/h */ * 1000.0f)/* m/h */ / 3600.0f/* m/s */;
        if (glm::dot(velocity, t34Front) < maxSpeed) {
          velocity += t34Front * 0.3f;
        }
      } else {
        // ベロシティのt34Front方向の長さを計算
        float v = glm::dot(t34Front, velocity);
        // 長さが0.2以上なら0.2を減速、それ以下なら長さ分を減速する
        velocity -= t34Front * glm::clamp(v, -0.2f, 0.2f);
      }
    }
    // 角度が10度以上の場合...
    else {
      // T-34戦車の正面ベクトルと、タイガーI戦車へのベクトルの外積を計算
      glm::vec3 n = glm::cross(t34Front, d);
      // yが0以上なら反時計回り、0未満なら時計回りに回転するほうが近い
      if (n.y >= 0) {
        rotation += glm::radians(90.0f) * deltaTime;
      } else {
        rotation -= glm::radians(90.0f) * deltaTime;
      }
    }

    // 弾を発射
    shotTimer -= deltaTime;
    if (shotTimer <= 0) {
      shotTimer = 3;

#ifdef USE_EASY_AUDIO
      Audio::PlayOneShot(SE_ENEMY_SHOT);
#else
      Audio::Get().Play(1, CRI_SE_SHOT);
#endif // USE_EASY_AUDIO

      // 発射位置を砲の先端に設定
      glm::vec3 position = this->position + t34Front * 2.0f;
      position.y += 2.0f;

      GameEngine& engine = GameEngine::Get();
      std::shared_ptr<Actor> bullet(new BulletActor{
        "EnemyBullet", engine.GetPrimitive("Res/Bullet.obj"), engine.LoadTexture("Res/Bullet.tga"),
        position, glm::vec3(0.25f), rotation, glm::vec3(0) });

      // 1.5秒後に弾を消す
      bullet->lifespan = 1.5f;

      // 戦車の向いている方向に、30m/sの速度で移動させる
      bullet->velocity = t34Front * 20.0f;

      // 弾に衝突判定を付ける
      bullet->collider = Box::Create(glm::vec3(-0.25f), glm::vec3(0.25f));
      bullet->mass = 6.8f;
      bullet->friction = 1.0f;

      engine.AddActor(bullet);
    }
  }
}

/**
* 衝突を処理する
*
* @param contact 衝突情報
*/
void T34TankActor::OnCollision(const struct Contact& contact)
{
  if (contact.b->name == "Bullet") {
#ifdef USE_EASY_AUDIO
    Audio::PlayOneShot(SE_HIT);
#else
    Audio::Get().Play(1, CRI_SE_HIT);
#endif // USE_EASY_AUDIO

    // T-34戦車の耐久値を減らす
    health -= 1;
    if (health <= 0) {
      isDead = true; // T-34戦車を消去する
      GameManager::Get().AddScore(200);

      GameEngine& engine = GameEngine::Get();
#if 0
      engine.AddActor(std::make_shared<ExplosionActor>(position, 4.0f));
#else
      const auto tex0 = engine.LoadTexture("Res/Sprite/Explosion.tga");
      std::shared_ptr<Sprite> sprExprosion =
        std::make_shared<Sprite>(position + glm::vec3(0, 1, 0), tex0);
      sprExprosion->lifespan = 0.5f;

//      AnimationPtr animation = std::make_shared<Animation>();
//      animation->AddClip<glm::vec3>(
//          &sprExprosion->scale, 0, 0.5f, glm::vec3(2), glm::vec3(8));
//      animation->AddClip<float>(
//          &sprExprosion->rotation, 0, 0.5f, 0.0f, 1.5f);
//      animation->AddClip<float>(
//          &sprExprosion->color.a, 0, 0.5f, 8, 0);
//      animation->Play();
//      sprExprosion->animation = animation;

      engine.AddActor(sprExprosion);

      // 煙エフェクトを発生
      const std::shared_ptr<Texture> texSmoke =
        engine.LoadTexture("Res/Sprite/smoke.tga");
#if 0
      const float smokeCount = 12;
      for (float i = 0; i < smokeCount; ++i) {
        const float r = glm::radians(360.0f / smokeCount * i);
        const glm::vec3 v(std::cos(r), 0, std::sin(r));
        const glm::vec3 pos = position + glm::vec3(0, 1, 0) + v;
        std::shared_ptr<Sprite> sprite = std::make_shared<Sprite>(pos, texSmoke);
        sprite->velocity = v * 6.0f;
        sprite->lifespan = 1.5f;
        sprite->gravityScale = 0;
        sprite->pixelsPerMeter = 10;
        engine.AddActor(sprite);
      }
#else
      auto curveScale = AnimationCurve::Create({ { 0, 2}, { 1, 8 } });
      AnimationCurvePtr curveRotation = AnimationCurve::Create();
      curveRotation->SetKey(KeyFrame{ 0, 0 });
      curveRotation->SetKey(KeyFrame{ 1, glm::radians(180.0f) });

      auto curveAlpha = AnimationCurve::Create({ { 0, 1 }, { 0.66f, 0.9f }, { 1, 0 } });
      for (float i = 0; i < 8; ++i) {
        glm::vec3 v;
        v.x = std::cos(glm::radians(360.0f / 8.0f * i));
        v.y = 0;
        v.z = std::sin(glm::radians(360.0f / 8.0f * i));
        glm::vec3 pos = position;
        std::shared_ptr<Sprite> sprite = std::make_shared<Sprite>(pos + v + glm::vec3(0, 1, 0), texSmoke);
        sprite->lifespan = 1.0f;
        sprite->pixelsPerMeter = 50;
        engine.AddActor(sprite);

        AnimationClipPtr clip = AnimationClip::Create();
        auto curveX = AnimationCurve::Create({ { 0, v.x * 10.0f }, {1, -v.x} });
        auto curveY = AnimationCurve::Create({ { 0, v.y * 10.0f }, {1, -v.y} });
        auto curveZ = AnimationCurve::Create({ { 0, v.z * 10.0f }, {1, -v.z} });
        clip->SetCurve(ANIMATION_TARGET(Actor, velocity.x), curveX);
        clip->SetCurve(ANIMATION_TARGET(Actor, velocity.y), curveY);
        clip->SetCurve(ANIMATION_TARGET(Actor, velocity.z), curveZ);
        clip->SetCurve(ANIMATION_TARGET(Actor, scale.x), curveScale);
        clip->SetCurve(ANIMATION_TARGET(Actor, scale.y), curveScale);
        clip->SetCurve([](Actor& a, float v) { a.rotation = v; }, curveRotation);
        clip->SetCurve(ANIMATION_TARGET(Actor, color.a), curveAlpha);

        AnimationPtr animation = Animation::Create();
        animation->SetClip(clip);
        animation->Play();
        sprite->SetAnimation(animation);
      }
#endif
#endif
      // 爆発音を再生
#ifdef USE_EASY_AUDIO
      Audio::PlayOneShot(SE_EXPLOSION);
#else
      Audio::Get().Play(1, CRI_SE_EXPLOSION);
#endif // USE_EASY_AUDIO
    }
    contact.b->isDead = true; // 弾を消去する
  }
}

