/**
* @file Main.cpp
*/
#include <glad/glad.h>
#include "GLContext.h"
#include "Primitive.h"
#include "ProgramPipeline.h"
#include "Texture.h"
#include "Sampler.h"
#include "Actor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <memory>
#pragma comment(lib, "opengl32.lib")

/// 座標データ:四角形
const glm::vec3 posRectangle[] = {
  {-0.2f, -0.5f, 0.1f},
  { 0.3f, -0.5f, 0.1f},
  { 0.3f,  0.3f, 0.1f},
  { 0.3f,  0.3f, 0.1f},
  {-0.2f,  0.3f, 0.1f},
  {-0.2f, -0.5f, 0.1f},
};

/// 座標データ:三角形
const glm::vec3 posTriangle[] = {
  {(-0.33f/2.0f)        * 10.0f, ( 0.5f/2.0f)      * 10.0f, 0.6f },
  {( 0.33f/2.0f)        * 10.0f, ( 0.5f/2.0f)      * 10.0f, 0.6f },
  {( 0.00f/2.0f)        * 10.0f, (-0.5f/2.0f)      * 10.0f, 0.6f },
  {(-0.33f/2.0f-0.165f) * 10.0f, ( 0.5f/2.0f+0.5f) * 10.0f, 0.6f },
  {( 0.33f/2.0f-0.165f) * 10.0f, ( 0.5f/2.0f+0.5f) * 10.0f, 0.6f },
  {( 0.00f/2.0f-0.165f) * 10.0f, (-0.5f/2.0f+0.5f) * 10.0f, 0.6f },
  {(-0.33f/2.0f+0.165f) * 10.0f, ( 0.5f/2.0f+0.5f) * 10.0f, 0.6f },
  {( 0.33f/2.0f+0.165f) * 10.0f, ( 0.5f/2.0f+0.5f) * 10.0f, 0.6f },
  {( 0.00f/2.0f+0.165f) * 10.0f, (-0.5f/2.0f+0.5f) * 10.0f, 0.6f },
};

/// 座標データ:立方体
const glm::vec3 posCube[] = {
  { 0, 0, 0}, { 1, 0, 0}, { 1, 0, 1}, { 0, 0, 1},
  { 0, 1, 0}, { 1, 1, 0}, { 1, 1, 1}, { 0, 1, 1},
};

/// 座標データ:
const glm::vec3 posTree[] = {
  // 木(葉)
  { 0.0f, 3.0f, 0.0f},
  { 0.0f, 1.0f,-1.0f},
  {-1.0f, 1.0f, 0.0f},
  { 0.0f, 1.0f, 1.0f},
  { 1.0f, 1.0f, 0.0f},
  { 0.0f, 1.0f,-1.0f},

  // 木(幹)
  { 0.0f, 2.0f, 0.0f},
  { 0.0f, 0.0f,-0.5f},
  {-0.5f, 0.0f, 0.0f},
  { 0.0f, 0.0f, 0.5f},
  { 0.5f, 0.0f, 0.0f},
  { 0.0f, 0.0f,-0.5f},
};

/// 座標データ:建物
const glm::vec3 posWarehouse[] = {
  {-2, 0,-2}, {-2, 0, 2}, { 2, 0, 2}, { 2, 0,-2}, {-2, 0,-2},
  {-2, 2,-2}, {-2, 2, 2}, { 2, 2, 2}, { 2, 2,-2}, {-2, 2,-2},
  { 2, 2, 2}, { 2, 2,-2},
};

/// 色データ:地面
const glm::vec4 colGround[] = {
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
};

/// 色データ:四角形
const glm::vec4 colRectangle[] = {
  {1.0f, 0.0f, 0.0f, 1.0f},
  {1.0f, 1.0f, 0.0f, 1.0f},
  {1.0f, 0.0f, 0.0f, 1.0f},
  {0.0f, 0.0f, 1.0f, 1.0f},
  {0.0f, 1.0f, 1.0f, 1.0f},
  {0.0f, 0.0f, 1.0f, 1.0f},
};

/// 色データ:三角形
const glm::vec4 colTriangle[] = {
  { 0.0f, 1.0f, 1.0f, 1.0f }, // 水色
  { 1.0f, 1.0f, 0.0f, 1.0f }, // 黄色
  { 1.0f, 0.0f, 1.0f, 1.0f }, // 紫色
  { 0.0f, 1.0f, 1.0f, 1.0f }, // 水色
  { 1.0f, 1.0f, 0.0f, 1.0f }, // 黄色
  { 1.0f, 0.0f, 1.0f, 1.0f }, // 紫色
  { 0.0f, 1.0f, 1.0f, 1.0f }, // 水色
  { 1.0f, 1.0f, 0.0f, 1.0f }, // 黄色
  { 1.0f, 0.0f, 1.0f, 1.0f }, // 紫色
};

/// 色データ:立方体
const glm::vec4 colCube[] = {
  { 1, 1, 1, 1}, { 1, 1, 1, 1}, { 1, 1, 1, 1}, { 1, 1, 1, 1},
  { 1, 1, 1, 1}, { 1, 1, 1, 1}, { 1, 1, 1, 1}, { 1, 1, 1, 1},
};

/// 色データ:木
const glm::vec4 colTree[] = {
  // 木(葉)
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},

  // 木(幹)
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
};

/// 色データ:建物
const glm::vec4 colWarehouse[] = {
  { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, 
  { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, 
  { 1, 1, 1, 1 }, { 1, 1, 1, 1 },
};

/// テクスチャ座標データ:四角形
const glm::vec2 tcRectangle[] = {
  { 0.0f, 0.0f}, { 1.0f, 0.0f}, { 1.0f, 1.0f},
  { 1.0f, 1.0f}, { 0.0f, 1.0f}, { 0.0f, 0.0f},
};

/// テクスチャ座標データ:三角形
const glm::vec2 tcTriangle[] = {
  { 0.0f, 0.0f}, { 1.0f, 0.0f}, { 0.5f, 1.0f},
  { 0.0f, 0.0f}, { 1.0f, 0.0f}, { 0.5f, 1.0f},
  { 0.0f, 0.0f}, { 1.0f, 0.0f}, { 0.5f, 1.0f},
};

/// テクスチャ座標データ:立方体
const glm::vec2 tcCube[] = {
  { 0.0f, 0.0f}, { 0.0f, 0.0f}, { 0.0f, 0.0f}, { 0.0f, 0.0f},
  { 0.0f, 0.0f}, { 0.0f, 0.0f}, { 0.0f, 0.0f}, { 0.0f, 0.0f},
};

/// テクスチャ座標データ:木
const glm::vec2 tcTree[] = {
  // 木(葉)
  { 0.5f, 1.0f},
  { 0.0f, 0.5f},
  { 0.25f, 0.5f},
  { 0.5f, 0.5f},
  { 0.75f, 0.5f},
  { 1.0f, 0.5f},

  // 木(幹)
  { 0.5f, 0.5f},
  { 0.0f, 0.0f},
  { 0.25f, 0.0f},
  { 0.5f, 0.0f},
  { 0.75f, 0.0f},
  { 1.0f, 0.0f},
};

/// テクスチャ座標データ:建物
const glm::vec2 tcWarehouse[] = {
  { 0.0f, 0.0f}, { 0.25f, 0.0f}, { 0.5f, 0.0f}, { 0.75f, 0.0f}, { 1.0f, 0.0f},
  { 0.0f, 0.5f}, { 0.25f, 0.5f}, { 0.5f, 0.5f}, { 0.75f, 0.5f}, { 1.0f, 0.5f},
  { 0.25f, 1.0f}, { 0.0f, 1.0f},
};

/// インデックスデータ:四角形
const GLushort indexRectangle[] = {
  0, 1, 2, 3, 4, 5,
};

/// インデックスデータ:三角形
const GLushort indexTriangle[] = {
  2, 1, 0, 5, 4, 3, 8, 7, 6,
};

/// インデックスデータ:立方体
const GLushort indexCube[] = {
 0, 1, 2, 2, 3, 0, 4, 5, 1, 1, 0, 4,
 5, 6, 2, 2, 1, 5, 6, 7, 3, 3, 2, 6,
 7, 4, 0, 0, 3, 7, 7, 6, 5, 5, 4, 7,
};

/// インデックスデータ:木
const GLushort indexTree[] = {
 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 1, 4, 3, 3, 2, 1, // 葉
 6, 7, 8, 6, 8, 9, 6, 9,10, 6,10,11, 7,10, 9, 9, 8, 7, // 幹
};

/// インデックスデータ:建物
const GLushort indexWarehouse[] = {
 0, 1, 6, 6, 5, 0,
 1, 2, 7, 7, 6, 1,
 2, 3, 8, 8, 7, 2,
 3, 4, 9, 9, 8, 3,
 5, 6,10,10,11, 5,
};

// 画像データ.
const int imageGroundWidth = 8; // 画像の幅.
const int imageGroundHeight = 8; // 画像の高さ.
const GLuint X = 0xff'18'18'18; // 黒.
const GLuint W = 0xff'ff'ff'ff; // 白.
const GLuint R = 0xff'10'10'e0; // 赤.
const GLuint B = 0xff'e0'10'10; // 青.
const GLuint imageGround[imageGroundWidth * imageGroundHeight] = {
  X, B, B, B, X, W, W, W,
  X, B, B, B, X, W, W, W,
  X, B, B, B, X, W, W, W,
  X, X, X, X, X, X, X, X,
  W, W, X, R, R, R, X, W,
  W, W, X, R, R, R, X, W,
  W, W, X, R, R, R, X, W,
  X, X, X, X, X, X, X, X,
};

const GLuint imageTriangle[6 * 6] = {
  X, W, X, W, X, W,
  X, W, X, W, X, W,
  X, W, X, W, X, W,
  X, W, X, W, X, W,
  X, W, X, W, X, W,
  X, W, X, W, X, W,
};

const GLuint G = 0xff'10'80'10; // 緑.
const GLuint D = 0xff'40'a0'40; // 茶色.
const GLuint imageGreen[8 * 8] = {
  G, G, G, G, G, G, G, G,
  G, D, G, G, G, G, G, G,
  G, G, G, G, G, G, G, G,
  G, G, G, G, G, D, G, G,
  G, G, G, G, G, G, G, G,
  G, G, G, G, G, G, G, G,
  G, G, G, G, G, G, G, G,
  G, G, G, G, G, G, G, G,
};

const GLuint P = 0xff'60'60'60;
const GLuint imageRoad[8 * 8] = {
  P, P, P, P, P, P, P, P,
  P, W, P, P, P, P, P, P,
  P, P, P, P, P, P, W, P,
  P, P, P, P, P, P, P, P,
  P, P, P, P, P, P, P, P,
  P, P, P, P, P, P, P, P,
  P, P, P, W, P, P, P, P,
  P, P, P, P, P, P, P, P,
};

/// マップデータ.
int mapData[10][10] = {
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 2, 2, 2, 2, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 2, 2, 2, 2},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
};

/// オブジェクトマップデータ.
int objectMapData[10][10] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 1, 1, 1, 0, 0, 0, 0, 2, 0, 0},
  { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 2, 0, 0, 0, 0, 2, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
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
  PrimitiveBuffer primitiveBuffer(200'000, 800'000);

  // 描画データを追加する.
  primitiveBuffer.AddFromObjFile("Res/Ground.obj");
  primitiveBuffer.AddFromObjFile("Res/Rectangle.obj");
  primitiveBuffer.AddFromObjFile("Res/Triangle.obj");
  primitiveBuffer.AddFromObjFile("Res/Cube.obj");
  primitiveBuffer.AddFromObjFile("Res/Tree.obj");
  primitiveBuffer.AddFromObjFile("Res/Warehouse.obj");
  primitiveBuffer.AddFromObjFile("Res/tank/Tiger_I.obj");
  primitiveBuffer.AddFromObjFile("Res/tank/T34.obj");
  primitiveBuffer.AddFromObjFile("Res/house/HouseRender.obj");

  // パイプライン・オブジェクトを作成する.
  ProgramPipeline pipeline("Res/FragmentLighting.vert", "Res/FragmentLighting.frag");
  if (!pipeline.IsValid()) {
    return 1;
  }

  // uniform変数の位置.
  const GLint locMatTRS = 0;
  const GLint locMatModel = 1;

  // 座標変換行列の回転角度.
  float degree = 0;

  // テクスチャを作成.
  std::shared_ptr<Texture> texGround(new Texture("Res/RoadTiles.tga"));
  std::shared_ptr<Texture> texTriangle(new Texture("Res/Triangle.tga"));
  std::shared_ptr<Texture> texGreen(new Texture("Res/Green.tga"));
  std::shared_ptr<Texture> texRoad(new Texture("Res/Road.tga"));
  std::shared_ptr<Texture> texTree(new Texture("Res/Tree.tga"));
  std::shared_ptr<Texture> texWarehouse(new Texture("Res/Building.tga"));
  std::shared_ptr<Texture> texTank(new Texture("Res/tank/PzVl_Tiger_I.tga"));
  std::shared_ptr<Texture> texTankT34(new Texture("Res/tank/T-34.tga"));
  std::shared_ptr<Texture> texBrickHouse(new Texture("Res/house/House38UVTexture.tga"));

  // サンプラを作成.
  std::shared_ptr<Sampler> sampler(new Sampler(GL_REPEAT));

  // 戦車のパラメータ
  Actor tank = { primitiveBuffer.Get(6), texTank,
    glm::vec3(0), glm::vec3(1), 0.0f, glm::vec3(0) };

  // T-34戦車のパラメータ
  Actor tankT34 = { primitiveBuffer.Get(7), texTankT34,
    glm::vec3(-5, 0, 0), glm::vec3(1), 0.0f, glm::vec3(0) };

  // 建物のパラメータ
  Actor brickHouse = { primitiveBuffer.Get(8), texBrickHouse,
    glm::vec3(-8, 0, 0), glm::vec3(2, 2, 2), 0.0f, glm::vec3(-2.6f, 2.0f, 0.8f) };

  // メインループ.
  double loopTime = glfwGetTime(); // 1/60秒間隔でループ処理するための時刻
  double diffLoopTime = 0;         // 時刻の差分
  while (!glfwWindowShouldClose(window)) {
    // 現在時刻を取得
    const double curLoopTime = glfwGetTime();
    // 現在時刻と前回時刻の差を、時刻の差分に加算
    diffLoopTime += curLoopTime - loopTime;
    // 時刻を現在時刻に更新
    loopTime = curLoopTime;
    // 時刻の差分が1/60秒未満なら、ループの先頭に戻る
    if (diffLoopTime < 1.0 / 60.0) {
      continue;
    }

    //
    // ゲーム状態を更新する
    //

    for (; diffLoopTime >= 1.0 / 60.0; diffLoopTime -= 1.0 / 60.0) {
      // 戦車を移動させる
      if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        tank.rotation += glm::radians(90.0f) / 60.0f;
      } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        tank.rotation -= glm::radians(90.0f) / 60.0f;
      }

      // tank.rotationが0のときの戦車の向きベクトル
      glm::vec3 tankFront(0, 0, 1);
      // tank.rotationラジアンだけ回転させる回転行列を作る
      const glm::mat4 matRot = glm::rotate(glm::mat4(1), tank.rotation, glm::vec3(0, 1, 0));
      // 向きベクトルをtank.rotationだけ回転させる
      tankFront = matRot * glm::vec4(tankFront, 1);

      if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        tank.position += tankFront * 4.0f / 60.0f;
      } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        tank.position -= tankFront * 4.0f / 60.0f;
      }
    }

    //
    // ゲーム状態を描画する
    //

    glEnable(GL_DEPTH_TEST); // 深度バッファを有効にする.
    //glEnable(GL_CULL_FACE);
    glClearColor(0.5f, 0.5f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    primitiveBuffer.BindVertexArray();
    pipeline.Bind();
    sampler->Bind(0);

    float s = sin(glm::radians(degree));
    float c = cos(glm::radians(degree));
    degree += 0.01f;
    glm::mat4 matT = glm::mat4(1);
    matT[3] = glm::vec4(-0.3,-0.5, 0.0, 1.0);
    glm::mat4 matS = glm::mat4(1);
    matS[0][0] = 0.5;
    matS[1][1] = 1.5;
    glm::mat4 matR = glm::mat4(1);
    matR[0][0] = c;
    matR[0][1] = -s;
    matR[1][0] = s;
    matR[1][1] = c;

    // プロジェクション行列を作成.
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    const glm::mat4 matProj =
      glm::perspective(glm::radians(45.0f),  aspectRatio, 0.1f, 200.0f);

    // ビュー行列を作成.
    const glm::mat4 matView =
      glm::lookAt(glm::vec3(0, 20, 20), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    // 三角形を描画する
    Actor actTriangle = { primitiveBuffer.Get(2), texTriangle,
      glm::vec3(0), glm::vec3(1), 0.0f, glm::vec3(0),
    };
    Draw(actTriangle, pipeline, matProj, matView);

    // 立方体を描画する
    Actor actCube = { primitiveBuffer.Get(3), texTriangle,
      glm::vec3(0), glm::vec3(1), 0.0f, glm::vec3(0),
    };
    Draw(actCube, pipeline, matProj, matView);

    // 建物を表示(課題12)
    Draw(brickHouse, pipeline, matProj, matView);

    // 戦車を表示
    Draw(tank, pipeline, matProj, matView);

    // T-34を表示(課題05)
    Draw(tankT34, pipeline, matProj, matView);

    // マップに配置する物体の表示データ.
    struct ObjectData {
      Primitive prim;
      const std::shared_ptr<Texture> tex;
    };

    // 描画する物体のリスト.
    const ObjectData objectList[] = {
      { Primitive(), 0 },    // なし
      { primitiveBuffer.Get(4), texTree }, // 木
      { primitiveBuffer.Get(5), texWarehouse }, // 建物
    };
    // 木を植える.
    //glBindTextureUnit(0, texTree); // テクスチャを割り当てる.
    //primTree.Draw();
#if 1
    for (int y = 0; y < 10; ++y) {
      for (int x = 0; x < 10; ++x) {
        const int objectNo = objectMapData[y][x];
        if (objectNo <= 0 || objectNo >= std::size(objectList)) {
          continue;
        }
        auto p = objectList[objectNo];

        // 四角形が4x4mなので、xとyを4倍した位置に表示する.
        const glm::vec3 position(x * 4 - 20, 0, y * 4 - 20);

        // 行列をシェーダに転送する 
        const glm::mat4 matModel = glm::translate(glm::mat4(1), position);
        const glm::mat4 matMVP = matProj * matView * matModel;
        pipeline.SetUniform(locMatTRS, matMVP);
        pipeline.SetUniform(locMatModel, matModel);

        p.tex->Bind(0); // テクスチャを割り当てる.
        p.prim.Draw();
      }
    }
#endif

    // マップを(-20,-20)-(20,20)の範囲に描画.
    const std::shared_ptr<Texture> mapTexList[] = { texGreen, texGround, texRoad };
    for (int y = 0; y < 10; ++y) {
      for (int x = 0; x < 10; ++x) {
        // 四角形が4x4mなので、xとyを4倍した位置に表示する.
        const glm::vec3 position(x * 4 - 20, 0, y * 4 - 20);

        // 行列をシェーダに転送する 
        const glm::mat4 matModel = glm::translate(glm::mat4(1), position);
        const glm::mat4 matMVP = matProj * matView * matModel;
        pipeline.SetUniform(locMatTRS, matMVP);
        pipeline.SetUniform(locMatModel, matModel);

        const int textureNo = mapData[y][x];
        mapTexList[textureNo]->Bind(0); // テクスチャを割り当てる.
        primitiveBuffer.Get(0).Draw();
      }
    }

    // テクスチャの割り当てを解除.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindSampler(0, 0);
    glBindProgramPipeline(0);
    primitiveBuffer.UnbindVertexArray();

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  // GLFWの終了.
  glfwTerminate();

  return 0;
}