/**
* @file Main.cpp
*/
#include <glad/glad.h>
#include "GLContext.h"
#include "Primitive.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#pragma comment(lib, "opengl32.lib")

/// 座標データ.
const Position positions[] = {
  // 地面
  {-20, 0, 20},
  { 20, 0, 20},
  { 20, 0,-20},
  {-20, 0,-20},

  // 木
  { 0.00f, 5.0f, 0.00f},
  { 0.00f, 1.5f,-1.00f},
  {-1.00f, 1.5f, 0.00f},
  { 0.00f, 1.5f, 1.00f},
  { 1.00f, 1.5f, 0.00f},
  { 0.00f, 4.0f, 0.00f},
  { 0.00f, 0.0f,-0.36f},
  {-0.31f, 0.0f, 0.18f},
  { 0.31f, 0.0f, 0.18f},

  // 建物
  {-2.0f, 0.0f, 3.0f },
  { 2.0f, 0.0f, 3.0f },
  { 2.0f, 0.0f,-3.0f },
  {-2.0f, 0.0f,-3.0f },
  {-2.0f, 0.0f, 3.0f },
  {-2.0f, 2.0f, 3.0f },
  { 2.0f, 2.0f, 3.0f },
  { 2.0f, 2.0f,-3.0f },
  {-2.0f, 2.0f,-3.0f },
  {-2.0f, 2.0f, 3.0f },
  { 2.0f, 2.0f,-3.0f },
  {-2.0f, 2.0f,-3.0f },

  {-0.3f, -0.3f, 0.1f},
  { 0.2f, -0.3f, 0.1f},
  { 0.2f,  0.5f, 0.1f},
  {-0.3f,  0.5f, 0.1f},

  {-0.2f, -0.5f, 0.5f},
  { 0.3f, -0.5f, 0.5f},
  { 0.3f,  0.3f, 0.5f},
  { 0.3f,  0.3f, 0.5f},
  {-0.2f,  0.3f, 0.5f},
  {-0.2f, -0.5f, 0.5f},

  {-0.33f, 0.5f, 0.5f },
  { 0.33f, 0.5f, 0.5f },
  { 0.0f,  -0.5f, 0.5f },
};

/// 色データ.
const Color colors[] = {
  // 地面
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},

  // 木
  {0.5f, 0.8f, 0.3f, 1.0f},
  {0.1f, 0.3f, 0.0f, 1.0f},
  {0.1f, 0.3f, 0.0f, 1.0f},
  {0.1f, 0.3f, 0.0f, 1.0f},
  {0.1f, 0.3f, 0.0f, 1.0f},
  {0.2f, 0.1f, 0.0f, 1.0f},
  {0.5f, 0.3f, 0.2f, 1.0f},
  {0.5f, 0.3f, 0.2f, 1.0f},
  {0.5f, 0.3f, 0.2f, 1.0f},

  // 建物
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},

  {0.0f, 1.0f, 0.0f, 1.0f},
  {0.0f, 0.0f, 1.0f, 1.0f},
  {1.0f, 0.0f, 0.0f, 1.0f},
  {0.0f, 0.0f, 1.0f, 1.0f},

  {1.0f, 0.0f, 0.0f, 1.0f},
  {1.0f, 1.0f, 0.0f, 1.0f},
  {1.0f, 0.0f, 0.0f, 1.0f},
  {0.0f, 0.0f, 1.0f, 1.0f},
  {0.0f, 1.0f, 1.0f, 1.0f},
  {0.0f, 0.0f, 1.0f, 1.0f},

  {0.0f, 1.0f, 1.0f, 1.0f}, // 水色
  {1.0f, 1.0f, 0.0f, 1.0f}, // 黄色
  {1.0f, 0.0f, 1.0f, 1.0f}, // 紫色
};

/// テクスチャ座標データ.
const glm::vec2 texcoords[] = {
  // 地面
  {-4.0f,-4.0f},
  { 4.0f,-4.0f},
  { 4.0f, 4.0f},
  {-4.0f, 4.0f},

  // 木
  { 0.0f, 1.0f},
  { 0.0f, 0.0f},
  { 0.3f, 0.0f},
  { 0.7f, 0.0f},
  { 1.0f, 0.0f},
  { 0.0f, 1.0f},
  { 0.0f, 0.0f},
  { 0.5f, 0.0f},
  { 1.0f, 0.0f},

  // 建物
  { 0.00f, 0.0f },
  { 0.25f, 0.0f },
  { 0.50f, 0.0f },
  { 0.75f, 0.0f },
  { 1.00f, 0.0f },
  { 0.00f, 0.5f },
  { 0.25f, 0.5f },
  { 0.50f, 0.5f },
  { 0.75f, 0.5f },
  { 1.00f, 0.5f },
  { 0.25f, 1.0f },
  { 0.00f, 1.0f },
};

// インデックスデータ.
const GLushort indices[] = {
  // 地面
  0, 1, 2, 2, 3, 0,

  // 木
  0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1, 1, 4, 3, 3, 2, 1,
  5, 6, 7, 5, 7, 8, 5, 8, 6,

  // 建物
  0, 1, 6, 6, 5, 0,
  1, 2, 7, 7, 6, 1,
  2, 3, 8, 8, 7, 2,
  3, 4, 9, 9, 8, 3,
  5, 6,10,10,11, 5,

  0, 1, 2, 2, 3, 0,
  4, 5, 6, 7, 8, 9,
};

// 描画データ.
const Primitive primGround(GL_TRIANGLES, 6, 0, 0); // 地面
const Primitive primTree(GL_TRIANGLES, 27, 6 * sizeof(GLushort), 4); // 木
const Primitive primBuilding(GL_TRIANGLES, 30, 33 * sizeof(GLushort), 13); // 建物
 
 // 画像データ.
const int imageWidth = 8; // 画像の幅.
const int imageHeight = 8; // 画像の高さ.
const GLuint X = 0xff'18'18'18; // 黒.
const GLuint W = 0xff'ff'ff'ff; // 白.
const GLuint R = 0xff'10'10'e0; // 赤.
const GLuint B = 0xff'e0'10'10; // 青.
const GLuint imageGround[imageWidth * imageHeight] = {
  X, B, B, B, X, W, W, W,
  X, B, B, B, X, W, W, W,
  X, B, B, B, X, W, W, W,
  X, X, X, X, X, X, X, X,
  W, W, X, R, R, R, X, W,
  W, W, X, R, R, R, X, W,
  W, W, X, R, R, R, X, W,
  X, X, X, X, X, X, X, X,
};

const GLuint G = 0xff'10'c0'10;
const GLuint imageTree[5 * 5] = {
  G, G, G, G, G,
  G, G, G, G, G,
  G, G, G, G, G,
  G, G, G, G, G,
  G, G, G, G, G,
};

/// 頂点シェーダー.
static const char* vsCode =
  "#version 450 \n"
  "layout(location=0) in vec3 vPosition; \n"
  "layout(location=1) in vec4 vColor; \n"
  "layout(location=2) in vec2 vTexcoord; \n"
  "layout(location=0) out vec4 outColor; \n"
  "layout(location=1) out vec2 outTexcoord; \n"
  "out gl_PerVertex { \n"
  "  vec4 gl_Position; \n"
  "}; \n"
  "layout(location=0) uniform mat4 matMVP; \n"
  "void main() { \n"
  "  outColor = vColor; \n"
  "  outTexcoord = vTexcoord; \n"
  "  gl_Position = matMVP * vec4(vPosition, 1.0); \n"
  "} \n";

/// フラグメントシェーダー.
static const GLchar* fsCode =
  "#version 450 \n"
  "layout(location=0) in vec4 inColor; \n"
  "layout(location=1) in vec2 inTexcoord; \n"
  "out vec4 fragColor; \n"
  "layout(binding=0) uniform sampler2D texColor; \n"
  "void main() { \n"
  "  fragColor = inColor * texture(texColor, inTexcoord); \n"
  "} \n";

/**
* OpenGLからのメッセージを処理する.
*
* @param source    メッセージの発信者(OpenGL、Windows、シェーダーなど).
* @param type      メッセージの種類(エラー、警告など).
* @param id        メッセージを一位に識別する値.
* @param severity  メッセージの重要度(高、中、低、最低).
* @param length    メッセージの文字数. 負数ならメッセージは0終端されている.
* @param message   メッセージ本体.
* @param userParam コールバック設定時に指定したポインタ.
*/
void GLAPIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
  if (length < 0) {
    std::cerr << message << "\n";
  } else {
    const std::string s(message, message + length);
    std::cerr << s << "\n";
  }
}

/**
* エントリーポイント.
*/
int main()
{
  // GLFWの初期化.
  if (glfwInit() != GLFW_TRUE) {
    return 1;
  }

  // 描画ウィンドウの作成.
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  GLFWwindow* window =
    glfwCreateWindow(1280, 720, "OpenGLGame", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);

  // OpenGL関数のアドレスを取得する.
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    glfwTerminate();
    return 1;
  }

  glDebugMessageCallback(DebugCallback, nullptr);

  // VAOを作成する.
  const GLuint vboPosition = GLContext::CreateBuffer(sizeof(positions), positions);
  const GLuint vboColor = GLContext::CreateBuffer(sizeof(colors), colors);
  const GLuint vboTexcoord = GLContext::CreateBuffer(sizeof(texcoords), texcoords);
  const GLuint ibo = GLContext::CreateBuffer(sizeof(indices), indices);
  const GLuint vao = GLContext::CreateVertexArray(vboPosition, vboColor,
    vboTexcoord, ibo);
  if (!vao) {
    return 1;
  }

  // パイプライン・オブジェクトを作成する.
  const GLuint vp = GLContext::CreateProgram(GL_VERTEX_SHADER, vsCode);
  const GLuint fp = GLContext::CreateProgram(GL_FRAGMENT_SHADER, fsCode);
  const GLuint pipeline = GLContext::CreatePipeline(vp, fp);
  if (!pipeline) {
    return 1;
  }

  // uniform変数の位置.
  const GLint locMatMVP = 0;

  // サンプラ・オブジェクトを作成する.
  const GLuint sampler = GLContext::CreateSampler(GL_REPEAT);
  if (!sampler) {
    return 1;
  }

  const GLuint texBuilding= GLContext::CreateImage2D("Res/Building.tga");
  const GLuint texGround = GLContext::CreateImage2D("Res/Ground.tga");
  const GLuint texTree =
    GLContext::CreateImage2D(5, 5, imageTree, GL_RGBA,
    GL_UNSIGNED_BYTE);
  if (!texGround || !texTree) {
    return 1;
  }

  // メインループ.
  while (!glfwWindowShouldClose(window)) {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao);
    glBindProgramPipeline(pipeline);
    glBindSampler(0, sampler);

    // 座標変換行列を作成してシェーダーに転送する.
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    const glm::mat4 matProj =
      glm::perspective(glm::radians(45.0f),  aspectRatio, 0.1f, 200.0f);
    const glm::mat4 matView =
      glm::lookAt(glm::vec3(0, 20, 20), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    // 地面を描画.
    {
      const glm::mat4 matModel = glm::mat4(1);
      const glm::mat4 matMVP = matProj * matView * matModel;
      glProgramUniformMatrix4fv(vp, locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
      glBindTextureUnit(0, texGround);
      primGround.Draw();
    }

    // 建物を描画.
    {
      const glm::mat4 matModel = glm::translate(glm::mat4(1), glm::vec3(-0, 0, 0));
      const glm::mat4 matMVP = matProj * matView * matModel;
      glProgramUniformMatrix4fv(vp, locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
      glBindTextureUnit(0, texBuilding);
      primBuilding.Draw();
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTextureUnit(0, texTree);

    // 木を描画.
    for (float i = 0; i < 18; ++i) {
      const glm::vec3 position(-17, 0, -17 + i * 2);
      const glm::mat4 matModel =
        glm::translate(glm::mat4(1), position);
      const glm::mat4 matMVP = matProj * matView * matModel;
      glProgramUniformMatrix4fv(vp, locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
 
      primTree.Draw();
    }
    for (float i = 0; i < 18; ++i) {
      const glm::vec3 position(17, 0, -17 + i * 2);
      const glm::mat4 matModel =
        glm::translate(glm::mat4(1), position);
      const glm::mat4 matMVP = matProj * matView * matModel;
      glProgramUniformMatrix4fv(vp, locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
 
      primTree.Draw();
    }
    for (float i = 0; i < 16; ++i) {
      const glm::vec3 position(-15 + i * 2, 0, -17);
      const glm::mat4 matModel =
        glm::translate(glm::mat4(1), position);
      const glm::mat4 matMVP = matProj * matView * matModel;
      glProgramUniformMatrix4fv(vp, locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
 
      primTree.Draw();
    }

    glBindSampler(0, 0);
    glBindProgramPipeline(0);
    glBindVertexArray(0);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  // 後始末.
  glDeleteTextures(1, &texTree);
  glDeleteTextures(1, &texGround);
  glDeleteSamplers(1, &sampler);
  glDeleteProgramPipelines(1, &pipeline);
  glDeleteProgram(fp);
  glDeleteProgram(vp);
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &ibo);
  glDeleteBuffers(1, &vboColor);
  glDeleteBuffers(1, &vboPosition);
  glDeleteBuffers(1, &vboTexcoord);

  // GLFWの終了.
  glfwTerminate();

  return 0;
}