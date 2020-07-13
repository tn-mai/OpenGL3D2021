/**
* @file MainGameScene.h
*/
#ifndef MAINGAMESCENE_H_INCLUDED
#define MAINGAMESCENE_H_INCLUDED
#include <GLFW/glfw3.h>

/**
* ÉÅÉCÉìÉQÅ[ÉÄâÊñ .
*/
class MainGameScene
{
public:
  MainGameScene() = default;
  ~MainGameScene() = default;
  MainGameScene(const MainGameScene&) = default;
  MainGameScene& operator=(const MainGameScene&) = default;

  bool Initialize();
  void ProcessInput(const GLFWwindow*);
  void Update(float deltaTime);
  void Render(const GLFWwindow*);
  void Finalize();

private:
};

#endif // MAINGAMESCENE_H_INCLUDED
