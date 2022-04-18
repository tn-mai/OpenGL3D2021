/**
* @file GameManager.cpp
*/
#include "GameManager.h"
#include "GameEngine.h"
#include "MapEditor.h"
#include "AStar.h"
#include "Actor/PlayerActor.h"
#include "Actor/T34TankActor.h"
#include "Actor/ElevatorActor.h"
#include "Actor/Boss01.h"
#include "Actor/HumanActor.h"
#include "Audio.h"
#ifdef USE_EASY_AUDIO
#include "EasyAudioSettings.h"
#else
#include "Audio/MainWorkUnit/BGM.h"
#include "Audio/MainWorkUnit/SE.h"
#endif // USE_EASY_AUDIO
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>

namespace {

GameManager* manager = nullptr;

const char* const mapFiles[] = {
  "mapdata00.txt",
  "mapdata01.txt",
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

}

/**
* ゲームマネージャの初期化
*/
bool GameManager::Initialize()
{
  if (!manager) {
    manager = new GameManager;
  }
  return true;
}

/**
* ゲームマネージャの終了
*/
void GameManager::Finalize()
{
  if (manager) {
    delete manager;
    manager = nullptr;
  }
}

/**
* ゲームエンジンを取得する
*/
GameManager& GameManager::Get()
{
  return *manager;
}

/**
*
*/
void GameManager::SetState(State s)
{
  static const char* names[] = {
    "initializeLevel",
    "start",
    "playing",
    "gameclear",
    "gameover",
    "ending",
  };
  std::cout << names[static_cast<int>(state)] <<
    "->" << names[static_cast<int>(s)] << "\n";

  state = s;
}

/**
* ゲームの動作状態を更新する
*/
void GameManager::Update(float deltaTime)
{
  GameEngine& engine = GameEngine::Get();
  switch (state) {
  case State::title:
    UpdateTitle(deltaTime);
    break;

  case State::start:
    // A*アルゴリズムのテスト
    // ※テストが終わったら消すこと
    AStar::Test();

    //score = 0;
    //SpawnPlayer();
    //SpawnEnemies();

    // マップデータをロードする
    stageNo = std::min(stageNo, std::size(mapFiles) - 1);
    MapEditor(MapEditor::SystemType::game).Load(mapFiles[stageNo], true);

    SpawnGrass();

    // プレイヤーが操作するアクターを取得する
    playerTank = engine.FindActor("Tiger-I");
    if (playerTank) {
      static_cast<PlayerActor&>(*playerTank).SetControlFlag(true);
    }

    // 人間アクターを配置
    engine.AddActor(std::make_shared<HumanActor>(
      playerTank->position + glm::vec3(0, 0, -5), glm::vec3(3), glm::radians(-60.0f)));

    // 敵アクターのポインタを配列にコピーする
    enemies.clear();
    for (auto& e : engine.GetNewActors()) {
      if (e->name == "T-34") {
        T34TankActor& enemy = static_cast<T34TankActor&>(*e);
        enemy.SetTarget(playerTank);
        enemies.push_back(e);
      } else if (e->name == "Boss01") {
        Boss01& enemy = static_cast<Boss01&>(*e);
        enemy.SetTarget(playerTank);
        enemies.push_back(e);
      }
    }

    // 破壊対象アクターのポインタを配列にコピーする
    targets.clear();
    for (auto& e : engine.GetNewActors()) {
      if (targetTagFlags[static_cast<int>(e->tag)]) {
        targets.push_back(e);
      }
    }
    
    { // ゲーム開始メッセージを表示する
      std::shared_ptr<Actor> gamestart(new Actor{ "GameStart",
        engine.GetPrimitive("Res/Plane.obj"),
        engine.LoadTexture("Res/GameStart.tga"),
        glm::vec3(0, 5, 0), glm::vec3(800.0f, 200.0f, 1.0f), 0.0f, glm::vec3(0) });
      gamestart->lifespan = 3;
      gamestart->isStatic = true;
      gamestart->layer = Layer::UI;
      engine.AddActor(gamestart);
    }

    // BGMを再生
#ifdef USE_EASY_AUDIO
    Audio::Play(AUDIO_PLAYER_ID_BGM, BGM_MAINGAME, 1.0f, true);
#else
    Audio::Get().Play(0, CRI_BGM_MAINGAME);
#endif // USE_EASY_AUDIO

    SetState(State::playing);
    break;

  case State::playing:
    if (playerTank->isDead) {
      std::shared_ptr<Actor> gameover(new Actor{ "GameOver",
        engine.GetPrimitive("Res/Plane.obj"),
        engine.LoadTexture("Res/GameOver.tga"),
        glm::vec3(0), glm::vec3(700, 200, 1), 0, glm::vec3(0) });
      gameover->isStatic = true;
      gameover->layer = Layer::UI;
      engine.AddActor(gameover);

#ifdef USE_EASY_AUDIO
      Audio::Play(AUDIO_PLAYER_ID_BGM, BGM_GAMEOVER);
#else
      Audio::Get().Play(0, CRI_BGM_GAMEOVER);
#endif // USE_EASY_AUDIO
      SetState(State::gameover);
    }
    else {
      bool allKill = true;
      for (int i = 0; i < targets.size(); ++i) {
        if (!targets[i]->isDead) {
          allKill = false;
          break;
        }
      }
      if (allKill) {
        // 最終ステージをクリアしたらエンディングへ
        if (stageNo + 1 >= std::size(mapFiles)) {
          InitializeEnding();
        } else {
          // ステージクリア画像を表示
          std::shared_ptr<Actor> gameclear(new Actor{ "GameClear",
            engine.GetPrimitive("Res/Plane.obj"),
            engine.LoadTexture("Res/GameClear.tga"),
            glm::vec3(0), glm::vec3(700, 200, 1), 0.0f, glm::vec3(0) });
          gameclear->isStatic = true;
          gameclear->layer = Layer::UI;
          engine.AddActor(gameclear);

#ifdef USE_EASY_AUDIO
          Audio::Play(AUDIO_PLAYER_ID_BGM, BGM_GAMECLEAR);
#else
          Audio::Get().Play(0, CRI_BGM_GAMECLEAR);
#endif // USE_EASY_AUDIO
          SetState(State::gameclear);
        }
      } // allKill
    }
    break;

  case State::gameclear:
    if (engine.GetKey(GLFW_KEY_ENTER)) {
#ifdef USE_EASY_AUDIO
      Audio::PlayOneShot(SE_OK);
#else
      Audio::Get().Play(1, CRI_SE_UI_OK);
#endif // USE_EASY_AUDIO
      std::shared_ptr<Actor> gameclear = engine.FindActor("GameClear");
      if (gameclear) {
        gameclear->isDead = true;
      }
      stageNo = std::min(stageNo + 1, std::size(mapFiles) - 1);
      SetState(State::start);
    }
    break;

  case State::gameover:
    if (engine.GetKey(GLFW_KEY_ENTER)) {
#ifdef USE_EASY_AUDIO
      Audio::PlayOneShot(SE_OK);
#else
      Audio::Get().Play(1, CRI_SE_UI_OK);
#endif // USE_EASY_AUDIO
      std::shared_ptr<Actor> gameover = engine.FindActor("GameOver");
      if (gameover) {
        gameover->isDead = true;
      }
      SetState(State::title);
    }
    break;

  case State::ending:
    UpdateEnding(deltaTime);
    break;
  }
}

/**
* カメラの状態を更新する
*/
void GameManager::UpdateCamera()
{
  GameEngine& engine = GameEngine::Get();

  // カメラデータを更新する
  std::shared_ptr<Actor> target = Find(engine.GetActors(), "Tiger-I");
  if (target) {
    const glm::mat4 matRot =
      glm::rotate(glm::mat4(1), target->rotation, glm::vec3(0, 1, 0));
    const glm::vec3 tankFront = matRot * glm::vec4(0, 0, 1, 1);
    Camera& camera = engine.GetCamera();
    camera.position = target->position + glm::vec3(0, 20, 20);
    camera.target = target->position;

    // ボケ量を設定(小さいほどボケる)
    camera.fNumber = 0.02f;
  }
}

/**
* UIの状態を更新する
*/
void GameManager::UpdateUI()
{
  switch (state) {
  case State::title: UpdateTitleUI(); break;
  case State::ending: UpdateEndingUI(); break;
  default:           UpdateGameUI(); break;
  }
}

/**
* ゲームUIの更新
*/
void GameManager::UpdateGameUI()
{
  /*
  * 表示物
  * - スコア
  * - プレイヤーのHP
  * - 残りの敵の数
  * - ステージ番号
  * - ミニマップ
  * - 敵のHP
  * - メイン武器の種類
  * - サブ武器の種類/残数
  */
  GameEngine& engine = GameEngine::Get(); // ゲームエンジンを取得
  ImGuiStyle& style = ImGui::GetStyle(); // スタイル構造体を取得
  const ImGuiStyle styleBackup = style;  // 元に戻すためのバックアップ

  // スコア(得点)表示
  {
    ImGui::SetNextWindowSize(ImVec2(200, 0));
    ImGui::SetNextWindowPos(ImVec2(540, 16));
    ImGui::Begin("SCORE", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);
    ImGui::SetWindowFontScale(3.0f);
    const ImVec2 pos = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(pos.x + 3, pos.y + 3));
    ImGui::TextColored(ImVec4(0.1f, 0.1f, 1.0f, 1.0f), "%d", score);
    ImGui::SetCursorPos(pos);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%d", score);
    ImGui::End();
  }

  // プレイヤーのHP表示
  if (playerTank) {
    ImGui::SetNextWindowSize(ImVec2(300, 0));
    ImGui::SetNextWindowPos(ImVec2(16, 16));
    ImGui::Begin("HP", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1, 0, 0, 1);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 0, 0, 0.5f);
    style.Colors[ImGuiCol_Border] = ImVec4(1.0f, 1, 1, 1.0f);
    style.FrameBorderSize = 3.0f; // 枠の太さ
    style.FrameRounding = 8.0f; // ふちの丸さ
    const float maxPlayerHealth = 10;
    const float f = playerTank->health / maxPlayerHealth;
    ImGui::ProgressBar(f, ImVec2(0, 0), "");
    style = styleBackup; // スタイルを元に戻す
    ImGui::End();
  }

  // 破壊対象の数を表示
  {
    ImGui::SetNextWindowSize(ImVec2(600, 0));
    ImGui::SetNextWindowPos(ImVec2(16, 720 - 16 - 40));
    ImGui::Begin("EnemyCount", nullptr, ImGuiWindowFlags_NoTitleBar);
    std::shared_ptr<Texture> tex = engine.LoadTexture("Res/IconEnemy.tga");
    const ImTextureID texId = reinterpret_cast<ImTextureID>(tex->GetId());
    for (const std::shared_ptr<Actor>& e : targets) {
      if (e->health > 0 && !e->isDead) {
        ImGui::SameLine();
        ImGui::Image(texId, ImVec2(40, 40), ImVec2(0, 1), ImVec2(1, 0));
      }
    }
    ImGui::End();
  }

  // レーダーを表示
  {
    const float radius = 100; // レーダーの半径
    const ImVec2 windowSize(
      radius * 2 + std::max(style.WindowBorderSize, style.WindowPadding.x) * 2,
      radius * 2 + std::max(style.WindowBorderSize, style.WindowPadding.y) * 2);
    const ImVec2 windowPos(1280 - 16 - windowSize.x, 720 - 16 - windowSize.y);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowPos(windowPos);
    ImGui::Begin("Lader", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

    // レーダーの円を表示
    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    ImVec2 center = ImGui::GetCursorScreenPos();
    center.x += radius;
    center.y += radius;
    drawlist->AddCircleFilled(center, radius, ImColor(0.0f, 0.0f, 0.0f, 0.75f));
    drawlist->AddLine(ImVec2(center.x - radius, center.y), ImVec2(center.x + radius, center.y), ImColor(0.1f, 1.0f, 0.1f));
    drawlist->AddLine(ImVec2(center.x, center.y - radius), ImVec2(center.x, center.y + radius), ImColor(0.1f, 1.0f, 0.1f));

    // プレイヤーを表示
    const float c = playerTank ? std::cos(playerTank->rotation) : 1.0f;
    const float s = playerTank ? std::sin(playerTank->rotation) : 0.0f;
    ImVec2 p[3] = { {0, 6}, {-4, -4}, {4, -4} };
    for (ImVec2& e : p) {
      e = ImVec2(center.x + e.x * c + e.y * s, center.y - e.x * s + e.y * c);
    }
    drawlist->AddTriangle(p[0], p[1], p[2], ImColor(1.0f, 1.0f, 0.1f), 2.0f);

    // 敵を表示
    if (playerTank) {
      for (const std::shared_ptr<Actor>& e : enemies) {
        if (!e->isDead) {
          const glm::vec3 v = (e->position - playerTank->position) * 2.0f;
          if (v.x * v.x + v.z * v.z < radius * radius) {
            const ImVec2 p(center.x + v.x, center.y + v.z);
            drawlist->AddCircleFilled(p, 4.0f, ImColor(1.0f, 0.1f, 0.1f));
          }
        }
      }
    }

    // レーダー枠を表示
    drawlist->AddCircle(center, radius, ImColor(1.0f, 1.0f, 1.0f, 1.0f), 0, 4.0f);

    ImGui::End();
  }

  if (state == State::gameclear) {
    const ImVec2 screenMin(0, 0);
    const ImVec2 screenMax(engine.GetWindowSize().x, engine.GetWindowSize().y);
    const float cx = (screenMin.x + screenMax.x) * 0.5f;
    const ImVec2 buttonSize(320, 64);
    ImGui::SetNextWindowPos(ImVec2(cx - buttonSize.x * 0.5f, 500));
    ImGui::Begin("NextStage", nullptr,
      ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);

    style.Colors[ImGuiCol_Button] = ImVec4(0.5f, 0.5f, 0.55f, 0.8f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.7f, 0.7f, 0.75f, 0.8f);
    style.Colors[ImGuiCol_Text] = ImVec4(0, 0, 0, 1);
    style.FrameRounding = 12;
    style.FrameBorderSize = 4;
    ImGui::SetWindowFontScale(4.0f);
    if (ImGui::Button("Next Stage", buttonSize)) {
#ifdef USE_EASY_AUDIO
      Audio::PlayOneShot(SE_OK);
#else
      Audio::Get().Play(1, CRI_SE_UI_OK);
#endif // USE_EASY_AUDIO
      std::shared_ptr<Actor> gameclear = engine.FindActor("GameClear");
      if (gameclear) {
        gameclear->isDead = true;
      }
      stageNo = std::min(stageNo + 1, std::size(mapFiles) - 1);
      SetState(State::start);
    }
    style = styleBackup; // スタイルを元に戻す
    ImGui::End();
  }

#if 0
  ImGui::Begin("SCORE", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
  ImGui::SetWindowFontScale(4);
  char scoreText[32];
  snprintf(scoreText, std::size(scoreText), "%d", score);
  const float width = ImGui::GetWindowContentRegionMax().x;
  const float textWidth = ImGui::CalcTextSize(scoreText).x;
  ImGui::SetCursorPosX(width - textWidth);
  ImGui::Text(scoreText);
  ImGui::End();

  style.WindowPadding = ImVec2(8, 8);
  style.WindowMinSize = ImVec2();
  style.Alpha = 1.0f;
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.2f, 0.4f, 1.0f);
  ImGui::SetNextWindowPos(ImVec2(16, 16));
  ImGui::Begin("hp", nullptr, ImGuiWindowFlags_NoDecoration);// | ImGuiWindowFlags_NoBackground);
  float hp = 0;
  char text[16] = " 0/10";
  if (playerTank) {
    hp = playerTank->health / 10.0f;
    int n = static_cast<int>(playerTank->health);
    for (int i = 1; i >= 0; --i) {
      text[i] = '0' + n % 10;
      n /= 10;
      if (n <= 0) {
        break;
      }
    }
  }
  ImGui::Text("HP");
  ImGui::SameLine();
  ImVec2 pos = ImGui::GetCursorPos();
  style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0, 1, 0, 1);
  style.Colors[ImGuiCol_FrameBg] = ImVec4(0, 0.5f, 0, 1);
  style.Colors[ImGuiCol_Text] = ImVec4(0, 0, 0, 1);
  ImGui::ProgressBar(hp, ImVec2(400.0f, 20.0f), "");
  const ImVec2 offset[] = { { -1, -1 }, { 1, -1 }, { 1, 1 }, { -1, 1} };
  for (int i = 0; i < 4; ++i) {
    ImGui::SetCursorPos(ImVec2(pos.x + 200 - 13 * 2.5f + offset[i].x, pos.y + offset[i].y));
    ImGui::Text(text);
  }
  style.Colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);
  ImGui::SetCursorPos(ImVec2(pos.x + 200 - 13 * 2.5f, pos.y));
  ImGui::Text(text);
  ImGui::End();
  style = oldStyle;
#endif
}

/**
* タイトル画面の更新
*/
void GameManager::UpdateTitle(float deltaTime)
{
  switch (titleState) {
  case TitleState::init:
    GameEngine::Get().ClearAllActors();
#ifdef USE_EASY_AUDIO
    Audio::Play(AUDIO_PLAYER_ID_BGM, BGM_TITLE, 1.0f, true);
#endif // USE_EASY_AUDIO
    titleLogoAlpha = 0;
    titleBgAlpha = 0;
    fadeAlpha = 0;
    titleState = TitleState::logoFadein;
    break;

  case TitleState::logoFadein:
    titleLogoAlpha += deltaTime;
    if (titleLogoAlpha >= 1.0f) {
      titleState = TitleState::bgFadein;
    }
    break;

  case TitleState::bgFadein:
    titleBgAlpha += deltaTime * 0.5f;
    if (titleBgAlpha >= 1.0f) {
      titleState = TitleState::idle;
    }
    break;

  case TitleState::idle:
    // 何もしない(UI操作待ち)
    break;

  case TitleState::fadeout:
#ifdef USE_EASY_AUDIO
    Audio::SetVolume(AUDIO_PLAYER_ID_BGM, std::max(0.0f, Audio::GetVolume(AUDIO_PLAYER_ID_BGM) - deltaTime));
#endif
    fadeAlpha += deltaTime;
    if (fadeAlpha > 1) {
      // スコアを初期化
      score = 0;

      titleState = TitleState::init;

#ifdef USE_EASY_AUDIO
      Audio::Stop(AUDIO_PLAYER_ID_BGM);
#endif // USE_EASY_AUDIO
      state = State::start;
    }
    break;
  }
}

/**
* タイトル画面UIの更新
*/
void GameManager::UpdateTitleUI()
{
  using namespace ImGui;

  GameEngine& engine = GameEngine::Get();
  ImGuiStyle& style = GetStyle();
  const ImGuiStyle styleBackup = style;
  ImDrawList* drawList = GetBackgroundDrawList();

  // 何度も使う値は定数として定義しておく
  const ImVec2 screenMin(0, 0);
  const ImVec2 screenMax(engine.GetWindowSize().x, engine.GetWindowSize().y);
  const ImVec2 uv0(0, 1);
  const ImVec2 uv1(1, 0);

  // 黒背景
  drawList->AddRectFilled(screenMin, screenMax, ImColor(0, 0, 0));

  // 背景
  std::shared_ptr<Texture> texBg = engine.LoadTexture("Res/title/title_bg.tga");
  drawList->AddImage(texBg->GetIdByPtr(), screenMin, screenMax,
    uv0, uv1, ImColor(0.4f, 0.35f, 0.3f, titleBgAlpha));

  // エフェクト
  std::shared_ptr<Texture> texEffect =
    engine.LoadTexture("Res/title/title_effect.tga");
  glTextureParameteri(texEffect->GetId(), GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTextureParameteri(texEffect->GetId(), GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  drawList->AddCallback(
    [](const ImDrawList*, const ImDrawCmd*) { glBlendFunc(GL_SRC_ALPHA, GL_ONE); },
    nullptr);
  const float effectSpeed[2] = { 1.0f, 0.3f };
  for (int i = 0; i < std::size(titleEffectPosX); ++i) {
    titleEffectPosX[i] = fmod(titleEffectPosX[i] + effectSpeed[i], screenMax.x * 2);
    drawList->AddImage(texEffect->GetIdByPtr(),
      ImVec2(screenMin.x - titleEffectPosX[i], screenMin.y),
      ImVec2(screenMax.x * 3 - titleEffectPosX[i], screenMax.y),
      ImVec2(0, 1 + static_cast<float>(i)), ImVec2(3 + 3 * static_cast<float>(i), 0),
      ImColor(1.0f, 0.9f, 0.8f, 0.5f * titleBgAlpha));
  }
  drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);

  // ロゴ
  std::shared_ptr<Texture> texLogo = engine.LoadTexture("Res/title/title_logo.tga");
  const glm::vec2 logoSize(texLogo->GetWidth(), texLogo->GetHeight());
  const float cx = (screenMin.x + screenMax.x) * 0.5f;
  drawList->AddImage(texLogo->GetIdByPtr(),
    ImVec2(cx - logoSize.x * 0.5f, 100),
    ImVec2(cx + logoSize.x * 0.5f, 100 + logoSize.y), uv0, uv1,
    ImColor(1.0f, 1.0f, 1.0f, titleLogoAlpha));

  if (titleState == TitleState::idle) {
    const ImVec2 buttonSize(320, 64);
    SetNextWindowPos(ImVec2(cx - buttonSize.x * 0.5f, 500));
    Begin("Start", nullptr,
      ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);
    style.Colors[ImGuiCol_Button] = ImVec4(0.5f, 0.5f, 0.55f, 0.8f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.7f, 0.7f, 0.75f, 0.8f);
    //style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.5f, 0.5f, 1.0f, 0.8f);
    style.Colors[ImGuiCol_Text] = ImVec4(0, 0, 0, 1);
    //style.Colors[ImGuiCol_Border] = ImVec4(0.5f, 0.5f, 0.5f, 1);
    //style.Colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 1);
    style.FrameRounding = 12;
    style.FrameBorderSize = 4;
    //style.ItemSpacing = ImVec2(8, 16);
    SetWindowFontScale(4.0f);
    if (Button(u8"ゲーム開始", buttonSize)) {
#ifdef USE_EASY_AUDIO
      Audio::PlayOneShot(SE_OK);
#endif // USE_EASY_AUDIO
      titleState = TitleState::fadeout;
    }
#ifdef USE_EASY_AUDIO
    if (IsItemHovered()) {
      if (!startHovered) {
        startHovered = true;
        Audio::PlayOneShot(SE_SELECT);
      }
    } else {
      startHovered = false;
    }
#endif // USE_EASY_AUDIO
    if (Button(u8"終了", buttonSize)) {
#ifdef USE_EASY_AUDIO
      Audio::PlayOneShot(SE_CANCEL);
#endif // USE_EASY_AUDIO
      engine.SetWindowShouldClose(true);
    }
#ifdef USE_EASY_AUDIO
    if (IsItemHovered()) {
      if (!exitHovered) {
        exitHovered = true;
        Audio::PlayOneShot(SE_SELECT);
      }
    } else {
      exitHovered = false;
    }
#endif // USE_EASY_AUDIO
    style = styleBackup;
    End();
  }

  // フェードアウト用の前景
  GetForegroundDrawList()->AddRectFilled(screenMin, screenMax,
    ImColor(0.0f, 0.0f, 0.0f, fadeAlpha));
}

/**
* エンディングを初期化する
*/
void GameManager::InitializeEnding()
{
  endingText.clear();

  // エンディング用テキストを読み込む
  std::ifstream ifs("Res/ending.txt");
  if (ifs) {
    while (!ifs.eof()) {
      std::string line;
      std::getline(ifs, line);
      uint32_t color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
      // 先頭に@があったら文字色を変える
      if (line.size() > 0 && line[0] == '@') {
        color = ImColor(0.7f, 0.8f, 1.0f, 1.0f);
        line.erase(line.begin()); // @を除去
      }
      endingText.push_back(EndingText{ color, line });
    }
  } else {
    // 読み込みに失敗したのでエラーメッセージを設定
    const uint32_t color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
    endingText.push_back(EndingText{ color, u8"ending.txtを開けませんでした" });
    endingText.push_back(EndingText{ color, u8"ファイル名を確認してください" });
  }

  // フォントサイズを設定
  const float defaultFontPixels = 13.0f; // ImGui標準のフォントサイズ(ピクセル)
  fontSize = defaultFontPixels * 4.0f;   // 適当なサイズを設定

  // 画面下をスクロール開始位置に設定
  GameEngine& engine = GameEngine::Get();
  const glm::vec2 windowSize = engine.GetWindowSize();
  endingPosY = windowSize.y;

  isScrollFinished = false; // スクロール開始
  fadeAlpha = 0; // フェードアウト無効

  // プレイヤーを操作不能にする
  if (playerTank) {
    static_cast<PlayerActor&>(*playerTank).SetControlFlag(false);
  }

  // TODO: ここでBGMを設定
#ifdef USE_EASY_AUDIO
  //Audio::Play(AUDIO_PLAYER_ID_BGM, BGM_ENDING);
#endif // USE_EASY_AUDIO

  // ゲーム状態を「エンディング」にする
  SetState(State::ending);
}

/**
* エンディングの状態を更新する
*/
void GameManager::UpdateEnding(float deltaTime)
{
  GameEngine& engine = GameEngine::Get();

  // スクロール処理
  if (!isScrollFinished) {
    const float speed = 8.0f; // スクロール速度(秒毎スクリーン)
    const glm::vec2 windowSize = engine.GetWindowSize();
    endingPosY -= (windowSize.y / speed) * deltaTime;

    // 最後の行の位置が、画面中央に到達したらスクロールを止める
    const float lastY = endingPosY + endingText.size() * fontSize;
    if (lastY <= windowSize.y * 0.5f) {
      isScrollFinished = true; // スクロール停止
    }
  }

  // フェードアウトが開始されていなければ、キー入力を受け付ける
  if (fadeAlpha <= 0) {
    if (engine.GetKey(GLFW_KEY_ENTER)) {
      fadeAlpha += deltaTime;
    }
  } else {
    // フェードアウトが完了したらタイトル画面に戻る
    fadeAlpha += deltaTime;
    if (fadeAlpha >= 1) {
      engine.ClearAllActors();
      endingText.clear();

      // タイトル画面に戻る
      SetState(State::title);
    }
  }
}

/**
* エンディング画面UIの更新
*/
void GameManager::UpdateEndingUI()
{
  using namespace ImGui;

  GameEngine& engine = GameEngine::Get();
  const glm::vec2 windowSize = engine.GetWindowSize();

  // 背景を徐々に消していく
  const float alpha = 1.0f - glm::max(endingPosY / windowSize.y, 0.0f);
  GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0),
    ImVec2(windowSize.x, windowSize.y), ImColor(0.0f, 0.0f, 0.0f, alpha));

  // テキストを表示
  ImFont* font = GetFont();
  ImDrawList* drawList = GetForegroundDrawList();
  ImVec2 pos(0, endingPosY);
  ImVec2 posShadow(0, endingPosY + fontSize * 0.05f);
  const ImU32 colorShadow = ImColor(0.3f, 0.3f, 0.3f, 0.8f);
  for (const auto& e : endingText) {
    // テキストが1文字以上ある場合だけ描画する
    if (e.text.size() >= 1) {
      // 表示位置が画面の上端より下、下端より上の場合だけ描画する
      if (pos.y >= -fontSize && pos.y <= windowSize.y) {
        const char* textBegin = e.text.data();
        const char* textEnd = textBegin + e.text.size();
        // テキストを中央寄せで表示
        const ImVec2 textSize = font->CalcTextSizeA(
          fontSize, FLT_MAX, -1.0f, textBegin, textEnd);
        pos.x = (windowSize.x - textSize.x) * 0.5f;
        posShadow.x = pos.x + fontSize * 0.05f;
        drawList->AddText(font, fontSize, posShadow,
          colorShadow, textBegin, textEnd);
        drawList->AddText(font, fontSize, pos,
          e.color, textBegin, textEnd);
      }
    }
    // 次の表示位置を設定
    pos.y += fontSize;
    posShadow.y += fontSize;
  }

  // タイトルに戻るボタンを表示
  if (isScrollFinished && fadeAlpha <= 0) {
    Begin("button", nullptr,
      ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);

    // ボタンのスタイルを設定
    PushStyleVar(ImGuiStyleVar_FrameRounding, 12);
    PushStyleVar(ImGuiStyleVar_FrameBorderSize, 4);
    ImDrawList* drawList = GetWindowDrawList();
    SetWindowFontScale(3.0f);
    
    // ボタンを右下端に表示
    const char text[] = u8" タイトルに戻る ";
    const ImVec2 textSize = CalcTextSize(text);
    const ImVec2 buttonSize(textSize.x + 32, textSize.y + 24);
    SetWindowPos(ImVec2(
      windowSize.x * 0.95f - buttonSize.x,
      windowSize.y * 0.95f - buttonSize.y));
    if (Button(text, buttonSize)) {
      fadeAlpha = 0.0001f; // ボタンが押されたらフェードアウト開始
    }

    PopStyleVar(2); // スタイルを元に戻す
    End();
  }

  // フェードアウト
  if (fadeAlpha > 0) {
    drawList->AddRectFilled(ImVec2(0, 0),
      ImVec2(windowSize.x, windowSize.y), ImColor(0.0f, 0.0f, 0.0f, fadeAlpha));
  }
}

/**
* フラグの総数を取得する
*/
size_t GameManager::GetGameFlagCount() const
{
  return gameFlags.size();
}

/**
* フラグの総数を設定する
*/
void GameManager::SetGameFlagCount(size_t size)
{
  gameFlags.resize(size);
}

/**
* フラグの値を取得する
*/
bool GameManager::GetGameFlag(size_t no) const
{
  if (no >= gameFlags.size()) {
    std::cerr << "[警告]" << __func__ << "添字" << no <<
      "は範囲" << gameFlags.size() << "を超えています\n";
    return false;
  }
  return gameFlags[no].value;
}

/**
* フラグに値を設定する
*/
void GameManager::SetGameFlag(size_t no, bool value)
{
  if (no >= gameFlags.size()) {
    std::cerr << "[警告]" << __func__ << "添字" << no <<
      "は範囲" << gameFlags.size() << "を超えています\n";
    return;
  }
  gameFlags[no].value = value;
}

/**
* フラグの説明を取得する
*/
std::string GameManager::GetGameFlagDesc(size_t no) const
{
  if (no >= gameFlags.size()) {
    std::cerr << "[警告]" << __func__ << "添字" << no <<
      "は範囲" << gameFlags.size() << "を超えています\n";
    return "";
  }
  return gameFlags[no].description;
}

/**
* フラグの説明を設定する
*/
void GameManager::SetGameFlagDesc(size_t no, std::string desc)
{
  if (no >= gameFlags.size()) {
    std::cerr << "[警告]" << __func__ << "添字" << no <<
      "は範囲" << gameFlags.size() << "を超えています\n";
    return;
  }
  gameFlags[no].description = desc;
}

/**
* タグが「ステージクリアに必要な破壊対象」かどうかを取得する
*/
bool GameManager::GetTargetFlag(ActorTag tag) const
{
  return targetTagFlags[static_cast<int>(tag)];
}

/**
* タグが「ステージクリアに必要な破壊対象」かどうかを設定する
*/
void GameManager::SetTargetFlag(ActorTag tag, bool flag)
{
  targetTagFlags[static_cast<int>(tag)] = flag;
}

/**
* 全てのタグを「ステージクリアに必要な破壊対象」から除外する
*/
void GameManager::ClearAllTargetFlags()
{
  for (auto& e : targetTagFlags) {
    e = false;
  }
}

/**
* 描画データを追加する.
*/
void GameManager::LoadPrimitives()
{
  PrimitiveBuffer& primitiveBuffer = GameEngine::Get().GetPrimitiveBuffer();

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
  primitiveBuffer.AddFromObjFile("Res/Plane.obj");
}

/**
* テクスチャを作成.
*/
void GameManager::LoadTextures()
{
  GameEngine& engine = GameEngine::Get();
  engine.LoadTexture("Res/RoadTiles.tga");
  engine.LoadTexture("Res/Triangle.tga");
  engine.LoadTexture("Res/Green.tga");
  engine.LoadTexture("Res/Road.tga");
  engine.LoadTexture("Res/Tree.tga");
  engine.LoadTexture("Res/Building.tga");
  engine.LoadTexture("Res/tank/PzVl_Tiger_I.tga");
  engine.LoadTexture("Res/tank/T-34.tga");
  engine.LoadTexture("Res/house/House38UVTexture.tga");
  engine.LoadTexture("Res/Bullet.tga");
  engine.LoadTexture("Res/house/broken-house.tga");
}

/**
* プレイヤーの戦車を生成する
*/
void GameManager::SpawnPlayer()
{
  // 以前に作成したタイガーI戦車を削除
  if (playerTank) {
    playerTank->isDead = true;
  }

  // 新しいタイガーI戦車を作成
  playerTank.reset(new PlayerActor{ glm::vec3(0), glm::vec3(1), 0.0f });

  // タイガーI戦車をゲームエンジンに登録
  GameEngine::Get().AddActor(playerTank);
}

/**
* 敵戦車を生成
*/
void GameManager::SpawnEnemies()
{
  GameEngine& engine = GameEngine::Get();

  // 以前に作成したT-34戦車を削除
  for (int i = 0; i < enemies.size(); ++i) {
    if (enemies[i]) {
      enemies[i]->isDead = true;
    }
  }
  enemies.clear();

  // 新しいT-34戦車を作成
  const glm::vec3 t34PosList[] = {
    glm::vec3(-5, 0, 0),
    glm::vec3(15, 0, 0),
    glm::vec3(-10, 0, -5),
  };
  for (auto& pos : t34PosList) {
    std::string name("T-34[");
    name += '0' + static_cast<char>(&pos - t34PosList);
    name += ']';
    std::shared_ptr<Actor> enemy(new T34TankActor{ name.c_str(),
      engine.GetPrimitive("Res/tank/T34.obj"),
      engine.LoadTexture("Res/tank/t-34.tga"),
      pos, glm::vec3(1), 0.0f, glm::vec3(-0.78f, 0, 1.0f), playerTank });
    //enemy->collider = CreateBoxCollider(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2.5f, 1.5f));
    enemy->collider = std::make_shared<Cylinder>(glm::vec3(0), 1.5f, 2.5f);
    enemy->mass = 36'000;
    enemies.push_back(enemy);
  }

  // T-34戦車をゲームエンジンに登録
  for (int i = 0; i < enemies.size(); ++i) {
    engine.AddActor(enemies[i]);
  }
}

/**
* 草を生やす
*/
void GameManager::SpawnGrass()
{
  GameEngine& engine = GameEngine::Get();
  const glm::ivec2 mapSize = engine.GetMapSize();

  // 草のインスタンシング用レンダラを作成
  InstancedMeshRendererPtr grassRenderer =
    std::make_shared<InstancedMeshRenderer>(mapSize.x * mapSize.y);
  grassRenderer->SetMesh(engine.LoadMesh("Res/grass.obj"));
  grassRenderer->SetMaterial(0,
    { "grass", glm::vec4(1), engine.LoadTexture("Res/grass.tga") });
  std::vector<InstancedMeshRenderer::InstanceData> matGrassList(mapSize.x * mapSize.y);
  for (int y = 0; y < mapSize.y; ++y) {
    for (int x = 0; x < mapSize.x; ++x) {
      glm::vec3 pos(x * 4 - mapSize.x * 2 + 2, 0, y * 4 - mapSize.y * 2 + 2);
      const int i = y * mapSize.x + x;
      matGrassList[i].matModel = glm::translate(glm::mat4(1), pos);
      matGrassList[i].color.r = engine.GetRandomFloat(0.666f, 1.0f);
      matGrassList[i].color.g = engine.GetRandomFloat(0.666f, 1.0f);
      matGrassList[i].color.b = engine.GetRandomFloat(0.666f, 1.0f);
      matGrassList[i].color.a = 1;

      matGrassList[y * mapSize.x + x].matModel *= glm::scale(glm::mat4(1),
        glm::vec3(1, engine.GetRandomFloat(0.5f, 1.2f), 1));
    }
  }
  grassRenderer->UpdateInstanceData(matGrassList.size(), matGrassList.data());

  auto grassActor = std::make_shared<Actor>("grass");
  grassActor->renderer = grassRenderer;
  grassActor->shader = Shader::InstancedMesh;
  engine.AddActor(grassActor);
}

/**
*
*/
void GameManager::SpawnMap()
{
  // マップに配置する物体の表示データ.
  struct ObjectData {
    const char* name;
    Primitive prim;
    const std::shared_ptr<Texture> tex;
    float scale = 1.0f;
    glm::vec3 ajustment = glm::vec3(0);
    std::shared_ptr<Collider> collider;
  };

  GameEngine& engine = GameEngine::Get();

  // 画面端にコライダーを設定
  ActorList& actors = engine.GetActors();
  PrimitiveBuffer& primitiveBuffer = engine.GetPrimitiveBuffer();
  std::shared_ptr<Texture> texTriangle = engine.LoadTexture("Res/Triangle.tga");

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(-36, 0, -34), glm::vec3(1, 2, 32), 0.0f, glm::vec3(0) }));
  actors.back()->collider = Box::Create(glm::vec3(0, 0, 0), glm::vec3(1, 4, 64));
  actors.back()->isStatic = true;

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(30, 0, -34), glm::vec3(1, 2, 32), 0.0f, glm::vec3(0) }));
  actors.back()->collider = Box::Create(glm::vec3(0, 0, 0), glm::vec3(1, 4, 64));
  actors.back()->isStatic = true;

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(-34, 0, -36), glm::vec3(32, 2, 1), 0.0f, glm::vec3(0) }));
  actors.back()->collider = Box::Create(glm::vec3(0, 0, 0), glm::vec3(64, 4, 1));
  actors.back()->isStatic = true;

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(-34, 0, 30), glm::vec3(32, 2, 1), 0.0f, glm::vec3(0) }));
  actors.back()->collider = Box::Create(glm::vec3(0, 0, 0), glm::vec3(64, 4, 1));
  actors.back()->isStatic = true;

  // 描画する物体のリスト.
  //std::shared_ptr<Box> col1 = CreateBoxCollider(glm::vec3(-1.75f, 0, -1.75f), glm::vec3(1.75f, 2, 1.75f));
  std::shared_ptr<Cylinder> col1 = Cylinder::Create(glm::vec3(0), 1.75f, 3.0f);
  const ObjectData objectList[] = {
    { "", Primitive(), 0 },    // なし
    { "Tree", primitiveBuffer.Get(4), engine.LoadTexture("Res/Tree.tga"), 1, {}, col1 }, // 木
    { "Warehouse", primitiveBuffer.Get(5), engine.LoadTexture("Res/Building.tga"), 1, {},
      Box::Create(glm::vec3(-2, 0, -2), glm::vec3(2, 3, 2)) }, // 建物
    { "BrickHouse", primitiveBuffer.Get(8), engine.LoadTexture("Res/house/House38UVTexture.tga"),
      3, glm::vec3(-2.6f, 2.0f, 0.8f), Box::Create(glm::vec3(-3, 0, -2), glm::vec3(3, 3, 2)) }, // 建物
    { "House2", primitiveBuffer.Get(10), engine.LoadTexture("Res/house/broken-house.tga"),
      1, {}, Box::Create(glm::vec3(-2.5f, 0, -3.5f), glm::vec3(2.5f, 3, 3.5f)) }, // 建物
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
      actors.back()->collider = p.collider;
      actors.back()->isStatic = true;
      actors.back()->mass = 1'000'000;
    }
  }

  // マップを(-20,-20)-(20,20)の範囲に描画.
  const std::shared_ptr<Texture> mapTexList[] = {
    engine.LoadTexture("Res/Green.tga"),
    engine.LoadTexture("Res/RoadTiles.tga"),
    engine.LoadTexture("Res/Road.tga") };
  for (int y = 0; y < 16; ++y) {
    for (int x = 0; x < 16; ++x) {
      // 四角形が4x4mなので、xとyを4倍した位置に表示する.
      const glm::vec3 position(x * 4 - 32, 0, y * 4 - 32);

      const int textureNo = mapData[y][x];
      actors.push_back(std::shared_ptr<Actor>(new Actor{ "Ground", primitiveBuffer.Get(0), mapTexList[textureNo],
        position, glm::vec3(1), 0.0f, glm::vec3(0) }));
      actors.back()->collider = Box::Create(glm::vec3(-2, -10, -2), glm::vec3(2, 0, 2));
      actors.back()->isStatic = true;
    }
  }

  // エレベーター
  {
    const glm::vec3 position(4 * 4 - 20, -2, 4 * 4 - 20);
    actors.push_back(std::shared_ptr<Actor>(new ElevatorActor{
      "Elevator", primitiveBuffer.Get(0), mapTexList[0],
      position, glm::vec3(1), 0.0f, glm::vec3(0) }));
    actors.back()->velocity.y = 1;
    actors.back()->collider = Box::Create(glm::vec3(-2, -10, -2), glm::vec3(2, 0, 2));
    actors.back()->isStatic = true;
  }

  // 三角形のパラメータ
  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Triangle", primitiveBuffer.Get(2), texTriangle,
    glm::vec3(0, 0, -5), glm::vec3(1), 0.0f, glm::vec3(0) }));
  // 立方体のパラメータ
  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Cube", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(0, 0, -4), glm::vec3(1), 0.0f, glm::vec3(0) }));
}

