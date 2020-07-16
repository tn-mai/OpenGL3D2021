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
#include <memory>

/**
* ÉÅÉCÉìÉQÅ[ÉÄâÊñ .
*/
class MainGameScene
{
public:
  MainGameScene() = default;
  ~MainGameScene() { Finalize(); }
  MainGameScene(const MainGameScene&) = delete;
  MainGameScene& operator=(const MainGameScene&) = delete;

  bool Initialize();
  void ProcessInput();
  void Update(GLFWwindow*, float deltaTime);
  void Render(GLFWwindow*) const;
  void Finalize();

private:
  std::shared_ptr<Shader::Pipeline> pipeline = nullptr;
  Mesh::PrimitiveBuffer primitiveBuffer;
  Texture::Sampler sampler;

  std::shared_ptr<Texture::Image2D> texGround = nullptr;
  std::shared_ptr<Texture::Image2D> texTree = nullptr;
  std::shared_ptr<Texture::Image2D> texHouse = nullptr;
  std::shared_ptr<Texture::Image2D> texCube = nullptr;

  Shader::PointLight pointLight;
};

#endif // MAINGAMESCENE_H_INCLUDED
