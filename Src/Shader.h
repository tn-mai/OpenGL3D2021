/**
* @file Shader.h
*/
#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>

namespace Shader {

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
  void SetMVP(const glm::mat4&);

private:
  GLuint id = 0;
  GLuint vp = 0;
  GLuint fp = 0;
};

void UnbindPipeline();

} // namespace Shader

#endif // SHADER_H_INCLUDED

