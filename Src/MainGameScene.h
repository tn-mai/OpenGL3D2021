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
#include "Sprite.h"
#include "FramebufferObject.h"
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

  void AddActor(ActorPtr p);
  void AddSprite(const std::shared_ptr<Sprite>& sprite) {
    sprites.push_back(sprite);
  }
  void AddBloodSprite(const glm::vec3& position);
  ActorPtr GetPlayerActor();
  const glm::vec3& GetMouseCursor() const;

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
  ActorList newActors;
  std::shared_ptr<PlayerActor> playerActor;

  // 出現させる敵の数.
  size_t appearanceEnemyCount = 100;

  // クリア条件を満たしたかどうか.
  bool isGameClear = false;
  bool isGameOver = false;

  // 残り経過時間.
  float remainingDeltaTime = 0;

  std::vector<std::shared_ptr<Sprite>> sprites;
  SpriteRenderer spriteRenderer;

  std::shared_ptr<FramebufferObject> fboMain;
};

#endif // MAINGAMESCENE_H_INCLUDED
