/**
* @file GLContext.cpp
*/
#include "GLContext.h"
#include <vector>
#include <iostream>

/**
* OpenGLコンテキストに関する機能を格納する名前空間.
*/
namespace GLContext {

/**
* バッファオブジェクトを作成する.
*
* @param size データのサイズ.
* @param data データへのポインタ.
*
* @return 作成したバッファオブジェクト.
*/
GLuint CreateBuffer(GLsizeiptr size, const GLvoid* data)
{
  GLuint id = 0;
  glCreateBuffers(1, &id);
  glNamedBufferStorage(id, size, data, 0);
  return id;
}

/**
* Vertex Array Objectを作成する.
*
* @param vboPosition VAOに関連付けられる座標データ.
* @param vboColor    VAOに関連付けられるカラーデータ.
* @param ibo         VAOに関連付けられるインデックスデータ.
*
* @return 作成したVAO.
*/
GLuint CreateVertexArray(GLuint vboPosition, GLuint vboColor, GLuint ibo)
{
  if (!vboPosition || !vboColor) {
    std::cerr << "[エラー]" << __func__ << ":バッファオブジェクトが0です。\n";
    return 0;
  }

  GLuint id = 0;
  glCreateVertexArrays(1, &id);

  const GLuint positionIndex = 0;
  const GLuint positionBindingIndex = 0;
  glEnableVertexArrayAttrib(id, positionIndex);
  glVertexArrayAttribFormat(id, positionIndex, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(id, positionIndex, positionBindingIndex);
  glVertexArrayVertexBuffer(
    id, positionBindingIndex, vboPosition, 0, sizeof(Position));

  const GLuint colorIndex = 1;
  const GLuint colorBindingIndex = 1;
  glEnableVertexArrayAttrib(id, colorIndex);
  glVertexArrayAttribFormat(id, colorIndex, 4, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(id, colorIndex, colorBindingIndex);
  glVertexArrayVertexBuffer(id, colorBindingIndex, vboColor, 0, sizeof(Color));

  glVertexArrayElementBuffer(id, ibo);

  return id;
}

/**
* シェーダー・プログラムをビルドする.
*
* @param type シェーダーの種類.
* @param code シェーダー・プログラムへのポインタ.
*
* @retval 0より大きい 作成したプログラム・オブジェクト.
* @retval 0          プログラム・オブジェクトの作成に失敗.
*/
GLuint CreateProgram(GLenum type, const GLchar* code)
{
  GLuint program = glCreateShaderProgramv(type, 1, &code);

  GLint status = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint infoLen = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen) {
      std::vector<char> buf;
      buf.resize(infoLen);
      if ((int)buf.size() >= infoLen) {
        glGetProgramInfoLog(program, infoLen, nullptr, buf.data());
        std::cerr << "[エラー]" << __func__ <<
          ":シェーダーのビルドに失敗.\n" << buf.data() << "\n";
      }
    }
    glDeleteProgram(program);
    return 0;
  }
  return program;
}

/**
* パイプライン・オブジェクトを作成する.
*
* @param vp  頂点シェーダー・プログラム.
* @param fp  フラグメントシェーダー・プログラム.
*
* @retval 0より大きい 作成したパイプライン・オブジェクト.
* @retval 0         パイプライン・オブジェクトの作成に失敗.
*/
GLuint CreatePipeline(GLuint vp, GLuint fp)
{
  glGetError(); // エラー状態をリセット.

  GLuint id;
  glCreateProgramPipelines(1, &id);
  glUseProgramStages(id, GL_VERTEX_SHADER_BIT, vp);
  glUseProgramStages(id, GL_FRAGMENT_SHADER_BIT, fp);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":プログラムパイプラインの作成に失敗.\n";
    glDeleteProgramPipelines(1, &id);
    return 0;
  }

  GLint testVp = 0;
  glGetProgramPipelineiv(id, GL_VERTEX_SHADER, &testVp);
  if (testVp != vp) {
    std::cerr << "[エラー]" << __func__ << ":頂点シェーダの設定に失敗.\n";
    glDeleteProgramPipelines(1, &id);
    return 0;
  }
  GLint testFp = 0;
  glGetProgramPipelineiv(id, GL_FRAGMENT_SHADER, &testFp);
  if (testFp != fp) {
    std::cerr << "[エラー]" << __func__ << ":フラグメントシェーダの設定に失敗.\n";
    glDeleteProgramPipelines(1, &id);
    return 0;
  }
  return id;
}

} // namespace GLContext