/**
* @file ProgramPipeline.cpp
*/
#include "ProgramPipeline.h"
#include "GLContext.h"
#include <iostream>

//#define USE_PROGRAM_OBJECT

/**
* コンストラクタ.
*
* @param vsCode  頂点シェーダファイル名.
* @param fsCode  フラグメントシェーダファイル名.
*/
ProgramPipeline::ProgramPipeline(const char* vsCode, const char* fsCode)
{
#ifdef USE_PROGRAM_OBJECT
  vp = fp = GLContext::CreateProgramFromFile(vsCode, fsCode);
  if (vp) {
    std::cout << "[情報] プログラムパイプラインを作成(id=" << pipeline <<
      ", vp=" << vsCode << ", fp=" << fsCode << ")\n";
  }
#else
  vp = GLContext::CreateProgramFromFile(GL_VERTEX_SHADER, vsCode);
  fp = GLContext::CreateProgramFromFile(GL_FRAGMENT_SHADER, fsCode);
  pipeline = GLContext::CreatePipeline(vp, fp);
  if (!pipeline) {
    std::cout << "[情報] プログラムパイプラインを作成(id=" << pipeline <<
      ", vp=" << vsCode << ", fp=" << fsCode << ")\n";
  }
#endif
}

/**
* デストラクタ.
*/
ProgramPipeline::~ProgramPipeline()
{
#ifdef USE_PROGRAM_OBJECT
  if (vp) {
    std::cout << "[情報]プログラムパイプラインを削除(id=" << pipeline << ")\n";
  }
  glDeleteProgram(vp);
#else
  if (pipeline) {
    std::cout << "[情報]プログラムパイプラインを削除(id=" << pipeline << ")\n";
  }
  glDeleteProgramPipelines(1, &pipeline);
  glDeleteProgram(fp);
  glDeleteProgram(vp);
#endif
}

/**
* オブジェクトが使える状態かどうかを調べる.
*
* @retval true  使える.
* @retval false 使えない(初期化に失敗している).
*/
bool ProgramPipeline::IsValid() const
{
#ifdef USE_PROGRAM_OBJECT
  return vp;
#else
  return pipeline;
#endif
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

  // ロケーション番号によってコピー先を変更する
  // - 0～99: 頂点シェーダ
  // - 100～: フラグメントシェーダ
  GLuint program = vp;
  if (location >= 100) {
    program = fp;
  }

  glProgramUniformMatrix4fv(program, location, 1, GL_FALSE, &data[0][0]);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":ユニフォーム変数の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* ユニフォーム変数にデータをコピーする
*/
bool ProgramPipeline::SetUniform(GLint location, const glm::vec3& data) const
{
  glGetError(); // エラー状態をリセット.

  const GLuint program = GetProgram(location);
  glProgramUniform3fv(program, location, 1, &data.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":ユニフォーム変数の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* ユニフォーム変数にデータをコピーする
*/
bool ProgramPipeline::SetUniform(GLint location, const glm::vec4& data) const
{
  glGetError(); // エラー状態をリセット.

  const GLuint program = GetProgram(location);
  glProgramUniform4fv(program, location, 1, &data.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":ユニフォーム変数の設定に失敗.\n";
    return false;
  }
  return true;
}

// TODO: テキスト未追加
/**
* ユニフォーム変数にデータをコピーする
*/
bool ProgramPipeline::SetUniform(
  GLint location, const glm::uint* data, size_t size) const
{
  glGetError(); // エラー状態をリセット

  const GLuint program = GetProgram(location);
  glProgramUniform1uiv(program, location, static_cast<GLsizei>(size), data);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":ユニフォーム変数の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* ユニフォーム変数にデータをコピーする
*/
bool ProgramPipeline::SetUniform(
  GLint location, const glm::vec4* data, size_t size) const
{
  glGetError(); // エラー状態をリセット

  const GLuint program = GetProgram(location);
  glProgramUniform4fv(program, location, static_cast<GLsizei>(size), &data->x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":ユニフォーム変数の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* ユニフォーム変数にデータをコピーする
*/
bool ProgramPipeline::SetUniform(GLint location, const glm::mat4* data,
  size_t count) const
{
  glGetError(); // エラー状態をリセット

  const GLuint program = GetProgram(location);
  glProgramUniformMatrix4fv(program, location, static_cast<GLsizei>(count),
    GL_FALSE, &data[0][0][0]);
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
#ifdef USE_PROGRAM_OBJECT
  glUseProgram(vp);
#else
  glBindProgramPipeline(pipeline);
#endif
}

/**
* プログラムパイプラインのバインドを解除する.
*/
void ProgramPipeline::Unbind() const
{
#ifdef USE_PROGRAM_OBJECT
  glUseProgram(0);
#else
  glBindProgramPipeline(0);
#endif
}

/**
* ロケーション番号に対応するプログラムIDを取得する
*/
GLuint ProgramPipeline::GetProgram(GLint location) const
{
  // ロケーション番号によってコピー先を変更する
  // - 0～99: 頂点シェーダ
  // - 100～: フラグメントシェーダ
  if (location >= 100) {
    return fp;
  }
  return vp;
}

