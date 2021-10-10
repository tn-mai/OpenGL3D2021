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
#include "GameEngine.h"
#include "GameManager.h"
#include "Actor/PlayerActor.h"
#include "Actor/T34TankActor.h"
#include "Actor/RandomMovingEnemyActor.h"
#include "Actor/ElevatorActor.h"
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

//// アクターの配列.
//std::vector<std::shared_ptr<Actor>> actors;

/**
* エントリーポイント.
*/
int main()
{
  GameEngine::Initialize();
  GameEngine& engine = GameEngine::Get();

  // ゲームマネージャを作成
  GameManager::Initialize();
  GameManager& manager = GameManager::Get();

  // メインループ.
  double loopTime = engine.GetTime();     // 1/60秒間隔でループ処理するための時刻
  double diffLoopTime = 0;             // 時刻の差分
  const float deltaTime = 1.0f / 60.0f;// 時間間隔
  while (!engine.WindowShouldClose()) {
    // 現在時刻を取得
    const double curLoopTime = engine.GetTime();
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
    engine.NewFrame();
    for (; diffLoopTime >= deltaTime; diffLoopTime -= deltaTime) {
      engine.UpdateActors(deltaTime);
      manager.Update(deltaTime);
      engine.PostUpdateActors();
      engine.UpdatePhysics(deltaTime);
      manager.UpdateCamera();
      engine.UpdateCamera();
      engine.RemoveDeadActors();
    }
    manager.UpdateUI();

    //
    // ゲーム状態を描画する
    //
    engine.RenderDefault();
    engine.RenderUI();
    engine.PostRender();
    engine.SwapBuffers();
  }

  GameManager::Finalize();
  GameEngine::Finalize();

  return 0;
}