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

private:
  void AddLineOfTrees(const glm::vec3& start, const glm::vec3& direction);

  std::shared_ptr<Texture::Image2D> texGround = nullptr;
  std::shared_ptr<Texture::Image2D> texTree = nullptr;
  std::shared_ptr<Texture::Image2D> texHouse = nullptr;
  std::shared_ptr<Texture::Image2D> texCube = nullptr;
  std::shared_ptr<Texture::Image2D> texZombie;
  std::shared_ptr<Texture::Image2D> texPlayer;
  std::shared_ptr<Texture::Image2D> texBullet;
  std::shared_ptr<Texture::Image2D> texGameClear;
  std::shared_ptr<Texture::Image2D> texBlack;
  std::shared_ptr<Texture::Image2D> texPointer;
  std::shared_ptr<Texture::Image2D> texWoodenBarrior;

  Shader::PointLight pointLight;

  glm::mat4 matProj = glm::mat4(1);
  glm::mat4 matView = glm::mat4(1);

  ActorList actors;
  ActorPtr playerActor;
  ActorPtr cursorActor;
  ActorPtr builderActor;

  float shotTimer = 0;
  const float shotInterval = 0.1f;
  int leftOfRounds = 0;
  const int maxRounds = 3;

  // 出現させる敵の数.
  size_t appearanceEnemyCount = 100;

  // クリア条件を満たしたかどうか.
  bool isGameClear = false;
};

#endif // MAINGAMESCENE_H_INCLUDED
