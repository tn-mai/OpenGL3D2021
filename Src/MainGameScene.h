/**
* @file MainGameScene.h
*/
#ifndef MAINGAMESCENE_H_INCLUDED
#define MAINGAMESCENE_H_INCLUDED
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "Mesh.h"
#include "Shader.h"
#include "Texture.h"
#include "Actor.h"
#include "Actors/PlayerActor.h"
#include <memory>

/**
* メインゲーム画面.
*/
class MainGameScene
{
public:
  MainGameScene() = default;
  ~MainGameScene() { Finalize(); }
  MainGameScene(const MainGameScene&) = delete;
  MainGameScene& operator=(const MainGameScene&) = delete;

  bool Initialize();
  void ProcessInput(GLFWwindow*);
  void Update(GLFWwindow*, float deltaTime);
  void Render(GLFWwindow*) const;
  void Finalize();

  void AddActor(ActorPtr p) { newActors.push_back(p); }
  ActorPtr GetPlayerActor() { return playerActor; }
  const glm::mat4& GetViewMatrix() const { return matView; }
  const glm::mat4& GetProjectionMatrix() const { return matProj; }
  const glm::vec3& GetMouseCursor() const { return posMouseCursor; }

private:
  void AddLineOfTrees(const glm::vec3& start, const glm::vec3& direction);

  std::shared_ptr<Texture::Image2D> texGround = nullptr;
  std::shared_ptr<Texture::Image2D> texTree = nullptr;
  std::shared_ptr<Texture::Image2D> texHouse = nullptr;
  std::shared_ptr<Texture::Image2D> texCube = nullptr;
  std::shared_ptr<Texture::Image2D> texZombie;
  std::shared_ptr<Texture::Image2D> texPlayer;
  std::shared_ptr<Texture::Image2D> texGameClear;
  std::shared_ptr<Texture::Image2D> texGameOver;
  std::shared_ptr<Texture::Image2D> texBlack;
  std::shared_ptr<Texture::Image2D> texPointer;

  Shader::PointLight pointLight;

  glm::mat4 matProj = glm::mat4(1); // プロジェクション行列.
  glm::mat4 matView = glm::mat4(1); // ビュー行列.

  // マウスカーソル座標.
  glm::vec3 posMouseCursor = glm::vec3(0);

  ActorList actors;
  std::shared_ptr<PlayerActor> playerActor;

  ActorList newActors;

  // 出現させる敵の数.
  size_t appearanceEnemyCount = 100;

  // クリア条件を満たしたかどうか.
  bool isGameClear = false;
  bool isGameOver = false;
};

#endif // MAINGAMESCENE_H_INCLUDED
