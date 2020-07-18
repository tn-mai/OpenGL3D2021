/**
* @file TitleScene.cpp
*/
#include "TitleScene.h"
#include "Global.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

/**
*
*/
bool TitleScene::Initialize()
{
  texLogo = std::make_shared<Texture::Image2D>("Res/TitleLogo.tga");
  return true;
}

/**
*
*/
void TitleScene::ProcessInput()
{
  Global& global = Global::Get();
  if (glfwGetKey(global.window, GLFW_KEY_ENTER)) {
    global.sceneId = 1;
  }
}

/**
*
*/
void TitleScene::Update(GLFWwindow*, float deltaTime)
{
  alpha += 0.5f * deltaTime;
  if (alpha >= 1) {
    alpha = 1;
  }
}

/**
*
*/
void TitleScene::Render(GLFWwindow*)
{
  Global& global = Global::Get();
  std::shared_ptr<Shader::Pipeline> pipeline = global.pipeline;
  Mesh::PrimitiveBuffer& primitiveBuffer = global.primitiveBuffer;
  Texture::Sampler& sampler = global.sampler;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glClearColor(0.5f, 0.3f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // 平行光源を設定する
  const Shader::DirectionalLight directionalLight{
    glm::normalize(glm::vec4(0, 0,-1, 0)),
    glm::vec4(1)
  };
  pipeline->SetLight(directionalLight);

  // 点光源を設定する
  const Shader::PointLight pointLight = Shader::PointLight{
    glm::vec4(0),
    glm::vec4(0)
  };
  pipeline->SetLight(pointLight);

  const glm::vec3 viewPosition(0, 0, 100);

  // 座標変換行列を作成.
  int w, h;
  glfwGetWindowSize(global.window, &w, &h);
  const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
  const glm::mat4 matProj =
    glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 500.0f);
  const glm::mat4 matView =
    glm::lookAt(viewPosition, glm::vec3(0), glm::vec3(0, 1, 0));

  primitiveBuffer.BindVertexArray();
  pipeline->Bind();
  sampler.Bind(0);

  // タイトルロゴを描画.
  {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const float z = 100 - 1 / tan(22.5f) * 4;
    const glm::mat4 matModelT = glm::translate(glm::mat4(1), glm::vec3(0, 0, z));
    const glm::mat4 matModelR = glm::scale(glm::mat4(1), glm::vec3(static_cast<float>(texLogo->Width()) / texLogo->Height(), 1, 1));
    const glm::mat4 matModel = matModelT * matModelR;
    const glm::mat4 matMVP = matProj * matView * matModel;
    pipeline->SetMVP(matMVP);
    pipeline->SetModelMatrix(matModel);
    pipeline->SetObjectColor(glm::vec4(1, 1, 1, alpha));
    texLogo->Bind(0);
    global.Draw(Global::PrimitiveId::plane);
  }
}

/**
*
*/
void TitleScene::Finalize()
{
}
