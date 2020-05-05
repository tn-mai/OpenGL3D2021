/**
* @file Shader.cpp
*/
#include "Shader.h"
#include "GLContext.h"

/**
* シェーダに関する機能を格納する名前空間.
*/
namespace Shader {

/**
* コンストラクタ.
*
* @param vsCode  頂点シェーダー・プログラムのアドレス.
* @param fsCode  フラグメントシェーダー・プログラムのアドレス.
*/
Pipeline::Pipeline(const char* vsFilename, const char* fsFilename)
{
  vp = GLContext::CreateProgramFromFile(GL_VERTEX_SHADER, vsFilename);
  fp = GLContext::CreateProgramFromFile(GL_FRAGMENT_SHADER, fsFilename);
  id = GLContext::CreatePipeline(vp, fp);
}

/**
* デストラクタ.
*/
Pipeline::~Pipeline()
{
  glDeleteProgramPipelines(1, &id);
  glDeleteProgram(fp);
  glDeleteProgram(vp);
}

/**
* プログラムパイプラインをバインドする.
*/
void Pipeline::Bind() const
{
  glBindProgramPipeline(id);
}

/**
* プログラムパイプラインのバインドを解除する.
*/
void Pipeline::Unbind() const
{
  glBindProgramPipeline(0);
}

/**
* シェーダにMVP行列を設定する.
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

