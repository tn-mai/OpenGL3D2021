/**
* @file Sampler.h
*/
#ifndef SAMPLER_H_INCLUDED
#define SAMPLER_H_INCLUDED
#include <glad/glad.h>

/**
* サンプラを管理するクラス.
*/
class Sampler
{
public:
  Sampler(GLenum wrapMode);
  ~Sampler();

  // オブジェクトの有効性を判定する
  bool IsValid() const;

  // バインド管理
  void Bind(GLuint unit) const;
  void Unbind(GLuint unit) const;

private:
  GLuint id = 0; // オブジェクトID
};

#endif // SAMPLER_H_INCLUDED
