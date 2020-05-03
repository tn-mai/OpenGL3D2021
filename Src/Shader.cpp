/**
* @file Shader.cpp
*/
#include "Shader.h"
#include "GLContext.h"

namespace Shader {

/**
*
*/
Pipeline::Pipeline(const char* vsCode, const char* fsCode)
{
  vp = GLContext::CreateProgram(GL_VERTEX_SHADER, vsCode);
  fp = GLContext::CreateProgram(GL_FRAGMENT_SHADER, fsCode);
  id = GLContext::CreatePipeline(vp, fp);
}

/**
*
*/
Pipeline::~Pipeline()
{
  glDeleteProgramPipelines(1, &id);
  glDeleteProgram(fp);
  glDeleteProgram(vp);
}

/**
*
*/
void Pipeline::Bind() const
{
  glBindProgramPipeline(id);
}

/**
*
*/
void Pipeline::SetMVP(const glm::mat4& matMVP)
{
  const GLint locMatMVP = 0;
  glProgramUniformMatrix4fv(vp, locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
}

/**
* プログラムパイプラインのバインドを解除する.
*/
void UnbindPipeline()
{
  glBindProgramPipeline(0);
}

} // namespace Shader

