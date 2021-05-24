/**
* @file ProgramPipeline.cpp
*/
#include "ProgramPipeline.h"
#include "GLContext.h"
#include <iostream>

/**
* コンストラクタ.
*
* @param vsCode  頂点シェーダファイル名.
* @param fsCode  フラグメントシェーダファイル名.
*/
ProgramPipeline::ProgramPipeline(const char* vsCode, const char* fsCode)
{
  vp = GLContext::CreateProgramFromFile(GL_VERTEX_SHADER, vsCode);
  fp = GLContext::CreateProgramFromFile(GL_FRAGMENT_SHADER, fsCode);
  pipeline = GLContext::CreatePipeline(vp, fp);
  if (pipeline) {
    std::cout << "[情報] プログラムパイプラインを作成(id=" << pipeline <<
      ", vp=" << vsCode << ", fp=" << fsCode << ")\n";
  }
}

/**
* デストラクタ.
*/
ProgramPipeline::~ProgramPipeline()
{
  if (pipeline) {
    std::cout << "[情報]プログラムパイプラインを削除(id=" << pipeline << ")\n";
  }
  glDeleteProgramPipelines(1, &pipeline);
  glDeleteProgram(fp);
  glDeleteProgram(vp);
}

/**
* オブジェクトが使える状態かどうかを調べる.
*
* @retval true  使える.
* @retval false 使えない(初期化に失敗している).
*/
bool ProgramPipeline::IsValid() const
{
   return pipeline;
}

/**
* ユニフォーム変数にデータをコピーする.
*
* @param location ユニフォーム変数の位置.
* @param data     ユニフォーム変数にコピーするデータ.
*
* @retval true  コピー成功.
* @retval false コピー失敗.
*/
bool ProgramPipeline::SetUniform(GLint location, const glm::mat4& data) const
{
  glGetError(); // エラー状態をリセット.

  glProgramUniformMatrix4fv(vp, location, 1, GL_FALSE, &data[0][0]);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":ユニフォーム変数の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* プログラムパイプラインをバインドする.
*/
void ProgramPipeline::Bind() const
{
  glBindProgramPipeline(pipeline);
}

/**
* プログラムパイプラインのバインドを解除する.
*/
void ProgramPipeline::Unbind() const
{
  glBindProgramPipeline(0);
}



