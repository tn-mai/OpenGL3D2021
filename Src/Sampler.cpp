/**
* @file Sampler.cpp
*/
#include "Sampler.h"
#include "GLContext.h"
#include <iostream>

/**
* コンストラクタ.
*/
Sampler::Sampler(GLenum wrapMode)
{
  id = GLContext::CreateSampler(wrapMode);
  if (id) {
    std::cout << "[情報]" << __func__ << "サンプラ" << id << "を作成.\n";
  }
}

/**
* デストラクタ.
*/
Sampler::~Sampler()
{
  if (id) {
    std::cout << "[情報]" << __func__ << "サンプラ" << id << "を削除.\n";
  }
  glDeleteSamplers(1, &id);
}

/**
* オブジェクトが使える状態かどうかを調べる.
*
* @retval true  使える.
* @retval false 使えない(初期化に失敗している).
*/
bool Sampler::IsValid() const
{
   return id;
}

/**
* サンプラをグラフィックスパイプラインに割り当てる.
*
* @param unit 割り当てるテクスチャイメージユニットの番号.
*/
void Sampler::Bind(GLuint unit) const
{
  glBindSampler(unit, id);
}

/**
* サンプラの割り当てを解除する.
*
* @param unit 割り当て解除するテクスチャイメージユニットの番号.
*/
void Sampler::Unbind(GLuint unit) const
{
  glBindSampler(unit, 0);
}

