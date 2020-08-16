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
    bullet,
    zombie_male_walk_0,
    zombie_male_walk_1,
    zombie_male_walk_2,
    zombie_male_walk_3,
    zombie_male_walk_4,
    zombie_male_walk_5,
    zombie_male_down_0,
    zombie_male_down_1,
    zombie_male_down_2,
    zombie_male_down_3,
    player_idle_0,
    player_idle_1,
    player_idle_2,
    player_idle_3,
    player_idle_4,
    player_run_0,
    player_run_1,
    player_run_2,
    player_run_3,
    player_run_4,
    player_run_5,
    player_run_6,
    player_run_7,
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
