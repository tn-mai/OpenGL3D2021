/**
* @file SceneManager.h
*/
#ifndef SCENEMANAGER_H_INCLUDED
#define SCENEMANAGER_H_INCLUDED
#include "glad/glad.h"
#include "TitleScene.h"
#include "MainGameScene.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <string>

// シーンを切り替えるときに指定するシーン名.
#define TITLE_SCENE_NAME "TitleScene"
#define MAINGAME_SCENE_NAME "MainGameScene"

/**
* シーン管理クラス.
*/
class SceneManager
{
public:
  static SceneManager& Get();

  void Update(GLFWwindow* window, float deltaTime);
  void Render(GLFWwindow* window) const;
  void ChangeScene(const std::string&);
  void Finalize();

private:
  // クラス変数を作れなくするため、コンストラクタやデストラクタをprivateにする.
  SceneManager() = default;
  ~SceneManager() = default;
  SceneManager(const SceneManager&) = delete;
  SceneManager& operator=(const SceneManager&) = delete;

  std::string currentSceneName; // 実行中のシーン名.
  std::string nextSceneName;    // 次に実行するシーン名.

  std::shared_ptr<TitleScene> titleScene;
  std::shared_ptr<MainGameScene> mainGameScene;
};


#endif // SCENEMANAGER_H_INCLUDED
