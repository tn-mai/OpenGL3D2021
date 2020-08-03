/**
* @file SceneManager.cpp
*/
#include "SceneManager.h"

/**
* シーンマネージャのシングルトン・インスタンスを取得する.
*/
SceneManager& SceneManager::Get()
{
  static SceneManager singleton;
  return singleton;
}

/**
* 現在起動しているシーンを更新する.
*
* @param window    GLコンテキストを管理するGLFWウィンドウ.
* @param deltaTime 前回の更新からの経過時間(秒).
*/
void SceneManager::Update(GLFWwindow* window, float deltaTime)
{
  // 次のシーンが指定されていたら、シーンを切り替える.
  if (!nextSceneName.empty()) {
    // 実行中のシーンを終了する.
    titleScene = nullptr;
    mainGameScene = nullptr;

    // 指定された名前に対応するシーンを作成して初期化する.
    if (nextSceneName == TITLE_SCENE_NAME) {
      titleScene = std::make_shared<TitleScene>();
      titleScene->Initialize();
    } else if (nextSceneName == MAINGAME_SCENE_NAME) {
      mainGameScene = std::make_shared<MainGameScene>();
      mainGameScene->Initialize();
    }

    // 実行中のシーン名を変更する.
    currentSceneName = nextSceneName;
    nextSceneName.clear();
  }

  // 実行中のシーンを更新する.
  if (titleScene) {
    titleScene->ProcessInput(window);
    titleScene->Update(window, deltaTime);
  }
  if (mainGameScene) {
    mainGameScene->ProcessInput(window);
    mainGameScene->Update(window, deltaTime);
  }
}

/**
* 実行中のシーンを描画する.
*
* @param window    GLコンテキストを管理するGLFWウィンドウ.
*/
void SceneManager::Render(GLFWwindow* window) const
{
  if (titleScene) {
    titleScene->Render(window);
  }else if (mainGameScene) {
    mainGameScene->Render(window);
  }
}

/**
* シーンを切り替える.
*
* @param scenaName 次に起動するシーンの名前.
*/
void SceneManager::ChangeScene(const std::string& sceneName)
{
  // 実行中のシーンと同じ名前が指定されていたら何もしないで終了.
  if (sceneName == currentSceneName) {
    return;
  }
  nextSceneName = sceneName;
}

/**
* シーンマネージャを終了する.
*/
void SceneManager::Finalize()
{
  titleScene = nullptr;
  mainGameScene = nullptr;
}

