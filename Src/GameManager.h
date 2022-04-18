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
    ending,    // エンディング画面
  };
  void SetState(State s);

  void Update(float deltaTime);
  void UpdateCamera();
  void UpdateUI();

  void AddScore(int n) { score += n; }

  // ゲームフラグの操作
  void SetGameFlagCount(size_t size);
  size_t GetGameFlagCount() const;
  void SetGameFlag(size_t no, bool value);
  bool GetGameFlag(size_t no) const;
  void SetGameFlagDesc(size_t no, std::string desc);
  std::string GetGameFlagDesc(size_t no) const;

  // 破壊対象フラグの操作
  bool GetTargetFlag(ActorTag tag) const;
  void SetTargetFlag(ActorTag tag, bool flag);
  void ClearAllTargetFlags();

private:
  GameManager() = default;
  ~GameManager() = default;
  GameManager(const GameManager&) = delete;
  GameManager& operator=(const GameManager&) = delete;

  void LoadPrimitives();
  void LoadTextures();
  void SpawnPlayer();
  void SpawnEnemies();
  void SpawnGrass();
  void SpawnMap();

  // 19bで実装. 19は未実装.
  void UpdateGameUI();
  void UpdateTitle(float deltaTime);
  void UpdateTitleUI();
  // ここまで(19bで実装. 19は未実装)

  // エンディング用メンバ関数
  void InitializeEnding();
  void UpdateEnding(float deltaTime);
  void UpdateEndingUI();

  State state = State::title; // 現在の動作状態
  std::shared_ptr<Actor> playerTank;
  std::vector<std::shared_ptr<Actor>> enemies;
  int score = 0;
  size_t stageNo = 0; // 21bで実装. 21は未実装

  // ゲームフラグ
  struct GameFlag
  {
    std::string description; // 説明文
    bool value = false; // フラグの値
  };
  std::vector<GameFlag> gameFlags;

  bool targetTagFlags[actorTagCount] = {}; // 破壊対象となるタグ
  std::vector<std::shared_ptr<Actor>> targets; // 破壊対象アクター

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

  // エンディング用データ
  struct EndingText {
    uint32_t color; // 色
    std::string text; // 文章(1行分)
  };
  std::vector<EndingText> endingText;
  bool isScrollFinished = false; // false=スクロール中 true=停止
  float endingPosY = 0.0f; // スクロール量
  float fontSize = 1.0f;   // エンディングの文字サイズ
};

#endif // GAMEMANAGER_H_INCLUDED
