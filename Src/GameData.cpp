/**
* @file GameData.cpp
*/
#include "GameData.h"
#include <iostream>

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

  // プリミティブバッファにモデルデータを読み込む.
  if (!primitiveBuffer.Allocate(20'000, 80'000)) {
    return false;
  }
  primitiveBuffer.AddFromObjFile("Res/Ground.obj");
  primitiveBuffer.AddFromObjFile("Res/Tree.obj");
  primitiveBuffer.AddFromObjFile("Res/House.obj");
  primitiveBuffer.AddFromObjFile("Res/Cube.obj");
  primitiveBuffer.AddFromObjFile("Res/Plane.obj");
  primitiveBuffer.AddFromObjFile("Res/Bullet.obj");

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

  primitiveBuffer.AddFromObjFile("Res/player_male_idle_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_idle_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_idle_2.obj");

  primitiveBuffer.AddFromObjFile("Res/player_male_run_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_2.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_3.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_4.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_5.obj");

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

  anmPlayerIdle = std::make_shared<Animation>();
  anmPlayerIdle->list.push_back(&primitiveBuffer.Get(PrimNo::player_idle_0));
  anmPlayerIdle->list.push_back(&primitiveBuffer.Get(PrimNo::player_idle_1));
  anmPlayerIdle->list.push_back(&primitiveBuffer.Get(PrimNo::player_idle_2));
  anmPlayerIdle->list.push_back(&primitiveBuffer.Get(PrimNo::player_idle_1));
  anmPlayerIdle->interval = 0.2f;

  anmPlayerRun = std::make_shared<Animation>();
  anmPlayerRun->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_0));
  anmPlayerRun->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_1));
  anmPlayerRun->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_2));
  anmPlayerRun->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_3));
  anmPlayerRun->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_4));
  anmPlayerRun->list.push_back(&primitiveBuffer.Get(PrimNo::player_run_5));
  anmPlayerRun->interval = 0.125f;

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
      { Key::shot,  GLFW_KEY_J },
    };

    // 現在押されているキーを取得.
    uint32_t newKey = 0; // 現在押されているキー.
    for (const auto& e : keyMap) {
      if (glfwGetKey(window, e.keyGlfw) == GLFW_PRESS) {
        newKey |= e.keyGamedata;
      }
    }

    // 前回のUpdateで押されておらず(~keyPressed)、
    // かつ現在押されている(newKey)キーを最後のフレームで押されたキーに設定.
    keyPressedInLastFrame = ~keyPressed & newKey;

    // 押されているキーを更新.
    keyPressed = newKey;
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
