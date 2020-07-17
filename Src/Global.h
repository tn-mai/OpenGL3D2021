/**
* @file Global.h
*/
#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED
#include "glad/glad.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include <GLFW/glfw3.h>
#include <memory>

/**
* ゲーム全体で使うデータ.
*/
class Global
{
public:
  static Global& Get();
  static bool Initialize(GLFWwindow*);
  static void Finalize();

  std::shared_ptr<Shader::Pipeline> pipeline = nullptr;
  Mesh::PrimitiveBuffer primitiveBuffer;
  Texture::Sampler sampler;
  GLFWwindow* window = nullptr;

private:
  Global() = default;
  ~Global() = default;
  Global(const Global&) = delete;
  Global& operator=(const Global&) = delete;

  static Global* p;
};


#endif // GLOBAL_H_INCLUDED
