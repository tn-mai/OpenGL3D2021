/**
* @file Main.cpp
*/
#include <glad/glad.h>
#include "GLContext.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

#pragma comment(lib, "opengl32.lib")

/// 座標データ: 地面
const glm::vec3 posGround[] = {
  // 地面
  {-20, 0, 20},
  { 20, 0, 20},
  { 20, 0,-20},
  {-20, 0,-20},
};

/// 座標データ: 木
const glm::vec3 posTree[] = {
  { 0.00f, 5.0f, 0.00f},
  { 0.00f, 1.5f,-1.00f},
  {-1.00f, 1.5f, 0.00f},
  { 0.00f, 1.5f, 1.00f},
  { 1.00f, 1.5f, 0.00f},
  { 0.00f, 4.0f, 0.00f},
  { 0.00f, 0.0f,-0.36f},
  {-0.31f, 0.0f, 0.18f},
  { 0.31f, 0.0f, 0.18f},
};

/// 座標データ: 家
const glm::vec3 posHouse[] = {
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

  { 2.8f, 0.0f, 3.0f},
  { 3.0f, 4.0f, 3.0f},

  { 0.0f, 6.0f, 3.0f},
  { 0.0f, 6.0f,-3.0f},

  { 0.0f, 6.0f,-3.0f},
  { 0.0f, 6.0f, 3.0f},
};

/// 座標データ: 立方体
const glm::vec3 posCube[] = {
  { 1,-1, 1},
  {-1,-1, 1},
  {-1, 1, 1},
  {-1,-1, 1},
  { 1,-1, 1},
  { 1, 1, 1},
  { 1,-1, 1},
  { 1,-1,-1},
  { 1, 1,-1},
  { 1,-1,-1},
  {-1,-1,-1},
  {-1, 1,-1},
  {-1,-1,-1},
  { 1,-1,-1},
};

/// 色データ: 地面
const glm::vec4 colGround[] = {
  {0.8f, 0.7f, 0.5f, 1.0f},
  {0.8f, 0.7f, 0.5f, 1.0f},
  {0.8f, 0.7f, 0.5f, 1.0f},
  {0.8f, 0.7f, 0.5f, 1.0f},
};

/// 色データ: 木
const glm::vec4 colTree[] = {
  {0.5f, 0.8f, 0.3f, 1.0f},
  {0.2f, 0.3f, 0.1f, 1.0f},
  {0.2f, 0.3f, 0.1f, 1.0f},
  {0.2f, 0.3f, 0.1f, 1.0f},
  {0.2f, 0.3f, 0.1f, 1.0f},
  {0.2f, 0.1f, 0.1f, 1.0f},
  {0.5f, 0.3f, 0.2f, 1.0f},
  {0.5f, 0.3f, 0.2f, 1.0f},
  {0.5f, 0.3f, 0.2f, 1.0f},
};

/// 色データ: 家
const glm::vec4 colHouse[] = {
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
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
};

/// 色データ: 立方体
const glm::vec4 colCube[] = {
  {1,1,1,1},
  {1,1,1,1},
  {1,1,1,1},
  {1,1,1,1},
  {1,1,1,1},
  {1,1,1,1},
  {1,1,1,1},
  {1,1,1,1},
  {1,1,1,1},
  {1,1,1,1},
  {1,1,1,1},
  {1,1,1,1},
  {1,1,1,1},
  {1,1,1,1},
};

/// テクスチャ座標データ: 地面
const glm::vec2 tcGround[] = {
  {-4.0f,-4.0f },
  { 4.0f,-4.0f },
  { 4.0f, 4.0f },
  {-4.0f, 4.0f },
};

/// テクスチャ座標データ: 木
const glm::vec2 tcTree[] = {
  { 0.0f, 1.0f },
  { 0.0f, 0.7f },
  { 0.3f, 0.7f },
  { 0.6f, 0.7f },
  { 1.0f, 0.7f },
  { 0.0f, 1.0f },
  { 0.0f, 0.0f },
  { 0.5f, 0.0f },
  { 1.0f, 0.0f },
};

/// テクスチャ座標データ: 家
const glm::vec2 tcHouse[] = {
  { 1.000f, 0.00f},
  { 1.000f, 0.69f},
  { 0.875f, 1.00f},
  { 0.750f, 0.69f},
  { 0.750f, 0.00f},
  { 0.500f, 0.00f},
  { 0.500f, 0.69f},
  { 0.375f, 1.00f},
  { 0.250f, 0.69f},
  { 0.250f, 0.00f},
  { 0.000f, 0.00f},
  { 0.000f, 0.69f},
  { 0.750f, 1.00f},
  { 0.500f, 1.00f},
  { 0.250f, 1.00f},
  { 0.000f, 1.00f},
};

/// テクスチャ座標データ: 立方体
const glm::vec2 tcCube[] = {
  { 0.0f / 4.0f, 1.0f / 3.0f},
  { 1.0f / 4.0f, 1.0f / 3.0f},
  { 2.0f / 4.0f, 1.0f / 3.0f},
  { 2.0f / 4.0f, 0.0f / 3.0f},
  { 3.0f / 4.0f, 0.0f / 3.0f},
  { 3.0f / 4.0f, 1.0f / 3.0f},
  { 4.0f / 4.0f, 1.0f / 3.0f},
  { 4.0f / 4.0f, 2.0f / 3.0f},
  { 3.0f / 4.0f, 2.0f / 3.0f},
  { 3.0f / 4.0f, 3.0f / 3.0f},
  { 2.0f / 4.0f, 3.0f / 3.0f},
  { 2.0f / 4.0f, 2.0f / 3.0f},
  { 1.0f / 4.0f, 2.0f / 3.0f},
  { 0.0f / 4.0f, 2.0f / 3.0f},
};

/// 法線データ: 地面
const glm::vec3 normGround[] = {
  { 0, 1, 0 },
  { 0, 1, 0 },
  { 0, 1, 0 },
  { 0, 1, 0 },
};

/// 法線データ: 木
const glm::vec3 normTree[] = {
  { 0.00f, 1.00f, 0.00f},
  { 0.00f,-0.44f,-0.90f},
  {-0.90f,-0.44f, 0.00f},
  { 0.00f,-0.44f, 0.90f},
  { 0.90f,-0.44f, 0.00f},
  { 0.00f, 1.00f, 0.00f},
  { 0.00f, 0.00f,-1.00f},
  {-0.87f, 0.00f, 0.49f},
  { 0.87f, 0.00f, 0.49f},
};

/// 法線データ: 家
const glm::vec3 normHouse[] = {
  { 0.7f, 0.0f, 0.7f},
  { 0.7f, 0.0f, 0.7f},
  { 0.0f, 0.7f, 0.7f},
  {-0.7f, 0.0f, 0.7f},
  {-0.7f, 0.0f, 0.7f},

  {-0.7f, 0.0f,-0.7f},
  {-0.7f, 0.0f,-0.7f},
  { 0.0f, 0.7f,-0.7f},
  { 0.7f, 0.0f,-0.7f},
  { 0.7f, 0.0f,-0.7f},

  { 0.7f, 0.0f, 0.7f},
  { 0.7f, 0.0f, 0.7f},

  { 0.0f, 1.0f, 0.0f},
  { 0.0f, 1.0f,-0.0f},

  { 0.0f, 1.0f,-0.0f},
  { 0.0f, 1.0f, 0.0f},
};

/// 法線データ: 立方体
const glm::vec3 normCube[] = {
  glm::normalize(glm::vec3{ 1,-1, 1}),
  glm::normalize(glm::vec3{-1,-1, 1}),
  glm::normalize(glm::vec3{-1, 1, 1}),
  glm::normalize(glm::vec3{-1,-1, 1}),
  glm::normalize(glm::vec3{ 1,-1, 1}),
  glm::normalize(glm::vec3{ 1, 1, 1}),
  glm::normalize(glm::vec3{ 1,-1, 1}),
  glm::normalize(glm::vec3{ 1,-1,-1}),
  glm::normalize(glm::vec3{ 1, 1,-1}),
  glm::normalize(glm::vec3{ 1,-1,-1}),
  glm::normalize(glm::vec3{-1,-1,-1}),
  glm::normalize(glm::vec3{-1, 1,-1}),
  glm::normalize(glm::vec3{-1,-1,-1}),
  glm::normalize(glm::vec3{ 1,-1,-1}),
};

/// インデックスデータ: 地面
const GLushort indexGround[] = {
  0, 1, 2, 2, 3, 0,
};

/// インデックスデータ: 木
const GLushort indexTree[] = {
  0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1, 1, 4, 3, 3, 2, 1,
  5, 6, 7, 5, 7, 8, 5, 8, 5,
};

/// インデックスデータ: 家
const GLushort indexHouse[] = {
  0, 1, 3, 3, 4, 0, 1, 2, 3,
  4, 3, 6, 6, 5, 4,
  3, 12, 13, 13, 6, 3,
  5, 6, 8, 8, 9, 5, 6, 7, 8,
  9, 8, 11, 11, 10, 9,
  8, 14, 15, 15, 11, 8,
};

/// インデックスデータ: 立方体
//       10-09
//        |  |
// 13-12-11-08-07
//  |  |  |  |  |
// 00-01-02-05-06
//        |  |
//       03-04
const GLushort indexCube[] = {
  0, 1, 12, 12, 13, 0,
  1, 2, 11, 11, 12, 1,
  2, 5, 8, 8, 11, 2,
  3, 4, 5, 5, 2, 3,
  5, 6, 7, 7, 8, 5,
  11, 8, 9, 9, 10, 11,
};

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
void DrawLineOfTrees(const Mesh::Primitive& prim, Shader::Pipeline& pipeline, const glm::mat4& matVP, const glm::vec3& start, const glm::vec3& direction)
{
  glm::vec3 offset = start;
  for (float i = 0; i < 19; ++i) {
    const glm::vec3 offset = start + direction * i;
    const glm::mat4 matModelT = glm::translate(glm::mat4(1), offset);
    const glm::mat4 matModelR = glm::rotate(glm::mat4(1), glm::radians(i * 30), glm::vec3(0, 1, 0));
    const glm::mat4 matModel = matModelT * matModelR;
    const glm::mat4 matMVP = matVP * matModel;
    pipeline.SetMVP(matMVP);
    pipeline.SetModelMatrix(matModel);
    prim.Draw();
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
  {
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
  }

  Mesh::PrimitiveBuffer primitiveBuffer;
  if (!primitiveBuffer.Allocate(20'000, 80'000)) {
    return 1;
  }
  primitiveBuffer.Add(std::size(posGround), posGround, colGround, tcGround, normGround, std::size(indexGround), indexGround);
  primitiveBuffer.Add(std::size(posTree), posTree, colTree, tcTree, normTree, std::size(indexTree), indexTree);
  primitiveBuffer.Add(std::size(posHouse), posHouse, colHouse, tcHouse, normHouse, std::size(indexHouse), indexHouse);
  primitiveBuffer.Add(std::size(posCube), posCube, colCube, tcCube, normCube, std::size(indexCube), indexCube);

  // パイプライン・オブジェクトを作成する.
  Shader::Pipeline pipeline("Res/VertexLighting.vert", "Res/Simple.frag");
  if (!pipeline) {
    return 1;
  }

  // uniform変数の位置.
  const GLint locMatMVP = 0;

  // サンプラ・オブジェクトを作成する.
  Texture::Sampler sampler;
  if (!sampler) {
    return 1;
  }
  sampler.SetWrapMode(GL_REPEAT);
  sampler.SetFilter(GL_NEAREST);

  //const GLuint texGround = GLContext::CreateImage2D(imageWidth, imageHeight, imageGround);
  const Texture::Image2D texGround("Res/Ground.tga");
  const Texture::Image2D texTree(imageWidth, imageHeight, imageTree, GL_RGBA, GL_UNSIGNED_BYTE);
  const Texture::Image2D texHouse("Res/House.tga");
  const Texture::Image2D texCube("Res/Rock.tga");
  if (!texGround ||!texTree || !texHouse || !texCube) {
    return 1;
  }

  // メインループ.
  while (!glfwWindowShouldClose(window)) {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 平行光源を設定する
    const Shader::DirectionalLight directionalLight{
      glm::normalize(glm::vec4(3, 2, 2, 0)),
      glm::vec4(1, 0.9f, 0.8f, 1)
    };
    pipeline.SetLight(directionalLight);

    // 点光源を設定する
    const Shader::PointLight pointLight{
      glm::vec4(8, 10,-8, 0),
      glm::vec4(0.4f, 0.7f, 1.0f, 0) * 200.0f
    };
    pipeline.SetLight(pointLight);

    // 視点を回転させる.
    const float degree = static_cast<float>(std::fmod(glfwGetTime() * 10.0, 360.0));
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

    primitiveBuffer.BindVertexArray();
    pipeline.Bind();
    sampler.Bind(0);

    //primTree.Draw();

    // 木を描画.
#if 1
    texTree.Bind(0);
    for (float j = 0; j < 4; ++j) {
      const glm::mat4 matRot = glm::rotate(glm::mat4(1), glm::radians(90.0f) * j, glm::vec3(0, 1, 0));
      DrawLineOfTrees(primitiveBuffer.Get(1), pipeline, matProj * matView, matRot * glm::vec4(-19, 0, 19, 1), matRot * glm::vec4(2, 0, 0, 1));
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
      primitiveBuffer.Get(1).Draw();
    }
#endif

    // 地面を描画.
    {
      const glm::mat4 matModel = glm::mat4(1);
      const glm::mat4 matMVP = matProj * matView * matModel;
      pipeline.SetMVP(matMVP);
      pipeline.SetModelMatrix(matModel);
      texGround.Bind(0);
      primitiveBuffer.Get(0).Draw();
    }

    // 家を描画.
    {
      const glm::mat4 matModel = glm::mat4(1);
      const glm::mat4 matMVP = matProj * matView * matModel;
      pipeline.SetMVP(matMVP);
      pipeline.SetModelMatrix(matModel);
      texHouse.Bind(0);
      primitiveBuffer.Get(2).Draw();
    }

    // 立方体を描画.
    {
      const glm::mat4 matModel = glm::translate(glm::mat4(1), glm::vec3(10, 1, 0));
      const glm::mat4 matMVP = matProj * matView * matModel;
      pipeline.SetMVP(matMVP);
      pipeline.SetModelMatrix(matModel);
      texCube.Bind(0);
      primitiveBuffer.Get(3).Draw();
    }
    Texture::UnbindAllTextures();
    Texture::UnbindAllSamplers();
    Shader::UnbindPipeline();
    primitiveBuffer.UnbindVertexArray();

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  // GLFWの終了.
  glfwTerminate();

  return 0;
}