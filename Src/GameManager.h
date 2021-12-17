/**
* @file GameManager.h
*/
#ifndef GAMEMANAGER_H_INCLUDED
#define GAMEMANAGER_H_INCLUDED
#include "Actor.h"
#include <vector>

/**
* ゲーム管理クラス
*/
class GameManager
{
public:
  static bool Initialize();
  static void Finalize();
  static GameManager& Get();

  // ゲームの動作状態
  enum class State {
    title,     // タイトル画面(19bで実装. 19は未実装)
    start,     // ゲーム開始
    playing,   // ゲームプレイ中
    gameclear, // ゲームクリア
    gameover,  // ゲームオーバー
  };
  void SetState(State s);

  void Update(float deltaTime);
  void UpdateCamera();
  void UpdateUI();

  void AddScore(int n) { score += n; }

private:
  GameManager() = default;
  ~GameManager() = default;
  GameManager(const GameManager&) = delete;
  GameManager& operator=(const GameManager&) = delete;

  void LoadPrimitives();
  void LoadTextures();
  void SpawnPlayer();
  void SpawnEnemies();
  void SpawnMap();

  // 19bで実装. 19は未実装.
  void UpdateGameUI();
  void UpdateTitle(float deltaTime);
  void UpdateTitleUI();
  // ここまで(19bで実装. 19は未実装)

  // TODO: テキスト未実装
  void UpdateStage2(float deltaTime);

  State state = State::title; // 現在の動作状態
  std::shared_ptr<Actor> playerTank;
  std::vector<std::shared_ptr<Actor>> enemies;
  int score = 0;
  size_t stageNo = 0; // 21bで実装. 21は未実装

  // 19bで実装. 19は未実装.

  // タイトル画面の動作状態
  enum class TitleState {
    init,       // タイトル画面の初期化
    logoFadein, // ロゴフェードイン
    bgFadein,   // 背景フェードイン
    idle,       // ユーザーの入力待ち
    fadeout,    // タイトル画面フェードアウト
  };
  TitleState titleState = TitleState::init;
  float titleLogoAlpha = 0; // タイトルロゴの不透明度
  float titleBgAlpha = 0;   // タイトル背景の不透明度
  float titleEffectPosX[2] = { 0, 0 };
  float fadeAlpha = 0;      // フェードイン・アウトの不透明度
  bool startHovered = false;
  bool exitHovered = false;
  // ここまで(19bで実装. 19は未実装)
};

#endif // GAMEMANAGER_H_INCLUDED
