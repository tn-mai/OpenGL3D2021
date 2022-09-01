/**
* @file ProgramPipeline.h
*/
#ifndef PROGRAMPIPELINE_H_INCLUDED
#define PROGRAMPIPELINE_H_INCLUDED
#include <glad/glad.h>
#include <glm/glm.hpp>

/**
* シェーダープログラムを管理するクラス.
*/
class ProgramPipeline
{
public:
  ProgramPipeline(const char* vsCode, const char* fsCode);
  ~ProgramPipeline();

  // オブジェクトの有効性を判定する
  bool IsValid() const;

  // ユニフォーム変数の設定
  bool SetUniform(GLint, const glm::mat4&) const;
  bool SetUniform(GLint, const glm::vec3&) const;
  bool SetUniform(GLint, const glm::vec4&) const;

  // TODO: テキスト未追加
  bool SetUniform(GLint, const glm::vec4*, size_t) const;
  bool SetUniform(GLint, const glm::uint*, size_t) const;
  bool SetUniform(GLint, const glm::mat4*, size_t) const;

  // バインド管理
  void Bind() const;
  void Unbind() const;

private:
  GLuint GetProgram(GLint location) const;

  GLuint vp = 0;       // プログラム・オブジェクト(頂点シェーダ)
  GLuint fp = 0;       // プログラム・オブジェクト(フラグメントシェーダ)
  GLuint pipeline = 0; // プログラム・パイプライン・オブジェクト
};

#endif // PROGRAMPIPELINE_H_INCLUDED

