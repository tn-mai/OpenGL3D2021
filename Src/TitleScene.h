/**
* @file TitleScene.h
*/
#ifndef TITLESCENE_H_INCLUDED
#define TITLESCENE_H_INCLUDED
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "Texture.h"
#include <memory>

/**
* ƒ^ƒCƒgƒ‹‰æ–Ê.
*/
class TitleScene
{
public:
  TitleScene() = default;
  ~TitleScene() = default;
  TitleScene(const TitleScene&) = delete;
  TitleScene& operator=(const TitleScene&) = delete;

  bool Initialize();
  void ProcessInput(GLFWwindow*);
  void Update(GLFWwindow*, float);
  void Render(GLFWwindow*);
  void Finalize();

private:
  std::shared_ptr<Texture::Image2D> texLogo;
  std::shared_ptr<Texture::Image2D> texPressEnter;
  float alpha = 0;
};

#endif // TITLESCENE_H_INCLUDED
