/**
* @file MainGameScene.cpp
*
* 時は21世紀。「ナチス2.0」の科学中佐「ゲラルト・ベルコフ」(47)は、人間をゾンビ化する秘術「デス・ストラクチャリング」を実用化。
* ナチス復活と世界征服の野望の手始めに、とあるヨーロッパの辺鄙な片田舎をゾンビまみれにしようとしていた。
*
* SAS予備役でサバイバル・インストラクターの「リサ・エンフィールド」(26)は、たまたま夏季休暇で訪れた町でこの奇貨に巻き込まれてしまう。
* かろうじて逃げ込んだ町の教会で、牧師の「アンソニー・ウェスト」から驚くべき事実を告げられたのだった。
* 「あなたは神が遣わされた戦士なのです。この「祝福されたAK-47」と「聖なる手榴弾」をお取りなさい。」
* 「そして神の栄光をもって、邪な死者どもに安息をもたらすのです。」
* リサは驚きながらも「無辜の市民を守るのが軍人の務め。」という上官「エドガー・ラッセル」(35)の教えを思い出し、運命に従うことにする。
* ナチス2.0の野望をくじくため、荒れ果てた町とゾンビの巣食う深い森を駆け抜けるリサの過酷な戦いが始まった！
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
  texPlayer = std::make_shared<Texture::Image2D>("Res/player_female/player_female.tga");
  texBullet = std::make_shared<Texture::Image2D>("Res/Bullet.tga");
  texGameClear = std::make_shared<Texture::Image2D>("Res/Survived.tga");

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
    playerActor->SetAnimation(GameData::Get().anmPlayerIdle);
    playerActor->SetCylinderCollision(1.7f, 0, 0.5f);
    actors.push_back(playerActor);
  }

  // ゾンビを表示.
  const Mesh::Primitive* pPrimitive = &global.primitiveBuffer.Get(GameData::PrimNo::zombie_male_walk_0);
  for (size_t i = 0; i < appearanceEnemyCount; ++i) {
    glm::vec3 pos(0);
    pos.x = std::uniform_real_distribution<float>(-18, 18)(global.random);
    pos.z = std::uniform_real_distribution<float>(-18, 18)(global.random);
    std::shared_ptr<Actor> actor = std::make_shared<Actor>("zombie", pPrimitive, texZombie, pos);
    actor->rotation.y =
      std::uniform_real_distribution<float>(0, glm::radians(360.0f))(global.random);
    // アニメーションを設定.
    actor->SetAnimation(GameData::Get().anmZombieMaleWalk);
    actor->SetCylinderCollision(1.7f, 0, 0.5f);

    // 衝突処理を設定.
    actor->OnHit = [](Actor& a, Actor& b) {
      if (b.name == "bullet") {
        // 死亡アニメーションを設定.
        a.SetAnimation(GameData::Get().anmZombieMaleDown);
        // 衝突判定を無くす.
        a.collision.shape = Collision::Shape::none;
        // 倒したゾンビの数を1体増やす.
        ++GameData::Get().killCount;
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
    if (gamedata.keyPressedInLastFrame & GameData::Key::enter) {
      SceneManager::Get().ChangeScene(TITLE_SCENE_NAME);
    }
    return;
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

  if (glm::length(direction) > 0) {
    playerActor->rotation.y = std::atan2(-direction.z, direction.x);
    const float speed = 4.0f;
    playerActor->velocity = glm::normalize(direction) * speed;
    playerActor->SetAnimation(GameData::Get().anmPlayerRun);
  } else {
    playerActor->velocity = glm::vec3(0);
    playerActor->SetAnimation(GameData::Get().anmPlayerIdle);
  }

  static bool prevShootKey = false;
  const bool shootKey = glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS;
  const bool shoot = shootKey & !prevShootKey;
  prevShootKey = shootKey;
  if (shoot) {
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
}

/**
* メインゲーム画面を更新する.
*
* @param window    GLFWウィンドウへのポインタ.
* @param deltaTime 前回の更新からの経過時間.
*/
void MainGameScene::Update(GLFWwindow* window, float deltaTime)
{
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
      if (DetectCollision(a, b)) {
        // 衝突していたら、双方のOnHit関数を実行する.
        a.OnHit(a, b);
        b.OnHit(b, a);
      }
    } // 閉じ括弧の数に注意.
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
}

/**
* メインゲーム画面を描画する.
*
* @param window    GLFWウィンドウへのポインタ.
*/
void MainGameScene::Render(GLFWwindow* window) const
{
  GameData& global = GameData::Get();
  std::shared_ptr<Shader::Pipeline> pipeline = global.pipeline;
  Mesh::PrimitiveBuffer& primitiveBuffer = global.primitiveBuffer;
  Texture::Sampler& sampler = global.sampler;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  //glEnable(GL_FRAMEBUFFER_SRGB);
  glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // 環境光を設定する.
  pipeline->SetAmbientLight(glm::vec3(0.1f, 0.125f, 0.15f));

  // 平行光源を設定する
  const Shader::DirectionalLight directionalLight{
    glm::normalize(glm::vec4(3,-2,-2, 0)),
    glm::vec4(1, 0.9f, 0.8f, 1)
  };
  pipeline->SetLight(directionalLight);

  pipeline->SetLight(pointLight);

  const glm::vec3 viewPosition = playerActor->position + glm::vec3(0, 7, 7);
  const glm::vec3 viewTarget = playerActor->position + glm::vec3(0, 0, 0);

  // 座標変換行列を作成.
  int w, h;
  glfwGetWindowSize(window, &w, &h);
  const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
  const glm::mat4 matProj =
    glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 500.0f);
  const glm::mat4 matView =
    glm::lookAt(viewPosition, viewTarget, glm::vec3(0, 1, 0));

  primitiveBuffer.BindVertexArray();
  pipeline->Bind();
  sampler.Bind(0);

  // 地面を描画.
  {
    const glm::mat4 matModel = glm::mat4(1);
    const glm::mat4 matMVP = matProj * matView * matModel;
    pipeline->SetMVP(matMVP);
    pipeline->SetModelMatrix(matModel);
    texGround->Bind(0);
    primitiveBuffer.Get(GameData::PrimNo::ground).Draw();
  }

  // 影描画用の行列を作成.
  // XとZを平行光源の方向に引き伸ばし、Yを0にする.
  // 地面と密着するとちらつくので少し浮かせる.
  glm::mat4 matShadow(1);
  matShadow[1][0] = directionalLight.direction.x;
  matShadow[1][1] = 0;
  matShadow[1][2] = directionalLight.direction.z;
  matShadow[3][3] = 1;
  matShadow[3][1] = 0.01f;

  // アクターリストを描画.
  RenderActorList(actors, matProj * matView, matShadow);

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

  // 2D表示.
  {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // 座標変換行列を作成.
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    const float halfW = w / 2.0f;
    const float halfH = h / 2.0f;
    const glm::mat4 matProj =
      glm::ortho<float>(-halfW, halfW, -halfH, halfH, 1.0f, 500.0f);
    const glm::mat4 matView =
      glm::lookAt(glm::vec3(0, 0, 100), glm::vec3(0), glm::vec3(0, 1, 0));
    const glm::mat4 matVP = matProj * matView;

    std::shared_ptr<Shader::Pipeline> pipeline2D = GameData::Get().pipelineSimple;

    pipeline2D->Bind();

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
  std::cout << "[情報] MainGameSceneを終了.\n";
}
