/**
* @file FramebufferObject.cpp
*/
#include "FramebufferObject.h"
#include <iostream>

/**
* コンストラクタ
*
* @param w    フレームバッファの幅(ピクセル数)
* @param h    フレームバッファの高さ(ピクセル数)
* @param type FBOの種類
*/
FramebufferObject::FramebufferObject(int w, int h, FboType type)
{
  // カラーテクスチャを作成
  if (static_cast<int>(type) & 1) {
    texColor.reset(new Texture("FBO(Color)", w, h, nullptr, GL_RGBA, GL_HALF_FLOAT));
    if (!texColor || !texColor->GetId()) {
      std::cerr << "[エラー]" << __func__ <<
        ":フレームバッファ用カラーテクスチャの作成に失敗.\n";
      texColor.reset(); // カラーテクスチャを破棄
      return;
    }
  }

  // 深度テクスチャを作成
  if (static_cast<int>(type) & 2) {
    texDepth.reset(new Texture("FBO(Depth)", w, h, nullptr, GL_DEPTH_COMPONENT32F, GL_FLOAT));
    if (!texDepth || !texDepth->GetId()) {
      std::cerr << "[エラー]" << __func__ <<
        ":フレームバッファ用深度テクスチャの作成に失敗.\n";
      texColor.reset(); // カラーテクスチャを破棄
      texDepth.reset(); // 深度テクスチャを破棄
      return;
    }
  }

  // フレームバッファオブジェクトを作成
  glCreateFramebuffers(1, &fbo);
  if (static_cast<int>(type) & 1) {
    glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, texColor->GetId(), 0);
  } else {
    glNamedFramebufferDrawBuffer(fbo, GL_NONE);
  }
  if (static_cast<int>(type) & 2) {
    glNamedFramebufferTexture(fbo, GL_DEPTH_ATTACHMENT, texDepth->GetId(), 0);
  }

  // フレームバッファオブジェクトが作成できたかチェック
  if (glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "[エラー]" << __func__ << ":オフスクリーンバッファの作成に失敗.\n";
    glDeleteFramebuffers(1, &fbo);
    fbo = 0;
    texColor.reset();
    texDepth.reset();
    return;
  }
  width = w;
  height = h;
}

/**
* デストラクタ
*/
FramebufferObject::~FramebufferObject()
{
  glDeleteFramebuffers(1, &fbo);
}

/**
* グラフィックスパイプラインの描画先にFBOを割り当てる
*/
void FramebufferObject::Bind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glViewport(0, 0, width, height);
}

/**
* グラフィックスパイプラインの描画先をデフォルトのフレームバッファに戻す
*/
void FramebufferObject::Unbind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
* カラーテクスチャをグラフィックスパイプラインに割り当てる
*
* @param unit 割り当て先のテクスチャユニット番号
*/
void FramebufferObject::BindColorTexture(GLuint unit) const
{
  texColor->Bind(unit);
}

/**
* カラーテクスチャをグラフィックスパイプラインから取り外す
*
* @param unit 割り当て先のテクスチャユニット番号
*/
void FramebufferObject::UnbindColorTexture(GLuint unit) const
{
  texColor->Unbind(unit);
}

/**
* 深度ステンシルテクスチャをグラフィックスパイプラインに割り当てる
*
* @param unit 割り当て先のテクスチャユニット番号
*/
void FramebufferObject::BindDepthTexture(GLuint unit) const
{
  texDepth->Bind(unit);
}

/**
* 深度ステンシルテクスチャをグラフィックスパイプラインから取り外す
*
* @param unit 割り当て先のテクスチャユニット番号
*/
void FramebufferObject::UnbindDepthTexture(GLuint unit) const
{
  texDepth->Unbind(unit);
}

