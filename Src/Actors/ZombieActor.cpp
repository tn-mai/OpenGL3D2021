/**
* @file ZombieActor.cpp
*/
#include "ZombieActor.h"
#include "../MainGameScene.h"
#include "../GameData.h"
#include "../Audio.h"
#include "../Audio/MainWorkUnit/SE.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

/**
* コンストラクタ.
*
* @param pos    アクターを配置する座標.
* @param rotY   アクターの向き.
* @param pScene メインゲームシーンのアドレス.
*/
ZombieActor::ZombieActor(const glm::vec3& pos, float rotY,
  MainGameScene* pScene) :
  Actor("zombie", nullptr,
    std::make_shared<Texture::Image2D>("Res/zombie_male.tga"),
    pos),
  pMainGameScene(pScene)
{
  texMetallicSmoothness =
    std::make_shared<Texture::Image2D>("Res/zombie_male/zombie_male_spec.tga", false);

  // 重力の影響率を設定.
  gravityScale = 1;

  // アクターの耐久値を設定.
  health = 5;

  moveSpeed = 1;// std::uniform_real_distribution<float>(1.0f, 3.0f)(GameData::Get().random);
  // アクターのY軸回転を設定.
  rotation.y = rotY;
  // 垂直円柱型の衝突判定を設定.
  SetCylinderCollision(1.7f, 0, 0.5f);

  // 衝突処理を設定.
  OnHit = [](Actor& a, Actor& b) {
    if (a.state == State::dead) {
      return;
    }
    if (b.name == "bullet") {
      // 耐久値を減らす.
      a.health -= 2;
      // 耐久値が0より大きければダメージアニメーションを再生する.
      // 耐久値が0以下になったら死亡.
      if (a.health > 0) {
        // ノックバックを設定する.
        if (glm::dot(b.velocity, b.velocity)) {
          a.velocity += glm::normalize(b.velocity) * 2.0f;
        }
        // 同じアニメは再生できないのでnullptrを指定してアニメを削除する.
        a.SetAnimation(nullptr);
        // ダメージアニメーションを再生.
        a.SetAnimation(GameData::Get().anmZombieMaleDamage);
        // ダメージ状態に設定.
        a.state = Actor::State::damage;
      } else {
        // 死亡アニメーションを設定.
        a.SetAnimation(GameData::Get().anmZombieMaleDown);
        // 衝突判定を極薄くする.
        a.SetCylinderCollision(0.3f, 0, 0.3f);
        // 死亡状態に設定.
        a.state = Actor::State::dead;
        // 倒したゾンビの数を1体増やす.
        ++GameData::Get().killCount;
        Audio::Instance().Play(3, CRI_SE_ZOMBIE_VOICE_0);
        std::cout << "[情報] ゾンビ死亡\n";
      }
      ZombieActor& zombie = static_cast<ZombieActor&>(a);
      for (int i = 0; i < 10; ++i) {
        zombie.pMainGameScene->AddBloodSprite(zombie.position);
      }
      Audio::Instance().Play(3, CRI_SE_GUTTING_0);
    } else if (b.name == "explosion") {
      a.health -= 5;
      if (a.health > 0) {
      } else {
        // 死亡アニメーションを設定.
        a.SetAnimation(GameData::Get().anmZombieMaleDown);
        // 衝突判定を極薄くする.
        a.SetCylinderCollision(0.3f, 0, 0.3f);
        // 死亡状態に設定.
        a.state = Actor::State::dead;
        // 倒したゾンビの数を1体増やす.
        ++GameData::Get().killCount;
      }
      ZombieActor& zombie = static_cast<ZombieActor&>(a);
      for (int i = 0; i < 10; ++i) {
        zombie.pMainGameScene->AddBloodSprite(zombie.position);
      }
      Audio::Instance().Play(3, CRI_SE_GUTTING_1);
    }
  };

  // アニメーションを設定.
  SetAnimation(GameData::Get().anmZombieMaleWalk);
  state = State::run;

  texNormal = GameData::Get().texZombieNormal;
}

/**
* ゾンビの状態を更新する.
*
* @param deltaTime 前回の更新からの経過時間(秒).
*/
void ZombieActor::OnUpdate(float deltaTime)
{
  // ゾンビの行動.
  // 1. +X方向に直進.
  // 2. 現在向いている方向に直進.
  // 3. プレイヤーの方向を向く.
  // 4. 少しずつプレイヤーの方向を向く.

  // 攻撃中以外なら攻撃範囲を削除する.
  if (attackActor && state != Actor::State::attack) {
    attackActor->isDead = true;
    attackActor = nullptr;
  }

  // ダメージ状態の場合.
  if (state == Actor::State::damage) {
    // アニメが終了したら移動状態にする.
    if (animationNo >= animation->list.size() - 1) {
      velocity = glm::vec3(0);
      SetAnimation(GameData::Get().anmZombieMaleWalk);
      state = Actor::State::run;
    }
  }
  // 攻撃中なら攻撃終了を待つ.
  else if (state == Actor::State::attack) {
    const glm::vec3 front(std::cos(rotation.y), 0, -std::sin(rotation.y));
    velocity = front * moveSpeed;
    // アニメーション番号がアニメ枚数以上だったら、攻撃アニメ終了とみなす.
    if (animationNo >= animation->list.size() - 1) {
      SetAnimation(GameData::Get().anmZombieMaleWalk);
      state = Actor::State::run;
    }
    // アニメ番号が4以上かつ攻撃範囲が存在すれば攻撃範囲を削除する.
    else if (animationNo >= 4) {
      if (attackActor) {
        attackActor->isDead = true;
        attackActor.reset();
      }
    }
    // アニメ番号が3以上かつ攻撃範囲が存在しなければ攻撃範囲を作成する.
    else if (animationNo >= 3 && !attackActor) {
      // ゾンビの正面方向を計算.
      const glm::vec3 front(std::cos(rotation.y), 0, -std::sin(rotation.y));
      // 攻撃判定の発生位置を計算.
      const glm::vec3 pos = position + glm::vec3(0, 0.9f, 0) + front;
      // 攻撃判定アクターを作成.
      attackActor = std::make_shared<Actor>("zombie_attack", nullptr, nullptr, pos);
      // 攻撃判定を設定.
      attackActor->SetCylinderCollision(0.2f, -0.2f, 0.1f);
      attackActor->collision.blockOtherActors = false;
      pMainGameScene->AddActor(attackActor);
    }
  }

  // 死んでいなければ歩く.
  else if (state == Actor::State::run) {
    ActorPtr playerActor = pMainGameScene->GetPlayerActor();
    // プレイヤーのいる方向を計算.
    glm::vec3 toPlayer = playerActor->position - position;
    // ゾンビの正面方向を計算.
    glm::vec3 front(std::cos(rotation.y), 0, -std::sin(rotation.y));
    // 左右どちらに回転するかを決めるために外積を計算.
    const glm::vec3 c = glm::cross(front, toPlayer);
    // 垂直ベクトルのy座標がプラス側なら向きを増やし、マイナス側なら減らす.
    constexpr float speed = glm::radians(60.0f);
    if (c.y >= 0) {
      rotation.y += speed * deltaTime;
    } else {
      rotation.y -= speed * deltaTime;
    }
    // 360度を超えたら0度に戻す.
    constexpr float r360 = glm::radians(360.0f);
    rotation.y = fmod(rotation.y + r360, r360);
    // 向きが変化したので、正面方向のベクトルを計算しなおす.
    front.x = std::cos(rotation.y);
    front.z = -std::sin(rotation.y);
    // 正面方向に1m/sの速度で移動するように設定.
    velocity.x = front.x * moveSpeed;
    velocity.z = front.z * moveSpeed;

    // プレイヤーが生存中かつ距離3m以内かつ正面60度以内にいたら攻撃.
    if (playerActor->state != Actor::State::dead) {
      const float distanceSq = glm::dot(toPlayer, toPlayer);
      if (distanceSq <= 3 * 3) {
        const float distance = std::sqrt(distanceSq);
        const float angle = std::acos(glm::dot(front, toPlayer * (1.0f / distance)));
        if (angle <= glm::radians(30.0f)) {
          SetAnimation(GameData::Get().anmZombieMaleAttack);
          state = Actor::State::attack;
        }
      }
    }
  } else if (state != State::dead) {
    velocity.x = velocity.z = 0;
  }
}
