/**
* @file TitleScene.cpp
*/
#include "TitleScene.h"
#include "Global.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

/**
* タイトル画面を初期化する.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool TitleScene::Initialize()
{
  texLogo = std::make_shared<Texture::Image2D>("Res/TitleLogo.tga");
  texPressEnter = std::make_shared<Texture::Image2D>("Res/PressEnter.tga");
  std::cout << "[情報] TitleSceneを開始.\n";
  return true;
}

/**
* タイトル画面のキー入力を処理する.
*
* @param window GLFWウィンドウへのポインタ.
*/
void TitleScene::ProcessInput(GLFWwindow* window)
{
  Global& global = Global::Get();
  if (glfwGetKey(window, GLFW_KEY_ENTER)) {
    global.sceneId = 1;
  }
}

/**
* タイトル画面を更新する.
*
* @param window    GLFWウィンドウへのポインタ.
* @param deltaTime 前回の更新からの経過時間.
*/
void TitleScene::Update(GLFWwindow*, float deltaTime)
{
  alpha += 0.5f * deltaTime;
  if (alpha >= 1) {
    alpha = 1;
  }
}

/**
* タイトル画面を描画する.
*
* @param window GLFWウィンドウへのポインタ.
*/
void TitleScene::Render(GLFWwindow*)
{
  Global& global = Global::Get();
  std::shared_ptr<Shader::Pipeline> pipeline = global.pipelineSimple;
  Mesh::PrimitiveBuffer& primitiveBuffer = global.primitiveBuffer;
  Texture::Sampler& sampler = global.sampler;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glClearColor(0.3f, 0.2f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const glm::vec3 viewPosition(0, 0, 100);

  // 座標変換行列を作成.
  int w, h;
  glfwGetWindowSize(global.window, &w, &h);
  const glm::mat4 matProj = glm::ortho<float>(-w / 2.0f, w / 2.0f, -h / 2.0f, h / 2.0f, 1.0f, 500.0f);
  const glm::mat4 matView =
    glm::lookAt(glm::vec3(0, 0, 100), glm::vec3(0), glm::vec3(0, 1, 0));

  primitiveBuffer.BindVertexArray();
  pipeline->Bind();
  sampler.Bind(0);

  // タイトルロゴを描画.
  {
    const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(0, 200, 0));
    const glm::mat4 matModelS = glm::scale(glm::mat4(1), glm::vec3(texLogo->Width(), texLogo->Height(), 1));
    const glm::mat4 matMVP = matProj * matView * matModelT * matModelS;
    pipeline->SetMVP(matMVP);
    pipeline->SetObjectColor(glm::vec4(1, 1, 1, alpha));
    texLogo->Bind(0);
    global.Draw(Global::PrimitiveId::plane);
  }
  {
    const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(0, -200, 0));
    const glm::mat4 matModelS = glm::scale(glm::mat4(1), glm::vec3(texPressEnter->Width(), texPressEnter->Height(), 1));
    const glm::mat4 matMVP = matProj * matView * matModelT * matModelS;
    pipeline->SetMVP(matMVP);
    pipeline->SetObjectColor(glm::vec4(1));
    texPressEnter->Bind(0);
    global.Draw(Global::PrimitiveId::plane);
  }
}

/**
* タイトル画面を終了する.
*/
void TitleScene::Finalize()
{
  std::cout << "[情報] TitleSceneを終了.\n";
}
