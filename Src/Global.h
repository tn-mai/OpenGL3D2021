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
#include <random>

/**
* ゲーム全体で使うデータ.
*/
class Global
{
public:
  static Global& Get();
  bool Initialize(GLFWwindow*);

  enum PrimNo {
    ground,
    tree,
    house,
    cube,
    plane,
    zombie_male_walk_0,
    zombie_male_walk_1,
    zombie_male_walk_2,
    zombie_male_walk_3,
    zombie_male_walk_4,
    zombie_male_walk_5,
    player_stand,
    player_run_0,
    player_run_1,
    player_run_2,
  };
  void Draw(PrimNo) const;

  std::shared_ptr<Shader::Pipeline> pipeline = nullptr;
  std::shared_ptr<Shader::Pipeline> pipelineSimple;
  Mesh::PrimitiveBuffer primitiveBuffer;
  Texture::Sampler sampler;
  GLFWwindow* window = nullptr;

  std::mt19937 random;

private:
  Global() = default;
  ~Global();
  Global(const Global&) = delete;
  Global& operator=(const Global&) = delete;
};


#endif // GLOBAL_H_INCLUDED
