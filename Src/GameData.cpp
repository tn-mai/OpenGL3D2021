/**
* @file GameData.cpp
*/
#include "GameData.h"
#include <iostream>

/**
*
*/
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  GameData::Get().tmpScroll += yoffset;
}

/**
*
*/
GameData& GameData::Get()
{
  static GameData singleton;
  return singleton;
}

/**
* デストラクタ.
*/
GameData::~GameData()
{
  std::cout << "[情報] ゲームデータを破棄.\n";
}

/**
* グローバルデータを初期化する.
*
* @param window GLFWウィンドウへのポインタ.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool GameData::Initialize(GLFWwindow* window)
{
  std::cout << "[情報] ゲームデータの初期化を開始.\n";

  glfwSetScrollCallback(window, ScrollCallback);

  // プリミティブバッファにモデルデータを読み込む.
  if (!primitiveBuffer.Allocate(200'000, 800'000)) {
    return false;
  }
  primitiveBuffer.AddFromObjFile("Res/Ground.obj");
  primitiveBuffer.AddFromObjFile("Res/Tree.obj");
  primitiveBuffer.AddFromObjFile("Res/House.obj");
  primitiveBuffer.AddFromObjFile("Res/Cube.obj");
  primitiveBuffer.AddFromObjFile("Res/Plane.obj");
  primitiveBuffer.AddFromObjFile("Res/Bullet.obj");
  primitiveBuffer.AddFromObjFile("Res/wooden_barrier.obj");

  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_0.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_1.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_2.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_3.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_4.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_5.obj");

  primitiveBuffer.AddFromObjFile("Res/zombie_male_down_0.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_down_1.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_down_2.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_down_3.obj");

  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_0.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_1.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_2.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_3.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_4.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_5.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_attack_6.obj");

  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_damage_0.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_damage_1.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_damage_2.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male/zombie_male_damage_3.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male_idle_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_idle_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_idle_2.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male_run_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_2.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_3.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_4.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_5.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_back_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_back_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_back_2.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_back_3.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_back_4.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_back_5.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_left_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_left_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_left_2.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_left_3.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_left_4.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_left_5.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_right_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_right_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_right_2.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_right_3.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_right_4.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_run_right_5.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_down_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_down_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_down_2.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_down_3.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_damage_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_damage_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male/player_male_damage_2.obj");

  // パイプライン・オブジェクトを作成する.
  pipeline = std::make_shared<Shader::Pipeline>("Res/FragmentLighting.vert", "Res/FragmentLighting.frag");
  if (!pipeline || !*pipeline) {
    return false;
  }
  pipelineSimple = std::make_shared<Shader::Pipeline>("Res/Simple.vert", "Res/Simple.frag");
  if (!pipelineSimple || !*pipelineSimple) {
    return false;
  }
  pipelineShadow = std::make_shared<Shader::Pipeline>("Res/Shadow.vert", "Res/Shadow.frag");
  if (!pipelineShadow || !*pipelineShadow) {
    return false;
  }

  // サンプラ・オブジェクトを作成する.
  sampler.SetWrapMode(GL_REPEAT);
  sampler.SetFilter(GL_NEAREST);

  this->window = window;

  random.seed(std::random_device{}());

  /* アニメーションデータを作成 */

  anmZombieMaleWalk = std::make_shared<Animation>();
  anmZombieMaleWalk->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_walk_0));
  anmZombieMaleWalk->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_walk_1));
  anmZombieMaleWalk->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_walk_2));
  anmZombieMaleWalk->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_walk_3));
  anmZombieMaleWalk->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_walk_4));
  anmZombieMaleWalk->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_walk_5));
  anmZombieMaleWalk->interval = 0.2f;

  anmZombieMaleDown = std::make_shared<Animation>();
  anmZombieMaleDown->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_down_0));
  anmZombieMaleDown->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_down_1));
  anmZombieMaleDown->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_down_2));
  anmZombieMaleDown->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_down_3));
  anmZombieMaleDown->interval = 0.125f;
  anmZombieMaleDown->isLoop = false;

  anmZombieMaleAttack = std::make_shared<Animation>();
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_0));
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_1));
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_2));
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_3));
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_4));
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_5));
  anmZombieMaleAttack->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_attack_6));
  anmZombieMaleAttack->interval = 0.125f;
  anmZombieMaleAttack->isLoop = false;

  anmZombieMaleDamage = std::make_shared<Animation>();
  anmZombieMaleDamage->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_damage_0));
  anmZombieMaleDamage->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_damage_1));
  anmZombieMaleDamage->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_damage_2));
  anmZombieMaleDamage->list.push_back(&primitiveBuffer.Get(PrimNo::zombie_male_damage_3));
  anmZombieMaleDamage->interval = 0.1f;
  anmZombieMaleDamage->isLoop = false;


  anmPlayerIdle = std::make_shared<Animation>();
  anmPlayerIdle->list.push_back(&primitiveBuffer.Get(PrimNo::player_idle_0));
  anmPlayerIdle->list.push_back(&primitiveBuffer.Get(PrimNo::player_idle_1));
  anmPlayerIdle->list.push_back(&primitiveBuffer.Get(PrimNo::player_idle_2));
  anmPlayerIdle->list.push_back(&primitiveBuffer.Get(PrimNo::player_idle_1));
  anmPlayerIdle->interval = 0.2f;

  anmPlayerRunFront = std::make_shared<Animation>();
  anmPlayerRunFront->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_front_0));
  anmPlayerRunFront->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_front_1));
  anmPlayerRunFront->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_front_2));
  anmPlayerRunFront->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_front_3));
  anmPlayerRunFront->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_front_4));
  anmPlayerRunFront->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_front_5));
  anmPlayerRunFront->interval = 0.125f;

  anmPlayerRunBack = std::make_shared<Animation>();
  anmPlayerRunBack->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_back_0));
  anmPlayerRunBack->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_back_1));
  anmPlayerRunBack->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_back_2));
  anmPlayerRunBack->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_back_3));
  anmPlayerRunBack->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_back_4));
  anmPlayerRunBack->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_back_5));
  anmPlayerRunBack->interval = 0.125f;

  anmPlayerRunLeft = std::make_shared<Animation>();
  anmPlayerRunLeft->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_left_0));
  anmPlayerRunLeft->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_left_1));
  anmPlayerRunLeft->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_left_2));
  anmPlayerRunLeft->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_left_3));
  anmPlayerRunLeft->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_left_4));
  anmPlayerRunLeft->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_left_5));
  anmPlayerRunLeft->interval = 0.125f;

  anmPlayerRunRight = std::make_shared<Animation>();
  anmPlayerRunRight->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_right_0));
  anmPlayerRunRight->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_right_1));
  anmPlayerRunRight->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_right_2));
  anmPlayerRunRight->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_right_3));
  anmPlayerRunRight->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_right_4));
  anmPlayerRunRight->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_right_5));
  anmPlayerRunRight->interval = 0.125f;

  anmPlayerDown = std::make_shared<Animation>();
  anmPlayerDown->list.push_back(&primitiveBuffer.Get(PrimNo::player_down_0));
  anmPlayerDown->list.push_back(&primitiveBuffer.Get(PrimNo::player_down_1));
  anmPlayerDown->list.push_back(&primitiveBuffer.Get(PrimNo::player_down_2));
  anmPlayerDown->list.push_back(&primitiveBuffer.Get(PrimNo::player_down_3));
  anmPlayerDown->interval = 0.2f;
  anmPlayerDown->isLoop = false;

  anmPlayerDamage = std::make_shared<Animation>();
  anmPlayerDamage->list.push_back(&primitiveBuffer.Get(PrimNo::player_damage_1));
  anmPlayerDamage->list.push_back(&primitiveBuffer.Get(PrimNo::player_damage_2));
  anmPlayerDamage->list.push_back(&primitiveBuffer.Get(PrimNo::player_damage_1));
  anmPlayerDamage->list.push_back(&primitiveBuffer.Get(PrimNo::player_damage_0));
  anmPlayerDamage->interval = 0.1f;
  anmPlayerDamage->isLoop = false;

  std::cout << "[情報] ゲームデータの初期化を完了.\n";
  return true;
}

/**
* ゲームデータの更新.
*/
void GameData::Update()
{
  // [キー情報の更新]
  {
    // ゲームデータのキー番号とGLFWのキー番号の対応表を作る.
    // 操作キーを増やしたり変えたいときはこの対応表を変更する.
    const struct {
      Key keyGamedata;  // ゲームデータのキー.
      uint32_t keyGlfw; // GLFWのキー.
    } keyMap[] = {
      { Key::enter, GLFW_KEY_ENTER },
      { Key::left,  GLFW_KEY_A },
      { Key::right, GLFW_KEY_D },
      { Key::up,    GLFW_KEY_W },
      { Key::down,  GLFW_KEY_S },
    };

    // 現在押されているキーを取得.
    uint32_t newKey = 0; // 現在押されているキー.
    for (const auto& e : keyMap) {
      if (glfwGetKey(window, e.keyGlfw) == GLFW_PRESS) {
        newKey |= e.keyGamedata;
      }
    }
    // マウスの左ボタンで射撃.
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
      newKey |= Key::shot;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
      newKey |= Key::build;
    }

    // 前回のUpdateで押されておらず(~keyPressed)、
    // かつ現在押されている(newKey)キーを最後のフレームで押されたキーに設定.
    keyPressedInLastFrame = ~keyPressed & newKey;

    // 押されているキーを更新.
    keyPressed = newKey;
  }

  // マウスカーソル座標の更新.
  {
    // マウスカーソル座標を変数xとyに取得.
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    /* 取得した座標をOpenGL座標系に変換. */

    // ウィンドウサイズを変数wとhに取得.
    int w, h;
    glfwGetWindowSize(window, &w, &h);

    // 「左下原点、上が+Y」の座標系に変換.
    // 1を引いているのは、例えば高さ720の場合、座標が取りうる範囲は0～719の720段階になるため。
    y = (h - 1.0) - y;

    // 「画面中心が原点、上が+Y」の座標系(つまりOpenGLの座標系)に変換.
    x -= w * 0.5;
    y -= h * 0.5;

    // 座標をfloat型に変換してcursorPositionメンバ変数に代入.
    // (OpenGLは基本的にfloat型で処理を行うので、型を合わせておくと扱いやすい).
    cursorPosition.x = static_cast<float>(x);
    cursorPosition.y = static_cast<float>(y);
  }

  // スクロールの更新.
  {
    prevScroll = curScroll;
    curScroll = tmpScroll;
  }
}

/**
* プリミティブを描画する.
*
* @param id プリミティブのID.
*/
void GameData::Draw(GameData::PrimNo id) const
{
  primitiveBuffer.Get(static_cast<size_t>(id)).Draw();
}
