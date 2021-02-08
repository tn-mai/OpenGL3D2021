/**
* @file PlayerActor.cpp
*/
#define NOMINMAX
#include "PlayerActor.h"
#include "GrenadeActor.h"
#include "../MainGameScene.h"
#include "../GameData.h"
#include "../Audio.h"
#include "../Audio/MainWorkUnit/SE.h"

/**
* コンストラクタ.
*
* @param pos    アクターを配置する座標.
* @param rotY   アクターの向き.
* @param pScene メインゲームシーンのアドレス.
*/
PlayerActor::PlayerActor(const glm::vec3& pos, float rotY,
  MainGameScene* pScene) :
  Actor("player", nullptr,
    std::make_shared<Texture::Image2D>("Res/player_male.tga"),
    pos),
  pMainGameScene(pScene)
{
  // 重力の影響率を設定.
  gravityScale = 1;

  health = 10;
  SetAnimation(GameData::Get().anmPlayerIdle);
  SetCylinderCollision(1.7f, 0, 0.5f);
  OnHit = [](Actor& a, Actor& b) {
    if (b.name == "zombie_attack") {
      // 死んでいたら何もしない.
      if (a.state == Actor::State::dead) {
        return;
      }
      // 無敵タイマー稼働中は衝突しない.
      if (a.timer > 0) {
        return;
      }
      // 耐久力を減らす.
      a.health -= 1;
      b.collision.shape = Collision::Shape::none;
      // 耐久力が0より大きければダメージアニメーションを再生し、無敵タイマーを設定.
      // 0以下なら死亡.
      if (a.health > 0) {
        a.SetAnimation(GameData::Get().anmPlayerDamage);
        a.state = Actor::State::damage;
        a.timer = 2;
      } else {
        a.velocity = glm::vec3(0);
        a.timer = 3;
        a.SetAnimation(GameData::Get().anmPlayerDown);
        a.state = Actor::State::dead;
      }
    }
  };

  texBullet = std::make_shared<Texture::Image2D>("Res/Bullet.tga");
  texGrenade = std::make_shared<Texture::Image2D>("Res/m67_grenade.tga");
  texWoodenBarrior = std::make_shared<Texture::Image2D>("Res/wooden_barrier.tga");
}

/**
* プレイヤーアクターの入力処理.
*/
void PlayerActor::ProcessInput()
{
  GameData& global = GameData::Get();

  // プレイヤーアクターを移動させる.
  glm::vec3 direction = glm::vec3(0);
  if (global.keyPressed & GameData::Key::left) {
    direction.x -= 1;
  } else if (global.keyPressed & GameData::Key::right) {
    direction.x += 1;
  }
  if (global.keyPressed & GameData::Key::up) {
    direction.z -= 1;
  } else if (global.keyPressed & GameData::Key::down) {
    direction.z += 1;
  }

  std::shared_ptr<Animation> nextAnime;
  if (glm::length(direction) > 0) {
    //playerActor->rotation.y = std::atan2(-direction.z, direction.x);
    const float speed = 4.0f;
    direction = glm::normalize(direction);
    velocity.x = direction.x * speed;
    velocity.z = direction.z * speed;

    const glm::vec3 front(std::cos(rotation.y), 0, -std::sin(rotation.y));
    const float cf = glm::dot(front, direction);
    if (cf > std::cos(glm::radians(45.0f))) {
      nextAnime = GameData::Get().anmPlayerRunFront;
    } else if (cf < std::cos(glm::radians(135.0f))) {
      nextAnime = GameData::Get().anmPlayerRunBack;
    } else {
      const glm::vec3 right(std::cos(rotation.y-glm::radians(90.0f)), 0, -std::sin(rotation.y-glm::radians(90.0f)));
      const float cr = glm::dot(right, direction);
      if (cr > std::cos(glm::radians(90.0f))) {
        nextAnime = GameData::Get().anmPlayerRunRight;
      } else {
        nextAnime = GameData::Get().anmPlayerRunLeft;
      }
    }
  } else {
    velocity.x = velocity.z = 0;
    nextAnime = GameData::Get().anmPlayerIdle;
  }

  // ジャンプ.
  // maxH = v^2 / 2g
  // v = sqrt(maxH * 2g)
  if (GameData::Get().keyPressedInLastFrame & GameData::Key::jump) {
    velocity.y = 4.0f;
  }

  // ダメージアニメ再生中はダメージアニメが終わるまで待つ.
  if (animation == global.anmPlayerDamage) {
    if (animationNo >= animation->list.size() - 1) {
      SetAnimation(nextAnime);
    }
  } else {
    SetAnimation(nextAnime);
  }

  // 発射キーが押されていたら三点射を起動.
  if (GameData::Get().keyPressedInLastFrame & GameData::Key::shot) {
    leftOfRounds = maxRounds;
    shotTimer = 0;
  }
  // 発射数が残っていて発射タイマーが0以下なら1発撃つ.
  if (leftOfRounds > 0 && shotTimer <= 0) {
    --leftOfRounds;
    shotTimer = shotInterval;

    // プレイヤーのY軸回転から正面方向を計算.
    const float fx = std::cos(rotation.y);
    const float fz = -std::sin(rotation.y); // Z軸の向きは数学と逆.
    const glm::vec3 front = glm::vec3(fx, 0, fz);

    // プレイヤーのY軸回転から右方向を計算.
    const float rx = std::cos(rotation.y - glm::radians(90.0f));
    const float rz = -std::sin(rotation.y - glm::radians(90.0f)); // 同上
    const glm::vec3 right = glm::vec3(rx, 0, rz);

    // 弾丸の発射位置(銃口)を計算. 3Dモデルを調べたところ、銃口は
    // プレイヤーの座標(足元)から前に0.6m、右に0.2m、上に0.9mの位置にある.
    const glm::vec3 bulletPosition =
      position + front * 0.6f + right * 0.2f + glm::vec3(0, 0.9f, 0);

    // 弾丸アクターを銃口の位置に作成.
    std::shared_ptr<Actor> bullet = std::make_shared<Actor>("bullet",
      &global.primitiveBuffer.Get(GameData::PrimNo::bullet), texBullet, bulletPosition);

    // 向き(回転)はプレイヤーアクターを継承.
    bullet->rotation = rotation;

    // front方向へ「毎秒20m」の速度で移動するように設定.
    bullet->velocity = front * 20.0f;

    // 衝突形状を設定.
    bullet->SetCylinderCollision(0.1f, -0.1f, 0.125f);
    bullet->collision.blockOtherActors = false;

    // 衝突処理を設定.
    bullet->OnHit = [](Actor& a, Actor& b) {
      // 衝突先が弾丸またはプレイヤーの場合は何もしない.
      if (b.name == "bullet" || b.name == "player") {
        return;
      }
      // 弾丸を消去.
      a.isDead = true;
    };

    // アクターをリストに追加.
    pMainGameScene->AddActor(bullet);

    Audio::Instance().Play(1, CRI_SE_BANG_1);
  }

  // 手榴弾を投げる.
  if (GameData::Get().keyPressedInLastFrame & GameData::Key::grenade) {
    // front方向へ「毎秒20m」の速度で移動するように設定.
    // プレイヤーのY軸回転から正面方向を計算.
    const float fx = std::cos(rotation.y);
    const float fz = -std::sin(rotation.y); // Z軸の向きは数学と逆.
    const glm::vec3 front = glm::vec3(fx, 0, fz);
    const glm::vec3 vel(front.x * 4, 4, front.z * 4);

    ActorPtr grenade = std::make_shared<GrenadeActor>(
      position + front * 0.6f + glm::vec3(0, 1.5f, 0), vel, rotation.y, pMainGameScene);
    pMainGameScene->AddActor(grenade);

    Audio::Instance().Play(1, CRI_SE_SLAP_0);
  }

  // 右クリックでバリケードを配置.
  if (!builderActor) {
    if (GameData::Get().keyPressed & GameData::Key::build) {
      builderActor = std::make_shared<Actor>("WoodenBarrior",
        &global.primitiveBuffer.Get(GameData::PrimNo::wooden_barrior), texWoodenBarrior, pMainGameScene->GetMouseCursor());
      // 衝突形状を設定.
      builderActor->SetBoxCollision(glm::vec3(-1, 0, -0.1f), glm::vec3(1, 2, 0.1f));
      builderActor->collision.blockOtherActors = false;
      builderActor->isShadowCaster = false;
      builderActor->health = 10;
      builderActor->OnHit = [](Actor& a, Actor& b) {
        a.baseColor = glm::vec4(1, 0.2f, 0.2f, 0.5f);
      };
      pMainGameScene->AddActor(builderActor);
    }
  }

  if (builderActor) {
    builderActor->position = pMainGameScene->GetMouseCursor();
    builderActor->position.y = 0;

    // 配置方向を90°単位で回転.
    if (GameData::Get().keyPressed & GameData::Key::scrollup) {
      builderActor->rotation.y -= glm::radians(90.0f);
    }
    if (GameData::Get().keyPressed & GameData::Key::scrolldown) {
      builderActor->rotation.y += glm::radians(90.0f);
    }

    // 角度に合わせて衝突判定を設定.
    builderActor->rotation.y = std::fmod(builderActor->rotation.y + glm::radians(360.0f), glm::radians(360.0f));
    if (std::abs(builderActor->rotation.y - glm::radians(90.0f)) < glm::radians(5.0f)) {
      builderActor->SetBoxCollision(glm::vec3(-0.25f, 0, -1), glm::vec3(0.25f, 2, 1));
    } else if (std::abs(builderActor->rotation.y - glm::radians(270.0f)) < glm::radians(5.0f)) {
      builderActor->SetBoxCollision(glm::vec3(-0.25f, 0, -1), glm::vec3(0.25f, 2, 1));
    } else {
      builderActor->SetBoxCollision(glm::vec3(-1, 0, -0.25f), glm::vec3(1, 2, 0.25f));
    }

    // キーが離されたらバリケードを配置.
    if (!(GameData::Get().keyPressed & GameData::Key::build)) {
      if (builderActor->baseColor.r < 1) {
        builderActor->baseColor = glm::vec4(1);
        builderActor->collision.blockOtherActors = true;
        builderActor->isShadowCaster = true;
        builderActor->OnHit = [](Actor& a, Actor& b) {
          if (b.name == "zombie_attack") {
            a.health -= 1;
            if (a.health <= 0) {
              a.isDead = true;
            }
            b.collision.shape = Collision::Shape::none;
          }
        };
      } else {
        builderActor->isDead = true;
      }
      builderActor.reset();
    }

    if (builderActor) {
      builderActor->baseColor = glm::vec4(0.2f, 0.2f, 1, 0.5f);
    }
  }

}

/**
* プレイヤーアクターの状態を更新する.
*
* @param deltaTime 前回の更新からの経過時間(秒).
*/
void PlayerActor::OnUpdate(float deltaTime)
{
  if (leftOfRounds > 0) {
    shotTimer -= deltaTime;
  }
}

