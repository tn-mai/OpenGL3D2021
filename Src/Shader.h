/**
* @file Shader.h
*/
#ifndef SHADER_H_INCLUDED
#define SHADER_H_INCLUDED
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Shader {

/**
* ���s����.
*/
struct DirectionalLight
{
  glm::vec4 direction; // ����.
  glm::vec4 color;     // �F.
};

/**
* �v���O�����E�p�C�v���C��.
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
  bool SetLight(const DirectionalLight& light) const;

private:
  GLuint id = 0;
  GLuint vp = 0;
  GLuint fp = 0;
};

void UnbindPipeline();

} // namespace Shader

#endif // SHADER_H_INCLUDED
