/**
* @file MainGameScene.cpp
*
* 時は21世紀。「ナチス2.0」の科学中佐「ゲラルト・ベルコフ」(47)は、人間をゾンビ化する秘術「デス・ストラクチャリング」を実用化。
* ナチス復活と世界征服の野望の手始めに、とあるヨーロッパの辺鄙な片田舎をゾンビまみれにしようとしていた。
*
* SAS予備役でサバイバル・インストラクターの「ジョン・スミス」(31)は、たまたま夏季休暇で訪れた町でこの奇貨に巻き込まれてしまう。
* かろうじて逃げ込んだ町の教会で、牧師の「アンソニー・ウェスト」から驚くべき事実を告げられたのだった。
* 「あなたは神が遣わされた戦士なのです。この「祝福されたAK-47」と「聖なる手榴弾」をお取りなさい。」
* 「そして神の栄光をもって、邪な死者どもに安息をもたらすのです。」
* ジョンは驚きながらも「無辜の市民を守るのが軍人の務め。」という上官「エドガー・ラッセル」(35)の教えを思い出し、運命に従うことにする。
* ナチス2.0の野望をくじくため、荒れ果てた町とゾンビの巣食う深い森を駆け抜けるジョンの過酷な戦いが始まった！
*
* 「遠慮はいらねえ。奴らはとっくに死んでるんだ。」
*
* [後期の内容候補]
*
* 方向性:
* - ホラー感より爽快感を重視.
* - ホラー感はゾンビの大群という時点で十分.
* - 派手な爆発、炎上、誘爆、そして飛び散る血しぶき.
*
* プログラムの強化点:
* - サウンド.
* - マウスによる照準と射撃. 半直線と衝突図形との衝突判定が必要.
* - 球の衝突判定.
* - Y軸回転した直方体の衝突判定.
* - フェードイン・アウト.
* - モーフィングによる補完アニメーション
* - オフスクリーンサーフェスによる画面効果.
* - ノーマルマップ.
* - 三角ポリゴン・四角ポリゴンと直線の衝突判定.
* - モデルデータの軽量化.
* - 専用のモデルフォーマットの開発.
*
* プレイヤーの機能:
* - 手榴弾を投げる. 転がす(追加のアニメーションが必要).
* - 下方向への攻撃手段.
* - バリケード作成.
* - 前転、振りほどき、ジャンプ、吹き飛びダウン、ダウンからの起き上がり.
* - 近接攻撃.
*
* ゾンビの機能:
* - ゾンビモデルのバリエーションを増やす.
* - 下半身損傷or欠損による匍匐移動および攻撃.
* - 吹き飛びダウン.
* - ダウンからの起き上がり.
* - 頭部爆散からのダウン.
* - 上半身爆散からのダウン.
* - 炎上.
* - 炎上ダウン.
* - 噛みつき攻撃.
* - すしざんまいのポーズ.
* - ボスゾンビ.
*/
#include "MainGameScene.h"
#include "GameData.h"
#include "SceneManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

/**
* 並木を描画する.
*/
void MainGameScene::AddLineOfTrees(const glm::vec3& start, const glm::vec3& direction)
{
  GameData& global = GameData::Get();

  glm::vec3 offset = start;
  for (float i = 0; i < 19; ++i) {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>("tree",
      &global.primitiveBuffer.Get(GameData::PrimNo::tree),
      texTree, start + direction * i);
    actor->rotation.y = glm::radians(i * 30);
    actor->SetBoxCollision(glm::vec3(-1, 0, -1), glm::vec3(1, 6, 1));
    actors.push_back(actor);
  }
}

/**
* メインゲーム画面を初期化する.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool MainGameScene::Initialize()
{
  texGround = std::make_shared<Texture::Image2D>("Res/Ground.tga");
  texTree   = std::make_shared<Texture::Image2D>("Res/Tree.tga");
  texHouse  = std::make_shared<Texture::Image2D>("Res/House.tga");
  texCube   = std::make_shared<Texture::Image2D>("Res/Rock.tga");
  if (!texGround ||!texTree || !texHouse || !texCube) {
    return false;
  }

  texZombie = std::make_shared<Texture::Image2D>("Res/zombie_male.tga");
  texPlayer = std::make_shared<Texture::Image2D>("Res/player_male.tga");
  texBullet = std::make_shared<Texture::Image2D>("Res/Bullet.tga");
  texGameClear = std::make_shared<Texture::Image2D>("Res/Survived.tga");
  texGameOver = std::make_shared<Texture::Image2D>("Res/GameOver.tga");
  texBlack = std::make_shared<Texture::Image2D>("Res/Black.tga");
  texPointer = std::make_shared<Texture::Image2D>("Res/Pointer.tga");
  texWoodenBarrior = std::make_shared<Texture::Image2D>("Res/wooden_barrier.tga");

  GameData& global = GameData::Get();

  std::random_device rd;
  std::mt19937 random(rd());

  // 木を表示.
  for (float j = 0; j < 4; ++j) {
    const glm::mat4 matRot = glm::rotate(glm::mat4(1), glm::radians(90.0f) * j, glm::vec3(0, 1, 0));
    AddLineOfTrees(matRot * glm::vec4(-19, 0, 19, 1), matRot * glm::vec4(2, 0, 0, 1));
  }

  // 家を表示.
  {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>(
      "house", &global.primitiveBuffer.Get(GameData::PrimNo::house), texHouse, glm::vec3(0));
    actor->SetBoxCollision(glm::vec3(-3, 0, -3), glm::vec3(3, 5, 3));
    actors.push_back(actor);
  }

  // 立方体を表示.
  {
    std::shared_ptr<Actor> actor = std::make_shared<Actor>(
      "cube", &global.primitiveBuffer.Get(GameData::PrimNo::cube), texCube, glm::vec3(10, 1, 0));
    actor->SetBoxCollision(glm::vec3(-1), glm::vec3(1));
    actors.push_back(actor);
  }

  // プレイヤーを表示.
  {
    playerActor = std::make_shared<Actor>("player", &global.primitiveBuffer.Get(GameData::PrimNo::player_idle_0),
      texPlayer, glm::vec3(10, 0, 10));
    playerActor->health = 10;
    playerActor->SetAnimation(GameData::Get().anmPlayerIdle);
    playerActor->SetCylinderCollision(1.7f, 0, 0.5f);
    playerActor->OnHit = [](Actor& a, Actor& b) {
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
    actors.push_back(playerActor);
  }

  // マウスカーソル位置を示すアクター.
  cursorActor = std::make_shared<Actor>("cursor", &global.primitiveBuffer.Get(GameData::PrimNo::cube), texCube, glm::vec3(0));
  cursorActor->rotation = glm::vec3(0.75f, 0.75f, 0.75f);
  cursorActor->scale = glm::vec3(0.25f);
  actors.push_back(cursorActor);

  // ゾンビを表示.
  const Mesh::Primitive* pPrimitive = &global.primitiveBuffer.Get(GameData::PrimNo::zombie_male_walk_0);
  for (size_t i = 0; i < appearanceEnemyCount; ++i) {
    glm::vec3 pos(0);
    pos.x = std::uniform_real_distribution<float>(-18, 18)(global.random);
    pos.z = std::uniform_real_distribution<float>(-18, 18)(global.random);
    std::shared_ptr<Actor> actor = std::make_shared<Actor>("zombie", pPrimitive, texZombie, pos);
    actor->health = 5;
    actor->rotation.y =
      std::uniform_real_distribution<float>(0, glm::radians(360.0f))(global.random);
    // アニメーションを設定.
    actor->SetAnimation(GameData::Get().anmZombieMaleWalk);
    actor->state = Actor::State::run;
    actor->SetCylinderCollision(1.7f, 0, 0.5f);

    // 衝突処理を設定.
    actor->OnHit = [](Actor& a, Actor& b) {
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
          // 衝突判定を無くす.
          a.collision.shape = Collision::Shape::none;
          // 死亡状態に設定.
          a.state = Actor::State::dead;
          // 倒したゾンビの数を1体増やす.
          ++GameData::Get().killCount;
        }
      }
    };
    actors.push_back(actor);
  }

  // 点光源を設定する
  pointLight = Shader::PointLight{
    glm::vec4(8, 10,-8, 0),
    glm::vec4(0.4f, 0.7f, 1.0f, 0) * 200.0f
  };

  // ゲームデータの初期設定.
  GameData& gamedata = GameData::Get();
  gamedata.killCount = 0;

  // マウスカーソルを非表示にする.
  glfwSetInputMode(gamedata.window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

  std::cout << "[情報] MainGameSceneを開始.\n";
  return true;
}

/**
* メインゲーム画面のキー入力を処理する.
*
* @param window    GLFWウィンドウへのポインタ.
*/
void MainGameScene::ProcessInput(GLFWwindow* window)
{
  // クリアしている?
  GameData& gamedata = GameData::Get();
  if (isGameClear) {
    // Enterキーが押されたらタイトル画面に移動.
    if (gamedata.keyPressedInLastFrame & GameData::Key::enter) {
      SceneManager::Get().ChangeScene(TITLE_SCENE_NAME);
    }
    return;
  }

  // ゲームオーバー?
  if (isGameOver) {
    // Enterキーが押されたらタイトル画面に移動.
    if (gamedata.keyPressedInLastFrame & GameData::Key::enter) {
      SceneManager::Get().ChangeScene(TITLE_SCENE_NAME);
    }
    return;
  }

  // プレイヤーが死んでいたら
  if (playerActor->state == Actor::State::dead) {
    // アニメーションが終了していたらゲームオーバーにする.
    if (playerActor->animationNo >= playerActor->animation->list.size() - 1) {
      isGameOver = true;
    }
    return;
  }

  // マウスポインタを地面に表示.
  {
    const glm::vec2 cursor = GameData::Get().cursorPosition;

    Segment seg;
    const glm::vec2 screenPosition((cursor.x / 1280) * 2, (cursor.y / 720) * 2);
    const glm::mat4 matInverseVP = glm::inverse(matProj * matView);

    const glm::vec4 start = matInverseVP * glm::vec4(screenPosition, -1, 1);
    seg.start = glm::vec3(start) / start.w;

    const glm::vec4 end = matInverseVP * glm::vec4(screenPosition, 1, 1);
    seg.end = glm::vec3(end) / end.w;

    const Plane plane{ glm::vec3(0, 1, 0), glm::vec3(0, 1, 0) };
    Intersect(seg, plane, &cursorActor->position);

    const glm::vec3 direction(cursorActor->position - playerActor->position);
    playerActor->rotation.y = std::atan2(-direction.z, direction.x);
  }

  // プレイヤーアクターを移動させる.
  glm::vec3 direction = glm::vec3(0);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    direction.x -= 1;
  } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    direction.x += 1;
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    direction.z -= 1;
  } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    direction.z += 1;
  }

  GameData& global = GameData::Get();

  std::shared_ptr<Animation> nextAnime;
  if (glm::length(direction) > 0) {
    //playerActor->rotation.y = std::atan2(-direction.z, direction.x);
    const float speed = 4.0f;
    direction = glm::normalize(direction);
    playerActor->velocity = direction * speed;

    const glm::vec3 front(std::cos(playerActor->rotation.y), 0, -std::sin(playerActor->rotation.y));
    const float cf = glm::dot(front, direction);
    if (cf > std::cos(glm::radians(45.0f))) {
      nextAnime = GameData::Get().anmPlayerRunFront;
    } else if (cf < std::cos(glm::radians(135.0f))) {
      nextAnime = GameData::Get().anmPlayerRunBack;
    } else {
      const glm::vec3 right(std::cos(playerActor->rotation.y-glm::radians(90.0f)), 0, -std::sin(playerActor->rotation.y-glm::radians(90.0f)));
      const float cr = glm::dot(right, direction);
      if (cr > std::cos(glm::radians(90.0f))) {
        nextAnime = GameData::Get().anmPlayerRunRight;
      } else {
        nextAnime = GameData::Get().anmPlayerRunLeft;
      }
    }
  } else {
    playerActor->velocity = glm::vec3(0);
    nextAnime = GameData::Get().anmPlayerIdle;
  }

  // ダメージアニメ再生中はダメージアニメが終わるまで待つ.
  if (playerActor->animation == global.anmPlayerDamage) {
    if (playerActor->animationNo >= playerActor->animation->list.size() - 1) {
      playerActor->SetAnimation(nextAnime);
    }
  } else {
    playerActor->SetAnimation(nextAnime);
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
    const float fx = std::cos(playerActor->rotation.y);
    const float fz = -std::sin(playerActor->rotation.y); // Z軸の向きは数学と逆.
    const glm::vec3 front = glm::vec3(fx, 0, fz);

    // プレイヤーのY軸回転から右方向を計算.
    const float rx = std::cos(playerActor->rotation.y - glm::radians(90.0f));
    const float rz = -std::sin(playerActor->rotation.y - glm::radians(90.0f)); // 同上
    const glm::vec3 right = glm::vec3(rx, 0, rz);

    // 弾丸の発射位置(銃口)を計算. 3Dモデルを調べたところ、銃口は
    // プレイヤーの座標(足元)から前に0.6m、右に0.2m、上に0.9mの位置にある.
    const glm::vec3 position =
      playerActor->position + front * 0.6f + right * 0.2f + glm::vec3(0, 0.9f, 0);

    // 弾丸アクターを銃口の位置に作成.
    std::shared_ptr<Actor> bullet = std::make_shared<Actor>("bullet",
      &global.primitiveBuffer.Get(GameData::PrimNo::bullet), texBullet, position);

    // 向き(回転)はプレイヤーアクターを継承.
    bullet->rotation = playerActor->rotation;

    // front方向へ「毎秒20m」の速度で移動するように設定.
    bullet->velocity = front * 20.0f;

    // 衝突形状を設定.
    bullet->SetCylinderCollision(0.1f, -0.1f, 0.125f);
    bullet->collision.isBlock = false;

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
    actors.push_back(bullet);
  }

  // 右クリックでバリケードを配置.
  if (!builderActor) {
    if (GameData::Get().keyPressed & GameData::Key::build) {
      builderActor = std::make_shared<Actor>("WoodenBarrior",
        &global.primitiveBuffer.Get(GameData::PrimNo::wooden_barrior), texBlack, cursorActor->position);
      builderActor->texture = texWoodenBarrior;
      // 衝突形状を設定.
      builderActor->SetBoxCollision(glm::vec3(-1, 0, -0.1f), glm::vec3(1, 2, 0.1f));
      builderActor->collision.isBlock = true;
    }
  }

  if (builderActor) {
    builderActor->position = cursorActor->position;
    builderActor->position.y = 0;

    const double scroll = global.curScroll - global.prevScroll;
    if (scroll <= -1) {
      builderActor->rotation.y -= glm::radians(90.0f);
    } else if (scroll >= 1) {
      builderActor->rotation.y += glm::radians(90.0f);
    }
    builderActor->rotation.y = std::fmod(builderActor->rotation.y + glm::radians(360.0f), glm::radians(360.0f));
    if (std::abs(builderActor->rotation.y - glm::radians(90.0f)) < glm::radians(5.0f)) {
      builderActor->SetBoxCollision(glm::vec3(-0.25f, -1, -1), glm::vec3(0.25f, 1, 1));
    } else if (std::abs(builderActor->rotation.y - glm::radians(270.0f)) < glm::radians(5.0f)) {
      builderActor->SetBoxCollision(glm::vec3(-0.25f, -1, -1), glm::vec3(0.25f, 1, 1));
    } else {
      builderActor->SetBoxCollision(glm::vec3(-1, -1, -0.25f), glm::vec3(1, 1, 0.25f));
    }

    glm::vec4 color = glm::vec4(0.2f, 0.2f, 1, 0.5f);
    for (size_t i = 0; i < actors.size(); ++i) {
      if (DetectCollision(*builderActor, *actors[i], false)) {
        color = glm::vec4(1, 0.2f, 0.2f, 0.5f);
        break;
      }
    }
    builderActor->baseColor = color;

    if (!(GameData::Get().keyPressed & GameData::Key::build)) {
      if (builderActor->baseColor.r < 1) {
        builderActor->baseColor = glm::vec4(1);
        actors.push_back(builderActor);
      }
      builderActor.reset();
    }
  }
}

/**
* メインゲーム画面を更新する.
*
* @param window    GLFWウィンドウへのポインタ.
* @param deltaTime 前回の更新からの経過時間.
*/
void MainGameScene::Update(GLFWwindow* window, float deltaTime)
{
  if (leftOfRounds > 0) {
    shotTimer -= deltaTime;
  }

  float dt = deltaTime;
  deltaTime = 1.0f / 60.0f;
  for (; dt > 0; dt -= 1.0f / 60.0f) {
    if (dt < deltaTime) {
      deltaTime = dt;
    }
    // アクターの行動を処理.
    ActorList newActors; // 新規アクターの配列.
    newActors.reserve(100);
    for (auto& e : actors) {
      // ゾンビアクターの場合.
      if (e->name == "zombie") {
        // ゾンビの行動.
        // 1. +X方向に直進.
        // 2. 現在向いている方向に直進.
        // 3. プレイヤーの方向を向く.
        // 4. 少しずつプレイヤーの方向を向く.

        // 攻撃中以外なら攻撃範囲を削除する.
        if (e->attackActor && e->state != Actor::State::attack) {
          e->attackActor->isDead = true;
          e->attackActor = nullptr;
        }

        // ダメージ状態の場合.
        if (e->state == Actor::State::damage) {
          // アニメが終了したら移動状態にする.
          if (e->animationNo >= e->animation->list.size() - 1) {
            e->velocity = glm::vec3(0);
            e->SetAnimation(GameData::Get().anmZombieMaleWalk);
            e->state = Actor::State::run;
          }
        }
        // 攻撃中なら攻撃終了を待つ.
        else if (e->state == Actor::State::attack) {
          // アニメーション番号がアニメ枚数以上だったら、攻撃アニメ終了とみなす.
          if (e->animationNo >= e->animation->list.size() - 1) {
            e->SetAnimation(GameData::Get().anmZombieMaleWalk);
            e->state = Actor::State::run;
          }
          // アニメ番号が4以上かつ攻撃範囲が存在すれば攻撃範囲を削除する.
          else if (e->animationNo >= 4 && e->attackActor) {
            e->attackActor->isDead = true;
            e->attackActor.reset();
          }
          // アニメ番号が3以上かつ攻撃範囲が存在しなければ攻撃範囲を作成する.
          else if (e->animationNo >= 3 && !e->attackActor) {
            // ゾンビの正面方向を計算.
            const glm::vec3 front(std::cos(e->rotation.y), 0, -std::sin(e->rotation.y));
            // 攻撃判定の発生位置を計算.
            const glm::vec3 pos = e->position + glm::vec3(0, 0.9f, 0) + front;
            // 攻撃判定アクターを作成.
            e->attackActor = std::make_shared<Actor>("zombie_attack", nullptr, nullptr, pos);
            // 攻撃判定を設定.
            e->attackActor->SetCylinderCollision(0.2f, -0.2f, 0.1f);
            e->attackActor->collision.isBlock = false;
            newActors.push_back(e->attackActor);
          }
        }

        // 死んでいなければ歩く.
        else if (e->state == Actor::State::run) {
          // プレイヤーのいる方向を計算.
          glm::vec3 toPlayer = playerActor->position - e->position;
          // ゾンビの正面方向を計算.
          glm::vec3 front(std::cos(e->rotation.y), 0, -std::sin(e->rotation.y));
          // 左右どちらに回転するかを決めるために外積を計算.
          const glm::vec3 c = glm::cross(front, toPlayer);
          // 垂直ベクトルのy座標がプラス側なら向きを増やし、マイナス側なら減らす.
          constexpr float speed = glm::radians(60.0f);
          if (c.y >= 0) {
            e->rotation.y += speed * deltaTime;
          } else {
            e->rotation.y -= speed * deltaTime;
          }
          // 360度を超えたら0度に戻す.
          constexpr float r360 = glm::radians(360.0f);
          e->rotation.y = fmod(e->rotation.y + r360, r360);
          // 向きが変化したので、正面方向のベクトルを計算しなおす.
          front.x = std::cos(e->rotation.y);
          front.z = -std::sin(e->rotation.y);
          // 正面方向に1m/sの速度で移動するように設定.
          e->velocity = front;

          // プレイヤーが生存中かつ距離3m以内かつ正面60度以内にいたら攻撃.
          if (playerActor->state != Actor::State::dead) {
            const float distanceSq = glm::dot(toPlayer, toPlayer);
            if (distanceSq <= 3 * 3) {
              const float distance = std::sqrt(distanceSq);
              const float angle = std::acos(glm::dot(front, toPlayer * (1.0f / distance)));
              if (angle <= glm::radians(30.0f)) {
                e->SetAnimation(GameData::Get().anmZombieMaleAttack);
                e->state = Actor::State::attack;
              }
            }
          }
        } else {
          e->velocity = glm::vec3(0);
        }
      }
    }

    // 新規アクターが存在するなら、それをアクターリストに追加する.
    if (!newActors.empty()) {
      actors.insert(actors.end(), newActors.begin(), newActors.end());
      newActors.clear();
    }

    // アクターリストに含まれるアクターの状態を更新する.
    UpdateActorList(actors, deltaTime);

    // 衝突判定.
    for (size_t ia = 0; ia < actors.size(); ++ia) {
      Actor& a = *actors[ia]; // アクターA
      // アクターAが死亡している場合は衝突しない.
      if (a.isDead) {
        continue;
      }
      // 計算済み及び自分自身を除く、残りのアクターとの間で衝突判定を実行.
      for (size_t ib = ia + 1; ib < actors.size(); ++ib) {
        Actor& b = *actors[ib]; // アクターB
        // アクターBが死亡している場合は衝突しない.
        if (b.isDead) {
          continue;
        }
        // 衝突判定.
        if (DetectCollision(a, b, true)) {
          // 衝突していたら、双方のOnHit関数を実行する.
          a.OnHit(a, b);
          b.OnHit(b, a);
        }
      } // 閉じ括弧の数に注意.
    }
  }

  // まだクリアしていない?
  if (!isGameClear) {
    // クリア条件(「倒した敵の数」が「出現する敵の数」以上)を満たしている?
    if (GameData::Get().killCount >= appearanceEnemyCount) {
      // ゲームクリアフラグをtrueにする.
      isGameClear = true;

      // プレイヤーアクターを待機状態にする.
      playerActor->velocity = glm::vec3(0);
      playerActor->SetAnimation(GameData::Get().anmPlayerIdle);

      std::cout << "[情報] ゲームクリア条件を達成\n";
    }
  }

  // カメラをプレイヤーアクターのななめ上に配置.
  const glm::vec3 viewPosition = playerActor->position + glm::vec3(0, 7, 7);
  // プレイヤーアクターの足元が画面の中央に映るようにする.
  const glm::vec3 viewTarget = playerActor->position;

  // 座標変換行列を作成.
  int w, h;
  glfwGetWindowSize(window, &w, &h);
  const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
  matProj =
    glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 500.0f);
  matView =
    glm::lookAt(viewPosition, viewTarget, glm::vec3(0, 1, 0));
}

/**
* メインゲーム画面を描画する.
*
* @param window    GLFWウィンドウへのポインタ.
*/
void MainGameScene::Render(GLFWwindow* window) const
{
  int fbw, fbh;
  glfwGetFramebufferSize(window, &fbw, &fbh);
  if (fbw <= 0 || fbh <= 0) {
    return;
  }

  GameData& global = GameData::Get();
  std::shared_ptr<Shader::Pipeline> pipeline = global.pipeline;
  Mesh::PrimitiveBuffer& primitiveBuffer = global.primitiveBuffer;
  Texture::Sampler& sampler = global.sampler;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  //glEnable(GL_FRAMEBUFFER_SRGB);
  glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // 環境光を設定する.
  pipeline->SetAmbientLight(glm::vec3(0.1f, 0.125f, 0.15f));

  // 平行光源を設定する
  const Shader::DirectionalLight directionalLight{
    glm::normalize(glm::vec4(3,-2,-2, 0)),
    glm::vec4(1, 0.9f, 0.8f, 1)
  };
  pipeline->SetLight(directionalLight);

  pipeline->SetLight(pointLight);

  primitiveBuffer.BindVertexArray();
  pipeline->Bind();
  sampler.Bind(0);

  // 地面を描画.
  {
    const glm::mat4 matModel = glm::mat4(1);
    const glm::mat4 matMVP = matProj * matView * matModel;
    pipeline->SetMVP(matMVP);
    pipeline->SetModelMatrix(matModel);
    pipeline->SetObjectColor(glm::vec4(1));
    texGround->Bind(0);
    primitiveBuffer.Get(GameData::PrimNo::ground).Draw();
  }

  // アクターリストを描画.
  const glm::mat4 matVP = matProj * matView;
  for (size_t i = 0; i < actors.size(); ++i) {
    actors[i]->Draw(*pipeline, matVP, Actor::DrawType::color);
  }

  // 未確定の建築物を描画.
  if (builderActor) {
    builderActor->Draw(*pipeline, matVP, Actor::DrawType::color);
  }

  // 点光源の位置を描画.
  {
    // Y軸回転.
    const float degree = static_cast<float>(std::fmod(glfwGetTime() * 180.0, 360.0));
    const glm::mat4 matModelR =
      glm::rotate(glm::mat4(1), glm::radians(degree), glm::vec3(0, 1, 0));
    // 拡大縮小.
    const glm::mat4 matModelS =
      glm::scale(glm::mat4(1), glm::vec3(0.5f, 0.25f, 0.5f));
    // 平行移動.
    const glm::mat4 matModelT =
      glm::translate(glm::mat4(1), glm::vec3(pointLight.position) + glm::vec3(0, -1.25f, 0));
    // 拡大縮小・回転・平行移動を合成.
    const glm::mat4 matModel = matModelT * matModelR * matModelS;
    pipeline->SetMVP(matProj * matView * matModel);
    pipeline->SetModelMatrix(matModel);
    texTree->Bind(0);
    primitiveBuffer.Get(GameData::PrimNo::tree).Draw();
  }

  // アクターの影を描画.
  {
    // ステンシルバッファを有効にする.
    glEnable(GL_STENCIL_TEST);
    // 「比較に使う値」を1にして、常に比較が成功するように設定.
    glStencilFunc(GL_ALWAYS, 1, 0xff);
    // ステンシル深度の両方のテストに成功した場合に「比較する値」を書き込むように設定.
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    // ステンシルバッファの全ビットの書き込みを許可.
    glStencilMask(0xff);
    // カラーバッファへの書き込みを禁止.
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    // 深度バッファへの書き込みを禁止.
    glDepthMask(GL_FALSE);

    // 高さ1mの物体が落とす影の長さを計算.
    const float scale = 1.0f / -directionalLight.direction.y;
    const float sx = directionalLight.direction.x * scale;
    const float sz = directionalLight.direction.z * scale;

    // ぺちゃんこ行列(Y座標を0にする行列)を作成.
    const glm::mat4 matShadow(
      1.00f, 0.00f, 0.00f, 0.00f,
      sx, 0.00f, sz, 0.00f,
      0.00f, 0.00f, 1.00f, 0.00f,
      0.00f, 0.01f, 0.00f, 1.00f);

    // 影用パイプランをバインド.
    std::shared_ptr<Shader::Pipeline> pipelineShadow = GameData::Get().pipelineShadow;
    pipelineShadow->Bind();

    // ぺちゃんこ行列→ビュー行列→プロジェクション行列の順番に掛ける行列を作る.
    const glm::mat4 matShadowVP = matVP * matShadow;

    // ぺちゃんこビュープロジェクション行列を使って全てのアクターを描画する.
    for (const auto& actor : actors) {
      actor->Draw(*pipelineShadow, matShadowVP, Actor::DrawType::shadow);
    }

    // ステンシル値が1の場合のみテストに成功するように設定.
    glStencilFunc(GL_EQUAL, 1, 0xff);
    // ステンシルバッファ
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    // カラーバッファへの描き込みを許可.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    // 深度バッファを無効化.
    glDisable(GL_DEPTH_TEST);

    // 画面全体に影色を塗る.
    texBlack->Bind(0);
    pipelineShadow->SetMVP(glm::scale(glm::mat4(1), glm::vec3(2)));
    primitiveBuffer.Get(GameData::PrimNo::plane).Draw();

    // ステンシルバッファを無効化.
    glDisable(GL_STENCIL_TEST);
    // 深度バッファを有効化.
    glEnable(GL_DEPTH_TEST);
    // 深度バッファへの描き込みを許可.
    glDepthMask(GL_TRUE);
  }

  // 2D表示.
  {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // 座標変換行列を作成.
    const float halfW = fbw / 2.0f;
    const float halfH = fbh / 2.0f;
    const glm::mat4 matProj =
      glm::ortho<float>(-halfW, halfW, -halfH, halfH, 1.0f, 500.0f);
    const glm::mat4 matView =
      glm::lookAt(glm::vec3(0, 0, 100), glm::vec3(0), glm::vec3(0, 1, 0));
    const glm::mat4 matVP = matProj * matView;

    std::shared_ptr<Shader::Pipeline> pipeline2D = GameData::Get().pipelineSimple;
    pipeline2D->Bind();

    // マウスカーソル位置を描画.
    {
      const glm::mat4 matModelS = glm::scale(glm::mat4(1),
        glm::vec3(texPointer->Width(), texPointer->Height(), 1));
      const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(GameData::Get().cursorPosition, 0));
      const glm::mat4 matModel = matModelT * matModelS;
      pipeline2D->SetMVP(matVP * matModel);
      texPointer->Bind(0);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }

    // ゲームクリア画像を描画.
    if (isGameClear) {
      const glm::mat4 matModelS = glm::scale(glm::mat4(1),
        glm::vec3(texGameClear->Width() * 2.0f, texGameClear->Height() * 2.0f, 1));
      const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(0, 100, 0));
      const glm::mat4 matModel = matModelT * matModelS;
      pipeline2D->SetMVP(matVP * matModel);
      texGameClear->Bind(0);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }

    // ゲームオーバー画像を描画.
    if (isGameOver) {
      const glm::mat4 matModelS = glm::scale(glm::mat4(1),
        glm::vec3(texGameOver->Width() * 2.0f, texGameOver->Height() * 2.0f, 1));
      const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(0, 100, 0));
      const glm::mat4 matModel = matModelT * matModelS;
      pipeline2D->SetMVP(matVP * matModel);
      texGameOver->Bind(0);
      primitiveBuffer.Get(GameData::PrimNo::plane).Draw();
    }
  }

  Texture::UnbindAllTextures();
  Texture::UnbindAllSamplers();
  Shader::UnbindPipeline();
  primitiveBuffer.UnbindVertexArray();
}

/**
* メインゲーム画面を終了する.
*/
void MainGameScene::Finalize()
{
  // マウスカーソルを表示する.
  glfwSetInputMode(GameData::Get().window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  std::cout << "[情報] MainGameSceneを終了.\n";
}
