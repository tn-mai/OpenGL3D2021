/**
* @file Global.cpp
*/
#include "Global.h"
#include <iostream>

/**
*
*/
Global& Global::Get()
{
  static Global singleton;
  return singleton;
}

/**
* デストラクタ.
*/
Global::~Global()
{
  std::cout << "[情報] グローバルデータを破棄.\n";
}

/**
* グローバルデータを初期化する.
*
* @param window GLFWウィンドウへのポインタ.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool Global::Initialize(GLFWwindow* window)
{
  // プリミティブバッファにモデルデータを読み込む.
  if (!primitiveBuffer.Allocate(20'000, 80'000)) {
    return false;
  }
  primitiveBuffer.AddFromObjFile("Res/Ground.obj");
  primitiveBuffer.AddFromObjFile("Res/Tree.obj");
  primitiveBuffer.AddFromObjFile("Res/House.obj");
  primitiveBuffer.AddFromObjFile("Res/Cube.obj");
  primitiveBuffer.AddFromObjFile("Res/Plane.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_0.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_1.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_2.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_3.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_4.obj");
  primitiveBuffer.AddFromObjFile("Res/zombie_male_walk_5.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_idle_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_0.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_1.obj");
  primitiveBuffer.AddFromObjFile("Res/player_male_run_2.obj");

  // パイプライン・オブジェクトを作成する.
  pipeline = std::make_shared<Shader::Pipeline>("Res/FragmentLighting.vert", "Res/FragmentLighting.frag");
  if (!pipeline || !*pipeline) {
    return false;
  }
  pipelineSimple = std::make_shared<Shader::Pipeline>("Res/Simple.vert", "Res/Simple.frag");
  if (!pipelineSimple || !*pipelineSimple) {
    return false;
  }

  // サンプラ・オブジェクトを作成する.
  sampler.SetWrapMode(GL_REPEAT);
  sampler.SetFilter(GL_NEAREST);

  this->window = window;

  random.seed(std::random_device{}());

  std::cout << "[情報] グローバルデータを初期化.\n";

  return true;
}

/**
* プリミティブを描画する.
*
* @param id プリミティブのID.
*/
void Global::Draw(Global::PrimNo id) const
{
  primitiveBuffer.Get(static_cast<size_t>(id)).Draw();
}
