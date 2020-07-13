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

  bool Initialize(GLFWwindow*);
  void ProcessInput(GLFWwindow*);
  void Update(GLFWwindow*, float deltaTime);
  void Render(GLFWwindow*);
  void Finalize();

private:
  Shader::Pipeline* pipeline = nullptr;
  Mesh::PrimitiveBuffer primitiveBuffer;
  Texture::Sampler sampler;

  Texture::Image2D* texGround = nullptr;
  Texture::Image2D* texTree = nullptr;
  Texture::Image2D* texHouse = nullptr;
  Texture::Image2D* texCube = nullptr;

  Shader::PointLight pointLight;
};

#endif // MAINGAMESCENE_H_INCLUDED
