/**
* @file Main.cpp
*/
#include <glad/glad.h>
#include "GLContext.h"
#include "Mesh.h"
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
  {-0.87f, 1.5f, 0.50f},
  { 0.87f, 1.5f, 0.50f},
  { 0.00f, 4.0f, 0.00f},
  { 0.00f, 0.0f,-0.36f},
  {-0.31f, 0.0f, 0.18f},
  { 0.31f, 0.0f, 0.18f},

  // 家
  { 2.8f, 0.0f, 3.0f}, 
  { 3.0f, 4.0f, 3.0f}, 
  { 0.0f, 6.0f, 3.0f}, 
  {-3.0f, 4.0f, 3.0f}, 
  {-2.8f, 0.0f, 3.0f}, 

  {-2.8f, 0.0f,-3.0f}, 
  {-3.0f, 4.0f,-3.0f}, 
  { 0.0f, 6.0f,-3.0f}, 
  { 3.0f, 4.0f,-3.0f}, 
  { 2.8f, 0.0f,-3.0f}, 

  { 3.0f, 4.0f, 3.0f}, 
  { 0.0f, 6.0f, 3.0f}, 
  {-3.0f, 4.0f, 3.0f}, 
  {-3.0f, 4.0f,-3.0f}, 
  { 0.0f, 6.0f,-3.0f}, 
  { 3.0f, 4.0f,-3.0f}, 

  {-0.3f, -0.3f, 0.5f},
  { 0.2f, -0.3f, 0.5f},
  { 0.2f,  0.5f, 0.5f},
  {-0.3f,  0.5f, 0.5f},

  {-0.2f, -0.5f, 0.1f},
  { 0.3f, -0.5f, 0.1f},
  { 0.3f,  0.3f, 0.1f},
  { 0.3f,  0.3f, 0.1f},
  {-0.2f,  0.3f, 0.1f},
  {-0.2f, -0.5f, 0.1f},

  {-0.33f, 0.5f, 0.5f},
  { 0.33f, 0.5f, 0.5f},
  { 0.0f,  -0.5f, 0.5f},

  {-0.33f - 0.5f, -0.5f, 0.5f},
  { 0.33f - 0.5f, -0.5f, 0.5f},
  { 0.0f - 0.5f,  0.5f, 0.5f},

  {-0.33f + 0.5f, -0.5f, 0.5f},
  { 0.33f + 0.5f, -0.5f, 0.5f},
  { 0.0f + 0.5f,  0.5f, 0.5f},
};

/// 色データ.
const Color colors[] = {
  // 地面
  {0.8f, 0.7f, 0.5f},
  {0.8f, 0.7f, 0.5f},
  {0.8f, 0.7f, 0.5f},
  {0.8f, 0.7f, 0.5f},

  // 木
  {0.5f, 0.8f, 0.3f, 1.0f},
  {0.1f, 0.3f, 0.0f, 1.0f},
  {0.1f, 0.3f, 0.0f, 1.0f},
  {0.1f, 0.3f, 0.0f, 1.0f},
  {0.2f, 0.1f, 0.0f, 1.0f},
  {0.5f, 0.3f, 0.2f, 1.0f},
  {0.5f, 0.3f, 0.2f, 1.0f},
  {0.5f, 0.3f, 0.2f, 1.0f},

  // 家
  {0.4f, 0.3f, 0.2f, 1.0f},
  {0.6f, 0.5f, 0.3f, 1.0f},
  {0.5f, 0.4f, 0.2f, 1.0f},
  {0.6f, 0.5f, 0.3f, 1.0f},
  {0.4f, 0.3f, 0.2f, 1.0f},

  {0.4f, 0.3f, 0.2f, 1.0f},
  {0.6f, 0.5f, 0.3f, 1.0f},
  {0.5f, 0.4f, 0.2f, 1.0f},
  {0.6f, 0.5f, 0.3f, 1.0f},
  {0.4f, 0.3f, 0.2f, 1.0f},

  {0.2f, 0.1f, 0.1f, 1.0f},
  {0.3f, 0.2f, 0.2f, 1.0f},
  {0.2f, 0.1f, 0.1f, 1.0f},
  {0.2f, 0.1f, 0.1f, 1.0f},
  {0.3f, 0.2f, 0.2f, 1.0f},
  {0.2f, 0.1f, 0.1f, 1.0f},


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

  {0.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 0.0f, 1.0f},
  {1.0f, 0.0f, 1.0f, 1.0f},

  {0.0f, 0.0f, 1.0f, 1.0f},
  {0.0f, 1.0f, 0.0f, 1.0f},
  {1.0f, 0.0f, 0.0f, 1.0f},

  {0.5f, 0.5f, 0.5f, 1.0f},
  {0.0f, 0.0f, 0.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
};

/// テクスチャ座標データ.
const glm::vec2 texcoords[] = {
  // 地面
  {-4.0f,-4.0f },
  { 4.0f,-4.0f },
  { 4.0f, 4.0f },
  {-4.0f, 4.0f },

  // 木
  { 0.0f, 1.0f },
  { 0.0f, 0.0f },
  { 0.5f, 0.0f },
  { 1.0f, 0.0f },
  { 0.0f, 1.0f },
  { 0.0f, 0.0f },
  { 0.5f, 0.0f },
  { 1.0f, 0.0f },

  // 家
  { 1.000f, 1.00f },
  { 1.000f, 0.31f },
  { 0.875f, 0.00f },
  { 0.750f, 0.31f },
  { 0.750f, 1.00f },

  { 0.500f, 1.00f },
  { 0.500f, 0.31f },
  { 0.375f, 0.00f },
  { 0.250f, 0.31f },
  { 0.250f, 1.00f },

  { 0.750f, 0.31f },
  { 0.750f, 0.00f },
  { 0.750f, 0.31f },
  { 0.500f, 0.31f },
  { 0.500f, 0.00f },
  { 0.500f, 0.31f },
};

/// インデックスデータ.
const GLushort indices[] = {
  // 地面
  0, 1, 2, 2, 3, 0,

  // 木
  0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 2, 3,
  4, 5, 6, 4, 6, 7, 4, 7, 5,

  // 家
  0, 1, 3, 3, 4, 0, 1, 2, 3,
  5, 6, 8, 8, 9, 5, 6, 7, 8,
  9, 8, 1, 1, 0, 9,
  4, 3, 6, 6, 5, 4,
  15, 14, 11, 11, 10, 15,
  12, 11, 14, 14, 13, 12,

  0, 1, 2, 2, 3, 0,
  4, 5, 6, 7, 8, 9,
};

/// 描画データリスト.
const Primitive primGround(GL_TRIANGLES, 6, 0, 0); // 地面
const Primitive primTree(GL_TRIANGLES, 21, 6 * sizeof(GLushort), 4); // 木
const Primitive primHouse(GL_TRIANGLES, 42, 27 * sizeof(GLushort), 12);

// 画像データ.
const int imageWidth = 8; // 画像の幅.
const int imageHeight = 8; // 画像の高さ.
const GLuint B = 0xff'10'10'10; // 黒.
const GLuint G = 0xff'a0'a0'a0; // 黒.
const GLuint W = 0xff'ff'ff'ff; // 白.
const GLuint imageGround[imageWidth * imageHeight] = {
  W, W, B, W, W, W, W, W,
  W, B, W, W, W, W, W, W,
  W, W, B, W, W, W, W, W,
  B, B, B, B, B, B, B, B,
  W, W, W, W, W, W, B, W,
  W, W, W, W, W, B, W, W,
  W, W, W, W, W, W, B, W,
  B, B, B, B, B, B, B, B,
};
const GLuint imageTree[] = {
  G, B, G, B, G, B, G, B,
  W, G, W, G, W, G, W, G,
  W, W, W, W, W, W, W, W,
  G, G, G, G, G, G, G, G,
  G, W, G, W, G, W, G, W,
  W, W, W, W, W, W, W, W,
  G, G, G, G, G, G, G, G,
  W, G, W, G, W, G, W, G,
};

/// 頂点シェーダー.
static const GLchar* const vsCode =
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
  "}";

/// フラグメントシェーダー.
static const GLchar* const fsCode =
  "#version 450 \n"
  "layout(location=0) in vec4 inColor; \n"
  "layout(location=1) in vec2 inTexcoord; \n"
  "layout(binding=0) uniform sampler2D texColor; \n"
  "out vec4 fragColor; \n"
  "void main() { \n"
//  "  fragColor = inColor * texture(texColor, gl_FragCoord.xy * 0.01); \n"
  "  fragColor = inColor * texture(texColor, inTexcoord); \n"
  "}";

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
*
* 詳細は(https://www.khronos.org/opengl/wiki/Debug_Output)を参照.
*/
void GLAPIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
  if (length < 0) {
    std::cerr << "[MSG] " << message << "\n";
  } else {
    const std::string s(message, message + length);
    std::cerr << "[MSG] " << s << "\n";
  }
}

/**
* 並木を描画する.
*/
void DrawLineOfTrees(GLint vp, GLint locMatMVP, const glm::mat4& matVP, const glm::vec3& start, const glm::vec3& direction)
{
  glm::vec3 offset = start;
  for (float i = 0; i < 19; ++i) {
    const glm::vec3 offset = start + direction * i;
    const glm::mat4 matModelT = glm::translate(glm::mat4(1), offset);
    const glm::mat4 matModelR = glm::rotate(glm::mat4(1), glm::radians(i * 90), glm::vec3(0, 1, 0));
    const glm::mat4 matModel = matModelT * matModelR;
    const glm::mat4 matMVP = matVP * matModel;
    glProgramUniformMatrix4fv(vp, locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
    primTree.Draw();
  }
}

/**
* GLFWからのエラー報告を処理する.
*
* @param error エラー番号.
* @param desc  エラーの内容.
*/
void ErrorCallback(int error, const char* desc)
{
  std::cerr << "ERROR: " << desc << "\n";
}

/**
* エントリーポイント.
*/
int main()
{
  glfwSetErrorCallback(ErrorCallback);

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
    glfwCreateWindow(1280, 720, "OpenGL3DActionGame", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);

  // OpenGL関数のアドレスを取得する.
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::cerr << "ERROR: GLADの初期化に失敗しました." << std::endl;
    glfwTerminate();
    return 1;
  }

  glDebugMessageCallback(DebugCallback, nullptr);

  // OpenGLの情報をコンソールウィンドウへ出力する.
#define Output(name) {\
  GLint n = 0;\
  glGetIntegerv(name, &n);\
  std::cout << #name " = " << n << "\n";\
} (void)0
  const GLubyte* renderer = glGetString(GL_RENDERER);
  std::cout << "Renderer: " << renderer << "\n";
  const GLubyte* version = glGetString(GL_VERSION);
  std::cout << "OpenGL Version: " << version << "\n";
  Output(GL_MAX_VERTEX_ATTRIB_BINDINGS); // 少なくとも16.
  Output(GL_MAX_VERTEX_ATTRIBS); // 少なくとも16.
  Output(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET); // 少なくとも2047
  Output(GL_MAX_VERTEX_ATTRIB_STRIDE); // 少なくとも2048
  Output(GL_MAX_UNIFORM_LOCATIONS); // 少なくとも1024
  Output(GL_MAX_UNIFORM_BLOCK_SIZE); // 少なくとも16k
  Output(GL_MAX_VARYING_COMPONENTS); // 少なくとも60
  Output(GL_MAX_VARYING_VECTORS); // 少なくとも15
  Output(GL_MAX_VARYING_FLOATS); // 少なくとも32
  Output(GL_MAX_VERTEX_ATTRIBS); // 少なくとも16
  Output(GL_MAX_TEXTURE_SIZE); // 少なくとも1024
  Output(GL_MAX_TEXTURE_IMAGE_UNITS); // 少なくとも16
  Output(GL_MAX_TEXTURE_BUFFER_SIZE); // 少なくとも64k
#undef Output

  // VAOを作成する.
  const GLuint positionBuffer = GLContext::CreateBuffer(sizeof(positions), positions);
  const GLuint colorBuffer = GLContext::CreateBuffer(sizeof(colors), colors);
  const GLuint texcoordBuffer = GLContext::CreateBuffer(sizeof(texcoords), texcoords);
  const GLuint ibo = GLContext::CreateBuffer(sizeof(indices), indices);
  const GLuint vao = GLContext::CreateVertexArray(positionBuffer, colorBuffer, texcoordBuffer, ibo);
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
  const GLuint sampler = GLContext::CreateSampler();
  if (!sampler) {
    return 1;
  }
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
  glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  //const GLuint texGround = GLContext::CreateImage2D(imageWidth, imageHeight, imageGround);
  const GLuint texGround = GLContext::CreateImage2D("Res/Ground.tga");
  if (!texGround) {
    return 1;
  }
  const GLuint texTree = GLContext::CreateImage2D(imageWidth, imageHeight, imageTree, GL_RGBA);

  // メインループ.
  while (!glfwWindowShouldClose(window)) {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 視点を回転させる.
    const float degree = static_cast<float>(std::fmod(glfwGetTime() * 30.0, 360.0));
    const glm::mat4 matViewRot = glm::rotate(glm::mat4(1), glm::radians(degree), glm::vec3(0, 1, 0));
    const glm::vec3 viewPosition = matViewRot * glm::vec4(20, 30, 20, 1);

    // 座標変換行列を作成してシェーダーに転送する.
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    const glm::mat4 matProj =
      glm::perspective(glm::radians(45.0f),  aspectRatio, 0.1f, 500.0f);
    const glm::mat4 matView =
      glm::lookAt(viewPosition, glm::vec3(0), glm::vec3(0, 1, 0));
    //const glm::mat4 matModel = glm::translate(glm::mat4(1), glm::vec3(5, 0, 1));
    //const glm::mat4 matMVP = matProj * matView * matModel;
    //glProgramUniformMatrix4fv(vp, locMatMVP, 1, GL_FALSE, &matMVP[0][0]);

    glBindProgramPipeline(pipeline);
    glBindVertexArray(vao);

    //primTree.Draw();

    // 木を描画.
#if 1
    glBindTextureUnit(0, texTree);
    for (float j = 0; j < 4; ++j) {
      const glm::mat4 matRot = glm::rotate(glm::mat4(1), glm::radians(90.0f) * j, glm::vec3(0, 1, 0));
      DrawLineOfTrees(vp, locMatMVP, matProj * matView, matRot * glm::vec4(-19, 0, 19, 1), matRot * glm::vec4(2, 0, 0, 1));
    }
#else
    for (float i = 0; i < 19; ++i) {
      const glm::mat4 matModelT =
        glm::translate(glm::mat4(1), glm::vec3(-19 + i * 2, 0, 19));
      const glm::mat4 matModelR =
        glm::rotate(glm::mat4(1), glm::radians(i * 90), glm::vec3(0, 1, 0));
      const glm::mat4 matModel = matModelT * matModelR;
      const glm::mat4 matMVP = matVP * matModel;
      glProgramUniformMatrix4fv(vp, locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
      primTree.Draw();
    }
#endif

    // 地面を描画.
    {
      const glm::mat4 matModel = glm::mat4(1);
      const glm::mat4 matMVP = matProj * matView * matModel;
      glProgramUniformMatrix4fv(vp, locMatMVP, 1, GL_FALSE, &matMVP[0][0]);
      glBindTextureUnit(0, texGround);
      glBindSampler(0, sampler);
      primGround.Draw();
    }
    primHouse.Draw();

    glBindTextureUnit(0, 0);
    glBindSampler(0, 0);

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
  glDeleteBuffers(1, &colorBuffer);
  glDeleteBuffers(1, &positionBuffer);
  glDeleteBuffers(1, &texcoordBuffer);

  // GLFWの終了.
  glfwTerminate();

  return 0;
}