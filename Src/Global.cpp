/**
* @file Global.cpp
*/
#include "Global.h"
#include <iostream>


Global* Global::p;

/**
*
*/
Global& Global::Get()
{
  return *p;
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
  if (Global::p) {
    return true;
  }
  Global* p = new Global;// = std::make_shared<Derived>();

  // プリミティブバッファにモデルデータを読み込む.
  if (!p->primitiveBuffer.Allocate(20'000, 80'000)) {
    return false;
  }
  p->primitiveBuffer.AddFromObjFile("Res/Ground.obj");
  p->primitiveBuffer.AddFromObjFile("Res/Tree.obj");
  p->primitiveBuffer.AddFromObjFile("Res/House.obj");
  p->primitiveBuffer.AddFromObjFile("Res/Cube.obj");
  p->primitiveBuffer.AddFromObjFile("Res/Plane.obj");

  // パイプライン・オブジェクトを作成する.
  p->pipeline = std::make_shared<Shader::Pipeline>("Res/FragmentLighting.vert", "Res/FragmentLighting.frag");
  if (!p->pipeline || !*p->pipeline) {
    return false;
  }

  // サンプラ・オブジェクトを作成する.
  p->sampler.SetWrapMode(GL_REPEAT);
  p->sampler.SetFilter(GL_NEAREST);

  p->window = window;
  Global::p = p;

  std::cout << "[情報] グローバルデータを初期化.\n";

  return true;
}

/**
* グローバルデータを破棄する.
*/
void Global::Finalize()
{
  p = nullptr;
  std::cout << "[情報] グローバルデータを破棄.\n";
}

/**
* プリミティブを描画する.
*
* @param id プリミティブのID.
*/
void Global::Draw(Global::PrimitiveId id) const
{
  primitiveBuffer.Get(static_cast<size_t>(id)).Draw();
}



