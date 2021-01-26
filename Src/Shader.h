/**
* @file Shader.h
*/
#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Shader {

/**
* 平行光源.
*/
struct DirectionalLight
{
  glm::vec4 direction; // 向き.
  glm::vec4 color;     // 色.
};

/**
* 点光源
*/
struct PointLight {
  glm::vec4 position; // 位置.
  glm::vec4 color;    // 色.
};

/**
* プログラム・パイプライン.
*/
class Pipeline
{
public:
  Pipeline() = default;
  Pipeline(const char* vs, const char* fs);
  ~Pipeline();
  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  explicit operator bool() const { return id; }

  void Bind() const;
  void Unbind() const;
  bool SetMVP(const glm::mat4&) const;
  bool SetModelMatrix(const glm::mat4&) const;
  bool SetViewPosition(const glm::vec3& p) const;
  bool SetLight(const DirectionalLight& light) const;
  bool SetLight(const PointLight& light) const;
  bool SetAmbientLight(const glm::vec3& color) const;
  bool SetObjectColor(const glm::vec4&) const;
  bool SetMorphWeight(const glm::vec3& weight) const;

private:
  GLuint id = 0;
  GLuint vp = 0;
  GLuint fp = 0;

  GLuint lightingProgram = 0; // 光源データ転送先のプログラムID.
};

void UnbindPipeline();

} // namespace Shader

#endif // SHADER_H_INCLUDED

