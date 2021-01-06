/**
* @file Shader.cpp
*/
#include "Shader.h"
#include "GLContext.h"
#include <iostream>

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

  // 光源データ転送先のプログラムIDを設定.
  if (glGetUniformLocation(vp, "directionalLight.direction") >= 0) {
    lightingProgram = vp;
  } else if (glGetUniformLocation(fp, "directionalLight.direction") >= 0) {
    lightingProgram = fp;
  }

  // オブジェクトカラーの初期値を設定.
  if (glGetUniformLocation(vp, "objectColor") >= 0) {
    SetObjectColor(glm::vec4(1));
  }
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
*
* @param matMVP 設定するMVP行列.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetMVP(const glm::mat4& matMVP) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locMatMVP = 0;
  glProgramUniformMatrix4fv(vp, locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":MVP行列の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* シェーダにモデル行列を設定する.
*
* @param matModel 設定する法線行列.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetModelMatrix(const glm::mat4& matModel) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locMatModel = 1;
  glProgramUniformMatrix4fv(vp, locMatModel, 1, GL_FALSE, &matModel[0][0]);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":モデル行列の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* シェーダにライトデータを設定する.
*
* @param light 設定するライトデータ.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetLight(const DirectionalLight& light) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locDirLight = 2;

  glProgramUniform4fv(lightingProgram, locDirLight, 1, &light.direction.x);
  glProgramUniform4fv(lightingProgram, locDirLight + 1, 1, &light.color.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":平行光源の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* シェーダにライトデータを設定する.
*
* @param light 設定するライトデータ.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetLight(const PointLight& light) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locPointLight = 4;

  glProgramUniform4fv(lightingProgram, locPointLight, 1, &light.position.x);
  glProgramUniform4fv(lightingProgram, locPointLight + 1, 1, &light.color.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":点光源の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* シェーダに環境光を設定する.
*
* @param color 設定する環境光.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetAmbientLight(const glm::vec3& color) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locAmbientLight= 6;

  glProgramUniform3fv(lightingProgram, locAmbientLight, 1, &color.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":環境光の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* シェーダにオブジェクトの色を設定する.
*
* @param color 設定する色.
*
* @retval true  設定成功.
* @retval false 設定失敗.
*/
bool Pipeline::SetObjectColor(const glm::vec4& color) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locObjectColor = 8;

  glProgramUniform4fv(vp, locObjectColor, 1, &color.x);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":オブジェクトカラーの設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* モーフィングの合成比率を設定する.
*
* @param weight モーフィングの合成比率(0.0～1.0の範囲で指定すること).
*/
bool Pipeline::SetMorphWeight(float weight) const
{
  glGetError(); // エラー状態をリセット.

  const GLint locMorphRatio = 10;

  glProgramUniform1f(vp, locMorphRatio, weight);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":モーフィング合成比率の設定に失敗.\n";
    return false;
  }
  return true;
}

/**
* プログラムパイプラインのバインドを解除する.
*/
void UnbindPipeline()
{
  glBindProgramPipeline(0);
}

} // namespace Shader

