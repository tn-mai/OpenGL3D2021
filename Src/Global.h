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
  bool Initialize(GLFWwindow*);

  enum class PrimitiveId {
    ground,
    tree,
    house,
    cube,
    plane,
  };
  void Draw(PrimitiveId) const;

  std::shared_ptr<Shader::Pipeline> pipeline = nullptr;
  std::shared_ptr<Shader::Pipeline> pipelineSimple;
  Mesh::PrimitiveBuffer primitiveBuffer;
  Texture::Sampler sampler;
  GLFWwindow* window = nullptr;

  int sceneId = 0;

private:
  Global() = default;
  ~Global();
  Global(const Global&) = delete;
  Global& operator=(const Global&) = delete;
};


#endif // GLOBAL_H_INCLUDED
