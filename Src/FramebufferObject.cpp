/**
* @file FramebufferObject.cpp
*/
#include "FramebufferObject.h"
#include "GameData.h"
#include <iostream>

/**
* コンストラクタ.
*/
FramebufferObject::FramebufferObject(int w, int h, FboType type)
{
  texColor = std::make_shared<Texture::Image2D>("FBO(Color)", w, h, nullptr, GL_RGBA, GL_UNSIGNED_BYTE);
  if (!texColor || !texColor->GetId()) {
    std::cerr << "[エラー]" << __func__ << ":オフスクリーンバッファ用テクスチャの作成に失敗.\n";
    texColor.reset();
    return;
  }

  if (type == FboType::Color) {
    glCreateRenderbuffers(1, &depthStencil);
    glNamedRenderbufferStorage(depthStencil, GL_DEPTH24_STENCIL8, w, h);

    glCreateFramebuffers(1, &fbo);
    glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, texColor->GetId(), 0);
    glNamedFramebufferRenderbuffer(fbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthStencil);
  } else {
    texDepthStencil = std::make_shared<Texture::Image2D>("FBO(DepthStencil)", w, h, nullptr, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
    if (!texDepthStencil || !texDepthStencil->GetId()) {
      std::cerr << "[エラー]" << __func__ << ":オフスクリーンバッファ用テクスチャの作成に失敗.\n";
      texColor.reset();
      texDepthStencil.reset();
      return;
    }

    glCreateFramebuffers(1, &fbo);
    glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, texColor->GetId(), 0);
    glNamedFramebufferTexture(fbo, GL_DEPTH_STENCIL_ATTACHMENT, texDepthStencil->GetId(), 0);
  }

  if (glCheckNamedFramebufferStatus(fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "[エラー]" << __func__ << ":オフスクリーンバッファの作成に失敗.\n";
    glDeleteFramebuffers(1, &fbo);
    fbo = 0;
    texColor.reset();
    texDepthStencil.reset();
    return;
  }

  width = w;
  height = h;
}

/**
* デストラクタ.
*/
FramebufferObject::~FramebufferObject()
{
  glDeleteRenderbuffers(1, &depthStencil);
  glDeleteFramebuffers(1, &fbo);
}

/**
* フレームバッファを描画対象に設定.
*/
void FramebufferObject::Bind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glViewport(0, 0, width, height);
}

/**
* 描画対象をデフォルトフレームバッファに戻す.
*/
void FramebufferObject::Unbind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  int w, h;
  GLFWwindow* window = GameData::Get().window;
  glfwGetWindowSize(window, &w, &h);
  glViewport(0, 0, w, h);
}

/**
* カラーテクスチャをグラフィックスパイプラインに割り当てる.
*
* @param unit 割り当て先のテクスチャユニット番号.
*/
void FramebufferObject::BindColorTexture(GLuint unit) const
{
  texColor->Bind(unit);
}

/**
* カラーテクスチャをグラフィックスパイプラインから取り外す.
*/
void FramebufferObject::UnbindColorTexture() const
{
  texColor->Unbind();
}

/**
* 深度ステンシルテクスチャをグラフィックスパイプラインに割り当てる.
*
* @param unit 割り当て先のテクスチャユニット番号.
*/
void FramebufferObject::BindDepthStencilTexture(GLuint unit) const
{
  texDepthStencil->Bind(unit);
}

/**
* 深度ステンシルテクスチャをグラフィックスパイプラインから取り外す.
*/
void FramebufferObject::UnbindDepthStencilTexture() const
{
  texDepthStencil->Unbind();
}

