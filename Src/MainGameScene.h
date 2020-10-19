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
* ƒƒCƒ“ƒQ[ƒ€‰æ–Ê.
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
  std::shared_ptr<Texture::Image2D> texGameOver;
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

  // ˜AË—p•Ï”.
  float shotTimer = 0; // Ÿ‚Ì’e‚ğ”­Ë‚·‚é‚Ü‚Å‚Ìc‚èŠÔ(•b).
  const float shotInterval = 0.1f; // ’e‚Ì”­ËŠÔŠu(•b).
  int leftOfRounds = 0; // ’e‚Ìc‚è˜AË‰ñ”.
  const int maxRounds = 3; // 1‰ñ‚Ìƒ{ƒ^ƒ““ü—Í‚Å”­Ë‚³‚ê‚é’e”.

  // oŒ»‚³‚¹‚é“G‚Ì”.
  size_t appearanceEnemyCount = 100;

  // ƒNƒŠƒAğŒ‚ğ–‚½‚µ‚½‚©‚Ç‚¤‚©.
  bool isGameClear = false;
  bool isGameOver = false;
};

#endif // MAINGAMESCENE_H_INCLUDED
