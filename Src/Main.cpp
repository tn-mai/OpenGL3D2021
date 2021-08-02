/**
* @file Main.cpp
*/
#include <glad/glad.h>
#include "GLContext.h"
#include "GameEngine.h"
#include "Primitive.h"
#include "ProgramPipeline.h"
#include "Texture.h"
#include "Sampler.h"
#include "Actor.h"
#include "Actor/T34TankActor.h"
#include "Actor/RandomMovingEnemyActor.h"
#include "Actor/ElevatorActor.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <iostream>
#include <memory>
#include <algorithm>
#include <unordered_map>
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
  {(-0.33f / 2.0f) * 10.0f, (0.5f / 2.0f) * 10.0f, 0.6f },
  {(0.33f / 2.0f) * 10.0f, (0.5f / 2.0f) * 10.0f, 0.6f },
  {(0.00f / 2.0f) * 10.0f, (-0.5f / 2.0f) * 10.0f, 0.6f },
  {(-0.33f / 2.0f - 0.165f) * 10.0f, (0.5f / 2.0f + 0.5f) * 10.0f, 0.6f },
  {(0.33f / 2.0f - 0.165f) * 10.0f, (0.5f / 2.0f + 0.5f) * 10.0f, 0.6f },
  {(0.00f / 2.0f - 0.165f) * 10.0f, (-0.5f / 2.0f + 0.5f) * 10.0f, 0.6f },
  {(-0.33f / 2.0f + 0.165f) * 10.0f, (0.5f / 2.0f + 0.5f) * 10.0f, 0.6f },
  {(0.33f / 2.0f + 0.165f) * 10.0f, (0.5f / 2.0f + 0.5f) * 10.0f, 0.6f },
  {(0.00f / 2.0f + 0.165f) * 10.0f, (-0.5f / 2.0f + 0.5f) * 10.0f, 0.6f },
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
int mapData[16][16] = {
  { 2,2,2,2,2,2,2,2,0,0,1,1,0,0,2,2},
  { 2,2,2,2,2,2,2,2,0,0,1,1,0,0,2,2},
  { 2,2,0,0,0,0,2,2,2,2,2,2,0,0,2,2},
  { 2,2,0,0,0,0,2,2,2,2,2,2,0,0,2,2},
  { 2,2,2,2,2,2,0,0,0,0,0,0,2,2,2,2},
  { 2,2,2,2,2,2,0,0,0,0,0,0,2,2,2,2},
  { 2,2,0,0,2,2,2,2,2,2,0,0,2,2,0,0},
  { 2,2,0,0,2,2,2,2,2,2,0,0,2,2,0,0},
  { 2,2,2,2,0,0,0,0,2,2,2,2,2,2,2,2},
  { 2,2,2,2,0,0,0,0,2,2,2,2,2,2,2,2},
  { 0,0,2,2,2,2,0,0,0,0,0,0,0,0,2,2},
  { 0,0,2,2,2,2,0,0,0,0,0,0,0,0,2,2},
  { 2,2,0,0,0,0,1,1,1,1,0,0,2,2,2,2},
  { 2,2,0,0,0,0,1,1,1,1,0,0,2,2,2,2},
  { 2,2,2,2,2,2,1,1,1,1,2,2,2,2,0,0},
  { 2,2,2,2,2,2,1,1,1,1,2,2,2,2,0,0},
};

/// オブジェクトマップデータ.
int objectMapData[16][16] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 0, 0, 1, 3, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 4, 1, 0, 0},
  { 0, 0, 4, 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 3, 0, 0},
  { 0, 0, 1, 3, 1, 2, 0, 0, 0, 0, 0, 0, 4, 1, 0, 0},
  { 0, 0, 0, 0, 0, 0, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 2, 3, 1, 3, 1, 4, 0, 0, 0, 0},
  { 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 4, 1, 0, 0, 1, 4},
  { 0, 0, 1, 3, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1},
  { 0, 0, 0, 0, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 1, 4, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  { 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0},
  { 4, 1, 0, 0, 0, 0, 3, 1, 3, 1, 1, 3, 1, 4, 0, 0},
  { 0, 0, 3, 1, 1, 4, 0, 0, 0, 0, 4, 1, 0, 0, 0, 0},
  { 0, 0, 1, 1, 3, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3},
};

//// アクターの配列.
//std::vector<std::shared_ptr<Actor>> actors;

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
  
  GameEngine::Initialize();
  GameEngine& engine = GameEngine::Get();

  // アクターの配列.
  std::vector<std::shared_ptr<Actor>>& actors = engine.GetActors();

  // VAOを作成する.
  PrimitiveBuffer& primitiveBuffer = engine.GetPrimitiveBuffer();

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
  primitiveBuffer.AddFromObjFile("Res/Bullet.obj");
  primitiveBuffer.AddFromObjFile("Res/house/broken-house.obj");

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
  std::shared_ptr<Texture> texBullet(new Texture("Res/Bullet.tga"));
  std::shared_ptr<Texture> texHouse2(new Texture("Res/house/broken-house.tga"));

  // サンプラを作成.
  std::shared_ptr<Sampler> sampler(new Sampler(GL_REPEAT));

  // マップに配置する物体の表示データ.
  struct ObjectData {
    const char* name;
    Primitive prim;
    const std::shared_ptr<Texture> tex;
    float scale = 1.0f;
    glm::vec3 ajustment = glm::vec3(0);
    Box collider;
  };

  // 画面端にコライダーを設定
  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(-36, 0, -34), glm::vec3(1, 2, 32), 0.0f, glm::vec3(0) }));
  actors.back()->collider = Box{ glm::vec3(0, 0, 0), glm::vec3(1, 4, 64) };
  actors.back()->isStatic = true;

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(30, 0, -34), glm::vec3(1, 2, 32), 0.0f, glm::vec3(0) }));
  actors.back()->collider = Box{ glm::vec3(0, 0, 0), glm::vec3(1, 4, 64) };
  actors.back()->isStatic = true;

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(-34, 0, -36), glm::vec3(32, 2, 1), 0.0f, glm::vec3(0) }));
  actors.back()->collider = Box{ glm::vec3(0, 0, 0), glm::vec3(64, 4, 1) };
  actors.back()->isStatic = true;

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(-34, 0, 30), glm::vec3(32, 2, 1), 0.0f, glm::vec3(0) }));
  actors.back()->collider = Box{ glm::vec3(0, 0, 0), glm::vec3(64, 4, 1) };
  actors.back()->isStatic = true;

  // 描画する物体のリスト.
  const Box col1 = { glm::vec3(-1.75f, 0, -1.75f), glm::vec3(1.75f, 2, 1.75f) };
  const ObjectData objectList[] = {
    { "", Primitive(), 0 },    // なし
    { "Tree", primitiveBuffer.Get(4), texTree }, // 木
    { "Warehouse", primitiveBuffer.Get(5), texWarehouse, 1, {}, col1 }, // 建物
    { "BrickHouse", primitiveBuffer.Get(8), texBrickHouse, 3, glm::vec3(-2.6f, 2.0f, 0.8f),
      Box{ glm::vec3(-3, 0, -2), glm::vec3(3, 3, 2) } }, // 建物
    { "House2", primitiveBuffer.Get(10), texHouse2, 1, {},
      Box{ glm::vec3(-2.5f, 0, -3.5f), glm::vec3(2.5f, 3, 3.5f) } }, // 建物
  };

  // 木を植える.
  for (int y = 0; y < 16; ++y) {
    for (int x = 0; x < 16; ++x) {
      const int objectNo = objectMapData[y][x];
      if (objectNo <= 0 || objectNo >= std::size(objectList)) {
        continue;
      }
      const ObjectData p = objectList[objectNo];

      // 四角形が4x4mなので、xとyを4倍した位置に表示する.
      const glm::vec3 position(x * 4 - 32, 0, y * 4 - 32);

      actors.push_back(std::shared_ptr<Actor>(new Actor{ p.name, p.prim, p.tex,
        position, glm::vec3(p.scale), 0.0f, p.ajustment }));
      actors.back()->collider = col1;// p.collider;
      actors.back()->isStatic = true;
    }
  }

  // マップを(-20,-20)-(20,20)の範囲に描画.
  const std::shared_ptr<Texture> mapTexList[] = { texGreen, texGround, texRoad };
  for (int y = 0; y < 16; ++y) {
    for (int x = 0; x < 16; ++x) {
      // 四角形が4x4mなので、xとyを4倍した位置に表示する.
      const glm::vec3 position(x * 4 - 32, 0, y * 4 - 32);

      const int textureNo = mapData[y][x];
      actors.push_back(std::shared_ptr<Actor>(new Actor{ "Ground", primitiveBuffer.Get(0), mapTexList[textureNo],
        position, glm::vec3(1), 0.0f, glm::vec3(0) }));
      actors.back()->collider = Box{ glm::vec3(-2, -10, -2), glm::vec3(2, 0, 2) };
      actors.back()->isStatic = true;
    }
  }

  // エレベーター
  {
    const glm::vec3 position(4 * 4 - 20, -1, 4 * 4 - 20);
    actors.push_back(std::shared_ptr<Actor>(new ElevatorActor{
      "Elevator", primitiveBuffer.Get(0), mapTexList[0],
      position, glm::vec3(1), 0.0f, glm::vec3(0) }));
    actors.back()->velocity.y = 1;
    //actors.back()->collider = Box{ glm::vec3(-2, -10, -2), glm::vec3(2, 0, 2) };
    actors.back()->isStatic = true;
  }

  // 三角形のパラメータ
  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Triangle", primitiveBuffer.Get(2), texTriangle,
    glm::vec3(0, 0, -5), glm::vec3(1), 0.0f, glm::vec3(0) }));
  // 立方体のパラメータ
  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Cube", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(0, 0, -4), glm::vec3(1), 0.0f, glm::vec3(0) }));
  // 戦車のパラメータ
  std::shared_ptr<Actor> playerTank(new Actor{ "Tiger-I", primitiveBuffer.Get(6), texTank,
    glm::vec3(0), glm::vec3(1), 0.0f, glm::vec3(0) });
  playerTank->collider = Box{ glm::vec3(-1.8f, 0, -1.8f), glm::vec3(1.8f, 2.8f, 1.8f) };
  playerTank->mass = 57'000;
  //playerTank->cor = 0.1f;
  //playerTank->friction = 1.0f;
  actors.push_back(playerTank);


  std::shared_ptr<GameMap> gamemap(new GameMap(16, 16, -32, -32, 4, &objectMapData[0][0]));
  std::vector<glm::ivec2> route = gamemap->FindRoute(glm::ivec2(7, 15), glm::ivec2(10, 0));

  // T-34戦車のパラメータ
  const glm::vec3 t34PosList[] = {
    glm::vec3(-5, 0, 0),
    glm::vec3(15, 0, 0),
    glm::vec3(-10, 0, -5),
  };
  for (auto& pos : t34PosList) {
    std::string name("T-34[");
    name += '0' + static_cast<char>(&pos - t34PosList);
    name += ']';
    actors.push_back(std::shared_ptr<Actor>(new RandomMovingEnemyActor{ name.c_str(), primitiveBuffer.Get(7), texTankT34,
      pos, glm::vec3(1), 0.0f, glm::vec3(-0.78f, 0, 1.0f), gamemap }));
    actors.back()->collider = Box{ glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2.5f, 1.5f) };
    actors.back()->mass = 36'000;
  }

  // メインループ.
  double loopTime = glfwGetTime();     // 1/60秒間隔でループ処理するための時刻
  double diffLoopTime = 0;             // 時刻の差分
  const float deltaTime = 1.0f / 60.0f;// 時間間隔
  int oldShotButton = 0;               // 前回のショットボタンの状態
  glm::vec3 cameraPosition = glm::vec3(0, 20, 20); // カメラの座標
  glm::vec3 cameraTarget = glm::vec3(0, 0, 0);     // カメラの注視点の座標
  while (!glfwWindowShouldClose(window)) {
    // 現在時刻を取得
    const double curLoopTime = glfwGetTime();
    // 現在時刻と前回時刻の差を、時刻の差分に加算
    diffLoopTime += curLoopTime - loopTime;
    // 時刻を現在時刻に更新
    loopTime = curLoopTime;
    // 時刻の差分が1/60秒未満なら、ループの先頭に戻る
    if (diffLoopTime < deltaTime) {
      continue;
    }
    if (diffLoopTime > 20.0 / 60.0) {
      diffLoopTime = deltaTime;
    }

    //
    // ゲーム状態を更新する
    //

    for (; diffLoopTime >= deltaTime; diffLoopTime -= deltaTime) {

      // 以前の速度を更新
      for (int i = 0; i < actors.size(); ++i) {
        actors[i]->oldVelocity = actors[i]->velocity;
      }

      // 戦車を移動させる
      Actor* tank = Find(actors, "Tiger-I");
      if (tank) {
        if (tank->isOnActor) {
          if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            tank->rotation += glm::radians(90.0f) * deltaTime;
          } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            tank->rotation -= glm::radians(90.0f) * deltaTime;
          }
        }

        // tank.rotationが0のときの戦車の向きベクトル
        glm::vec3 tankFront(0, 0, 1);
        // tank.rotationラジアンだけ回転させる回転行列を作る
        const glm::mat4 matRot = glm::rotate(glm::mat4(1), tank->rotation, glm::vec3(0, 1, 0));
        // 向きベクトルをtank.rotationだけ回転させる
        tankFront = matRot * glm::vec4(tankFront, 1);

        if (tank->isOnActor) {
          float speed2 = glm::dot(tank->velocity, tank->velocity);
          //if (speed2 < 10.0f * 10.0f) {
          float tankAccel = 0.2f; // 戦車の加速度
          if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            tank->velocity += tankFront * tankAccel;
          } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            tank->velocity -= tankFront * tankAccel;
          } else {
            float v = glm::dot(tankFront, tank->velocity);
            tank->velocity -= tankFront * glm::clamp(v, -0.1f, 0.1f);
          }
          glm::vec3 tankRight = glm::normalize(glm::cross(tankFront, glm::vec3(0, 1, 0)));
          float rightSpeed = glm::dot(tankRight, tank->velocity);
          tank->velocity -= tankRight * glm::clamp(rightSpeed, -0.2f, 0.2f);
          //}
        }

        // マウス左ボタンの状態を取得する
        int shotButton = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

        // マウス左ボタンが押された瞬間に弾アクターを発射する
        static int shotInterval = 5;
        bool isShot = false;
        if (shotButton != 0) {
          if (oldShotButton == 0 || --shotInterval <= 0) {
            isShot = true;
            shotInterval = 5;
          }
        }
        if (isShot) {
          // 発射位置を砲の先端に設定
          glm::vec3 position = tank->position + tankFront * 6.0f;
          position.y += 2.0f;

          std::shared_ptr<Actor> bullet(new Actor{
            "Bullet", primitiveBuffer.Get(9), texBullet,
            position, glm::vec3(0.25f), tank->rotation, glm::vec3(0) });

          // 1.5秒後に弾を消す
          bullet->lifespan = 1.5f;

          // 戦車の向いている方向に、30m/sの速度で移動させる
          bullet->velocity = tankFront * 30.0f;

          // 弾に衝突判定を付ける
          bullet->collider = Box{ glm::vec3(-0.25f), glm::vec3(0.25f) };
          bullet->mass = 6.8f;
          bullet->friction = 1.0f;

          actors.push_back(bullet);
        }

        // 「前回のショットボタンの状態」を更新する
        oldShotButton = shotButton;
      }

      // アクターの状態を更新する
      for (int i = 0; i < actors.size(); ++i) {
        // アクターの寿命を減らす
        if (actors[i]->lifespan > 0) {
          actors[i]->lifespan -= deltaTime;

          // 寿命の尽きたアクターを「削除待ち」状態にする
          if (actors[i]->lifespan <= 0) {
            actors[i]->isDead = true;
            continue; // 削除待ちアクターは更新をスキップ
          }
        }

        actors[i]->OnUpdate(deltaTime);

        // 速度に重力加速度を加える
        if (!actors[i]->isStatic) {
          actors[i]->velocity.y += -9.8f * deltaTime;
        }

        // アクターの位置を更新する
        actors[i]->position += actors[i]->velocity * deltaTime;
      }

      GameEngine::Get().UpdateActors();

      // アクターの衝突判定を行う
#if 0
      std::vector<Actor*> dynamicActors;
      std::vector<Actor*> staticActors;
      dynamicActors.reserve(actors.size());
      staticActors.reserve(actors.size());
      for (int i = 0; i < actors.size(); ++i) {
        if (actors[i]->isStatic) {
          staticActors.push_back(&actors[i]);
        } else {
          dynamicActors.push_back(&actors[i]);
        }
      }
#endif

      for (int i = 0; i < actors.size(); ++i) {
        actors[i]->isOnActor = false;
      }

      std::vector<Contact> contacts;
      contacts.reserve(actors.size());
      for (int a = 0; a < actors.size(); ++a) {
        for (int b = a + 1; b < actors.size(); ++b) {

          // 削除待ちアクターは衝突しない
          if (actors[a]->isDead) {
            break;
          } else if (actors[b]->isDead) {
            continue;
          }

          Contact contact;
          if (DetectCollision(*actors[a], *actors[b], contact)) {
#if 1
            // 配列の中に、作成したコンタクト構造体と似ているものがあるか調べる
            auto itr = std::find_if(contacts.begin(), contacts.end(),
              [&contact](const Contact& c) { return Equal(contact, c); });

            // 似ているコンタクト構造体が見つからなければ、作成した構造体を配列に追加する
            if (itr == contacts.end()) {
              contacts.push_back(contact);
            } else {
              // 似ている構造体が見つかった場合、浸透距離が長いほうを残す
              if (contact.penLength > itr->penLength) {
                *itr = contact;
              }
            }
#else
            contacts.push_back(contact);
#endif
          }
        }
      }

      // 重なりを解決する
      for (int i = 0; i < contacts.size(); ++i) {
        Contact& c = contacts[i];

        // 衝突処理関数を呼び出す
        c.a->OnCollision(c);
        Contact contactBtoA;
        contactBtoA.a = c.b;
        contactBtoA.b = c.a;
        contactBtoA.velocityA = c.velocityB;
        contactBtoA.velocityB = c.velocityA;
        contactBtoA.accelA = c.accelB;
        contactBtoA.accelB = c.accelA;
        contactBtoA.penetration = -c.penetration;
        contactBtoA.normal = -c.normal;
        contactBtoA.position = c.position;
        contactBtoA.penLength = c.penLength;
        c.b->OnCollision(contactBtoA);

        // 重なりを解決する
        SolveContact(contacts[i]);
      }

      // 削除待ちのアクターを削除する
      actors.erase(
        std::remove_if(actors.begin(), actors.end(),
          [](std::shared_ptr<Actor>& a) { return a->isDead; }),
        actors.end());

      // カメラデータを更新する
      {
        Actor* target = Find(actors, "Tiger-I");
        if (target) {
          const glm::mat4 matRot = glm::rotate(glm::mat4(1), target->rotation, glm::vec3(0, 1, 0));
          const glm::vec3 tankFront = matRot * glm::vec4(0, 0, 1, 1);
          cameraPosition = target->position + glm::vec3(0, 20, 20);
          cameraTarget = target->position;
        }
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
    matT[3] = glm::vec4(-0.3, -0.5, 0.0, 1.0);
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
      glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 200.0f);

    // ビュー行列を作成.
    const glm::mat4 matView =
      glm::lookAt(cameraPosition, cameraTarget, glm::vec3(0, 1, 0));

    // アクターを描画する
    for (int i = 0; i < actors.size(); ++i) {
      Draw(*actors[i], pipeline, matProj, matView);
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

  GameEngine::Finalize();

  // GLFWの終了.
  glfwTerminate();

  return 0;
}