/**
* @file RandomMovingEnemyActor.cpp
*/
#include "RandomMovingEnemyActor.h"
#include "BulletActor.h"
#include "../GameEngine.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

/**
* コンストラクタ
*/
RandomMovingEnemyActor::RandomMovingEnemyActor(
  const std::string& name,
  const Primitive& prim,
  std::shared_ptr<Texture> tex,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment,
  const std::shared_ptr<GameMap>& gamemap) :
  Actor(name, prim, tex, position, scale, rotation, adjustment), // 基底クラスを初期化
  gamemap(gamemap),
  posGoals{ position, position },
  shotInterval(static_cast<float>(GameEngine::Get().GetRandom() % 9 + 1))
{
}

/**
* アクターの状態を更新する
*
* @param deltaTime 前回の更新からの経過時間(秒)
*/
void RandomMovingEnemyActor::OnUpdate(float deltaTime)
{
  /*
    「現在の移動先」と「次の移動先」を保持.
    「次の移動先」が「現在の移動先」の延長線上に…
      ない: 「現在の移動先」で一時停止.
      ある: 次の移動先を設定(一時停止しない).
  */
  if (timer > 0) {
    timer -= deltaTime;
  }

  GameEngine& engine = GameEngine::Get();

  // 移動先0までの距離を計算
  const float length = glm::length(posGoals[0] - position);

  // 距離が2m未満なら移動先0に到着したとみなす
  bool shouldSearchNextGoal = false;
  if (length < 2.0f) {
    // 移動先0と移動先1の移動方向が等しい場合、移動先1を移動先0にコピーし、新しい移動先1を作成する
    if (dirGoals[0] == dirGoals[1]) {
      shouldSearchNextGoal = true;
    } else {
      // 速度が1m/s未満になったら停止したとみなし、次の移動先を設定する
      if (glm::length(velocity) < 1) {
        shouldSearchNextGoal = true;
      }
    }
  }

  if (shouldSearchNextGoal || shouldRotate) {
    shouldSearchNextGoal = false;
    posGoals[0] = posGoals[1];
    dirGoals[0] = dirGoals[1];

    // マップを参照して移動可能な方向を調べる
    glm::vec3 pass[4] = {};
    bool hasPass[4] = {};
    const glm::vec3 dirList[] = { { 0, 0, 4}, { 4, 0, 0}, { 0, 0,-4}, {-4, 0, 0} };
    glm::vec3 endGoal = shouldRotate ? position : posGoals[1];
    endGoal.x = std::floor((endGoal.x + 2.0f) / 4.0f) * 4.0f;
    endGoal.z = std::floor((endGoal.z + 2.0f) / 4.0f) * 4.0f;
    for (int i = 0; i < 4; ++i) {
      if (shouldRotate && i == dirGoals[0]) {
        continue;
      }
      const float x = endGoal.x + dirList[i].x;
      const float z = endGoal.z + dirList[i].z;
      const int objectNo = gamemap->GetObjectNo(x, z);
      if (objectNo == 0) {
        pass[i] = glm::vec3(x, endGoal.y, z);
        hasPass[i] = true;
      }
    }

    unsigned int maxScore = 0;
    int nextDir = -1;
    const unsigned int basicScore[] = { 50, 1, 50, 90, 50, 1, 50 };
    for (int i = 0; i < 4; ++i) {
      if (hasPass[i]) {
        unsigned int score = engine.GetRandom() % 100 + basicScore[i + 3 - dirGoals[1]];
        if (score > maxScore) {
          maxScore = score;
          nextDir = i;
        }
      }
    }

    if (nextDir >= 0) {
      posGoals[1] = pass[nextDir];
      dirGoals[1] = nextDir;
    }

    if (shouldRotate) {
      posGoals[0] = posGoals[1];
      dirGoals[0] = dirGoals[1];
      shouldRotate = false;
      timer = 2.0f;
    }
  }

  // 目的地が正面に来るように旋回する
  const glm::mat4 matRot = glm::rotate(glm::mat4(1), rotation, glm::vec3(0, 1, 0));
  const glm::vec3 front = matRot * glm::vec4(0, 0, 1, 1);
  const glm::vec3 dirGoal = glm::normalize(posGoals[0] - position);
  const float r = std::acos(glm::dot(front, dirGoal));
  const float speed = glm::min(glm::radians(90.0f) * deltaTime, r);
  if (length >= 2.0f) {
    const glm::vec3 n = glm::cross(front, dirGoal);
    if (n.y >= 0) {
      rotation += speed;
    } else {
      rotation -= speed;
    }
  }
  
  if (r <= speed * 4) {
    // 移動先までの距離に応じて加速/減速する
    glm::vec3 dist = posGoals[0] - position;
    dist.y = 0;
    glm::vec3 targetVelocity = glm::clamp(dist * 4.0f, -8.0f, 8.0f);
    glm::vec3 error = targetVelocity - velocity;
    glm::vec3 force = glm::clamp(error * 4.0f, -20.0f, 20.0f);
    velocity += force * deltaTime;
  }

  shotInterval -= deltaTime;
  if (shotInterval <= 0) {
    shotInterval = static_cast<float>(engine.GetRandom() % 7 + 1);

    glm::vec3 position = this->position + front * 3.0f;
    position.y += 2.0f;

    std::shared_ptr<Actor> bullet(new BulletActor{
      "EnemyBullet", engine.GetPrimitive(9), engine.LoadTexture("Res/Bullet.tga"),
      position, glm::vec3(0.25f), rotation, glm::vec3(0) });
    bullet->lifespan = 1.5f;
    bullet->velocity = front * 20.0f;
    bullet->collider = Box::Create(glm::vec3(-0.25f), glm::vec3(0.25f));
    bullet->mass = 6.8f;
    bullet->friction = 1.0f;

    engine.AddActor(bullet);
  }
}

/**
* 衝突を処理する
*
* @param contact 衝突情報
*/
void RandomMovingEnemyActor::OnCollision(const struct Contact& contact)
{
  if (contact.b->name == "EnemyBullet") {
    return;
  }

  if (contact.b->name == "Bullet") {
    // T-34戦車の耐久値を減らす
    health -= 1;
    if (health <= 0) {
      isDead = true; // T-34戦車を消去する
    }
    contact.b->isDead = true; // 弾を消去する
  } else {
    if (timer <= 0) {
      if (std::abs(contact.normal.y) < 0.0001f) {
        glm::vec3 v = contact.velocityA - contact.velocityB;
        v.y = 0;
        const float l = glm::length(v);
        if (l > 0.0001f) {
          const float r = std::acos(glm::dot(-v * (1.0f / l), contact.normal));
          if (r < glm::radians(45.0f)) {
            shouldRotate = true;
            timer = 2.0f;
          }
        }
      }
    }
  }
}

