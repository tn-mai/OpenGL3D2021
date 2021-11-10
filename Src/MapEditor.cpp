/**
* @file MapEditor.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "MapEditor.h"
#include "Actor/PlayerActor.h"
#include "Actor/T34TankActor.h"
#include "Actor/Boss01.h"
#include "GameEngine.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <fstream>
#include <iostream>

/**
* スクリーン座標を始点とし、画面の奥へ向かう線分を計算する
*
* @param screenPos スクリーン座標
* @param matVP     ビュープロジェクション行列
* @param start     視線の始点が代入される
* @param end       視線の終点が代入される
*/
void ScreenPosToLine(const ImVec2& screenPos, const glm::mat4& matVP, glm::vec3& start, glm::vec3& end)
{
  // スクリーン座標をNDC座標に変換
  const glm::vec2 windowSize = GameEngine::Get().GetWindowSize();
  glm::vec4 ndcPos(screenPos.x / windowSize.x * 2.0f - 1.0f,
    1.0f - screenPos.y / windowSize.y * 2.0f, -1, 1);

  // 視線の始点座標を計算
  const glm::mat4 matInvVP = glm::inverse(matVP);
  glm::vec4 worldPos0 = matInvVP * ndcPos;
  start = worldPos0 / worldPos0.w;

  // 視線の終点座標を計算
  ndcPos.z = 1;
  glm::vec4 worldPos1 = matInvVP * ndcPos;
  end = worldPos1 / worldPos1.w;
}

/**
* 線分と平面が交差する座標を求める
*
* @param start  線分の始点
* @param end    線分の終点
* @param q      平面上の任意の点
* @param normal 平面の法線
* @param p      交点の座標が代入される
*
* @retval true  交差している
* @retval false 交差していない
*/
bool Intersect(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& q, const glm::vec3& normal, glm::vec3& p)
{
  const float distance = glm::dot(normal, q - p0);
  const glm::vec3 v = p1 - p0;

  // 分母がほぼ0の場合、線分は平面と平行なので交差しない.
  const float denom = glm::dot(normal, v);
  if (std::abs(denom) < 0.0001f) {
    return false;
  }

  // 交点までの距離tが0未満または1より大きい場合、交点は線分の外側にあるので実際には交差しない.
  const float t = distance / denom;
  if (t < 0 || t > 1) {
    return false;
  }

  // 交点は線分上にある.
  p = p0 + v * t;
  return true;
}

/**
* コンストラクタ
*/
MapEditor::MapEditor(SystemType type) : systemType(type)
{
  GameEngine& engine = GameEngine::Get();

  // 既存のアクターをすべて削除
  engine.ClearAllActors();

  LoadCommonPrimitive();
  InitActorList();
  if (systemType == SystemType::editor) {
    InitGroundActor();
    InitEditor();
  }
}

/**
* 汎用プリミティブを読み込む
*/
void MapEditor::LoadCommonPrimitive()
{
  GameEngine& engine = GameEngine::Get();

  engine.LoadPrimitive("Res/Plane.obj");
  engine.LoadPrimitive("Res/Bullet.obj");
}

/**
* 地面用アクターを作成
*/
void MapEditor::InitGroundActor()
{
  GameEngine& engine = GameEngine::Get();

  // アクター配置マップのサイズをマップサイズに合わせる
  map.resize(mapSize.x * mapSize.y);

  const char* texList[] = {
    "Res/Green.tga",
    "Res/Road.tga",
    "Res/RoadTiles.tga",
  };

  // 地面用アクターを作成
  engine.LoadPrimitive("Res/Ground.obj");
  std::shared_ptr<Actor> groundActor(new Actor("Ground",
    engine.GetPrimitive("Res/Ground.obj"),
    engine.LoadTexture("GroundTiles", texList, std::size(texList)),
    glm::vec3(0), glm::vec3(mapSize.x, 1, mapSize.y), 0, glm::vec3(0)));
  groundActor->shader = Shader::Ground;
  groundActor->isStatic = true;
  const glm::vec2 colliderSize = glm::vec2(mapSize) * gridSize * 0.5f;
  groundActor->collider = Box::Create(
    glm::vec3(-colliderSize.x, -10.0f, -colliderSize.y),
    glm::vec3(colliderSize.x, 0.0f, colliderSize.y));
  engine.AddActor(groundActor);

  // マップデータテクスチャ操作用の変数を初期化
  if (systemType == SystemType::editor) {
    groundMap.resize(mapSize.x * mapSize.y, 0);
    groundTiles.clear();
    groundTiles.reserve(std::size(texList));
    for (const char* filename : texList) {
      groundTiles.push_back(engine.LoadTexture(filename));
    }
  }
}

/**
* 配置用アクターを作成
*/
void MapEditor::InitActorList()
{
  GameEngine& engine = GameEngine::Get();

  actors.clear();

  // アクターの型
  enum class ActorType {
    player,
    t34tank,
    boss01,
    other,
  };

  // 配置用アクターを作成
  struct ObjectData {
    ActorType type;
    const char* name;
    const char* primitiveFilename;
    const char* textureFilename;
    std::shared_ptr<Collider> collider;
    glm::vec3 scale = glm::vec3(1);
    float rotation = 0;
    glm::vec3 adjustment = glm::vec3(0);
  };
  const ObjectData objectList[] = {
    {
      ActorType::other, "Cube",
      "Res/Cube.obj", "Res/Triangle.tga",
      Box::Create(glm::vec3(0, 0, 0), glm::vec3(2, 2, 2)) },
    {
      ActorType::other, "Tree",
      "Res/Tree.obj", "Res/Tree.tga",
      Box::Create(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2, 1.5f)) },
    {
      ActorType::other, "Tree2",
      "Res/tree/fir.obj", "Res/tree/branch.tga",
      Box::Create(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2, 1.5f)) },
    {
      ActorType::other, "Tree3",
      "Res/tree/LowPolyTree.obj", "Res/tree/LowPolyLeaves.tga",
      Box::Create(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2, 1.5f)) },
    {
      ActorType::other, "Tree4",
      "Res/tree/Tree_Conifer_1.obj", "Res/tree/Fantasy_conifer_1.tga",
      Box::Create(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2, 1.5f)), glm::vec3(0.02f) },
    {
      ActorType::other, "House-1-5(2)",
      "Res/house/test/House-1-5.obj", "Res/house/test/Houses Colorscheme 2.tga",
      Box::Create(glm::vec3(-3, 0, -5), glm::vec3(3, 8, 5)) },
    {
      ActorType::other, "House-1-5(3)",
      "Res/house/test/House-1-5.obj", "Res/house/test/Houses Colorscheme 3.tga",
      Box::Create(glm::vec3(-3, 0, -5), glm::vec3(3, 8, 5)) },
    {
      ActorType::other, "House-1-5(4)",
      "Res/house/test/House-1-5.obj", "Res/house/test/Houses Colorscheme 4.tga",
      Box::Create(glm::vec3(-3, 0, -5), glm::vec3(3, 8, 5)) },
    {
      ActorType::other, "House-1-5(5)",
      "Res/house/test/House-1-5.obj", "Res/house/test/Houses Colorscheme 5.tga",
      Box::Create(glm::vec3(-3, 0, -5), glm::vec3(3, 8, 5)) },
    {
      ActorType::other, "Lamppost",
      "Res/house/test/Lamppost.obj", "Res/house/test/Houses Colorscheme.tga",
      Box::Create(glm::vec3(-0.5f, 0, -0.5f), glm::vec3(0.5f, 2, 0.5f)) },
    {
      ActorType::other, "Bench",
      "Res/house/test/Bench.obj", "Res/house/test/Houses Colorscheme.tga",
      Box::Create(glm::vec3(-0.5f, 0, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f)) },
    {
      ActorType::other, "Chair",
      "Res/house/test/Chair.obj", "Res/house/test/Houses Colorscheme.tga",
      Box::Create(glm::vec3(-0.5f, 0, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f)) },
    {
      ActorType::other, "Warehouse",
      "Res/Warehouse.obj", "Res/Building.tga",
      Box::Create(glm::vec3(-2, 0, -2), glm::vec3(2, 3, 2)), glm::vec3(1.0f) },

    { ActorType::other, "BrickHouse",
      "Res/house/HouseRender.obj", "Res/house/House38UVTexture.tga",
      Box::Create(glm::vec3(-3, 0, -2), glm::vec3(3, 3, 2)),
      glm::vec3(2.0f), 0, glm::vec3(-2.6f, 2.0f, 0.8f) },

    { ActorType::other, "BrokenHouse",
      "Res/house/broken-house.obj", "Res/house/broken-house.tga",
      Box::Create(glm::vec3(-2, 0, -2), glm::vec3(2, 2, 2)),
      glm::vec3(1.00f) },

    { ActorType::t34tank, "T-34",
      "Res/tank/T34.obj", "Res/tank/T-34.tga",
      Box::Create(glm::vec3(-1, 0, -1), glm::vec3(1, 2, 1)) },

    { ActorType::boss01, "Boss01",
      "Res/Black_Track.obj", "Res/Black_Track.tga",
      Box::Create(glm::vec3(-3, 0, -3), glm::vec3(3, 2.5f, 3)) },

    { ActorType::player, "Tiger-I",
      "Res/tank/Tiger_I.obj", "Res/tank/PzVl_Tiger_I.tga",
      Box::Create(glm::vec3(-1, 0, -1), glm::vec3(1, 2, 1)) },
  };
  for (const auto& e : objectList) {
    engine.LoadPrimitive(e.primitiveFilename);
    std::shared_ptr<Actor> actor;
    switch (e.type) {
    case ActorType::player:
      actor.reset(new PlayerActor(glm::vec3(0), e.scale, e.rotation));
      break;

    case ActorType::t34tank:
      actor.reset(new T34TankActor(
        e.name,
        engine.GetPrimitive(e.primitiveFilename),
        engine.LoadTexture(e.textureFilename),
        glm::vec3(0), e.scale, e.rotation, e.adjustment, nullptr));
      actor->collider = e.collider;
      actor->mass = 36'000;
      break;

    case ActorType::boss01:
      actor.reset(new Boss01(
        glm::vec3(0), e.scale, e.rotation, nullptr));
      actor->collider = e.collider;
      break;

    case ActorType::other:
      actor.reset(new Actor(
        e.name,
        engine.GetPrimitive(e.primitiveFilename),
        engine.LoadTexture(e.textureFilename),
        glm::vec3(0), e.scale, e.rotation, e.adjustment));
      actor->collider = e.collider;
      actor->isStatic = true;
      break;
    }
    actors.push_back(actor);
  }
}

/**
* エディタを動かすために必要な変数を初期化する
*/
void MapEditor::InitEditor()
{
  GameEngine& engine = GameEngine::Get();

  // 位置表示アクターを作成
  cursor.reset(new Actor(*actors[0]));
  cursor->color = glm::vec4(0.2f, 0.5f, 1.0f, 0.5f);
  engine.AddActor(cursor);

  // カメラを設定
  Camera& camera = engine.GetCamera();
  camera.target = glm::vec3(0, 0, 0);
  camera.position = camera.target + cameraOffset;
}

/**
* マップサイズを変更する
*/
void MapEditor::Resize(const  glm::ivec2& newMapSize)
{
  if (mapSize.x == newMapSize.x && mapSize.y == newMapSize.y) {
    return;
  }

  GameEngine& engine = GameEngine::Get();

  // 地面モデルの大きさを変更
  std::shared_ptr<Actor> groundActor = engine.FindActor("Ground");
  if (groundActor) {
    groundActor->scale = glm::vec3(newMapSize.x, 1.0f, newMapSize.y);
    const glm::vec2 colliderSize = glm::vec2(newMapSize) * gridSize * 0.5f;
    groundActor->collider = Box::Create(
      glm::vec3(-colliderSize.x, -10.0f, -colliderSize.y),
      glm::vec3(colliderSize.x, 0.0f, colliderSize.y));
  }

  // マップデータテクスチャの大きさを変更
  std::vector<uint32_t> newGroundMap(newMapSize.x * newMapSize.y, 0);
  for (int y = 0; y < std::min(newMapSize.y, mapSize.y); ++y) {
    for (int x = 0; x < std::min(newMapSize.x, mapSize.x); ++x) {
      newGroundMap[x + y * newMapSize.x] = groundMap[x + y * mapSize.x];
    }
  }
  groundMap.swap(newGroundMap);
  engine.ResizeGroundMap(newMapSize.x, newMapSize.y, groundMap.data());

  // マップデータの大きさを変更
  for (auto& e : map) {
    if (e) {
      e->isDead = true;
    }
  }
  std::vector<std::shared_ptr<Actor>> newMap(newMapSize.x * newMapSize.y);
  for (int y = 0; y < std::min(newMapSize.y, mapSize.y); ++y) {
    for (int x = 0; x < std::min(newMapSize.x, mapSize.x); ++x) {
      std::shared_ptr<Actor>& actor = map[x + y * mapSize.x];
      if (actor) {
        actor->isDead = false;
        actor->position.x = (x - newMapSize.x / 2) * gridSize.x;
        actor->position.z = (y - newMapSize.y / 2) * gridSize.y;
        newMap[x + y * newMapSize.x] = actor;
      }
    }
  }
  map.swap(newMap);
  mapSize = newMapSize;
}

/**
* マップエディタの状態を更新する
*/
void MapEditor::Update(float deltaTime)
{
  GameEngine& engine = GameEngine::Get();

  // ImGuiがマウス操作を処理している場合、マップエディタ側ではマウス操作を行わない
  if (ImGui::GetIO().WantCaptureMouse) {
    return;
  }

  // ESCキーが押されたら「選択モード」に戻る
  if (engine.GetKey(GLFW_KEY_ESCAPE)) {
    mode = Mode::select;
  }

  // 右クリック
  if (engine.GetMouseButton(GLFW_MOUSE_BUTTON_LEFT)) {
    // マップ範囲外の場合は何もしない
    const int x = static_cast<int>(glm::round(cursor->position.x / 4)) + mapSize.x / 2;
    const int y = static_cast<int>(glm::round(cursor->position.z / 4)) + mapSize.y / 2;
    if (x < 0 || x >= mapSize.x || y < 0 || y >= mapSize.y) {
      return;
    }

    // マップからカーソル位置に対応するアクターを取得
    std::shared_ptr<Actor>& target = map[x + y * mapSize.x];

    switch (mode) {
    case Mode::set:
      // 選択アクターと種類が同じ場合は配置しない
      if (target && target->name == cursor->name) {
        target->rotation = cursor->rotation;
        break;
      }
      // 既存のアクターがあれば削除する
      if (target) {
        target->isDead = true;
      }
      // 選択中のアクターを配置する
      target.reset(new Actor(*cursor));
      target->color = glm::vec4(1);
      engine.AddActor(target);
      break;

    case Mode::remove:
      // カーソル位置のアクターを削除
      if (target) {
        target->isDead = true;
        target.reset();
      }
      break;

    case Mode::groundPaint:
      groundMap[x + y * mapSize.x] = currentTileNo;
      engine.UpdateGroundMap(0, 0, mapSize.x, mapSize.y, groundMap.data());
      break;

    default: break;
    }
  }
}

/**
* カメラ状態を更新する
*/
void MapEditor::UpdateCamera(float deltaTime)
{
  GameEngine& engine = GameEngine::Get();
  Camera& camera = engine.GetCamera();

  float cursorSpeed = 20.0f;
  if (engine.GetKey(GLFW_KEY_LEFT_SHIFT) || engine.GetKey(GLFW_KEY_RIGHT_SHIFT)) {
    cursorSpeed = 40.0f;
  }
  if (engine.GetKey(GLFW_KEY_W)) {
    camera.target.z -= cursorSpeed * deltaTime;
  } else if (engine.GetKey(GLFW_KEY_S)) {
    camera.target.z += cursorSpeed * deltaTime;
  }
  if (engine.GetKey(GLFW_KEY_A)) {
    camera.target.x -= cursorSpeed * deltaTime;
  } else if (engine.GetKey(GLFW_KEY_D)) {
    camera.target.x += cursorSpeed * deltaTime;
  }
  camera.position = camera.target + cameraOffset;

  // カーソル位置を設定
  const glm::mat4 matVP = camera.GetProjectionMatrix() * camera.GetViewMatrix();
  glm::vec3 start, end, p;
  ScreenPosToLine(ImGui::GetMousePos(), matVP, start, end);
  if (Intersect(start, end, glm::vec3(0), glm::vec3(0, 1, 0), p)) {
    cursor->position = glm::round(p / 4.0f) * 4.0f;
  }
}

/**
* UIを更新する
*/
void MapEditor::UpdateUI()
{
  using namespace ImGui;
  GameEngine& engine = GameEngine::Get();

  //ShowDemoWindow();

  Begin(u8"ツール");
  const char* toolName[] = { u8"選択", u8"配置", u8"削除", u8"地面ペイント" };
  const Mode modeList[] = { Mode::select, Mode::set, Mode::remove, Mode::groundPaint };
  for (int i = 0; i < std::size(toolName); ++i) {
    SameLine();
    if (Button(toolName[i])) {
      mode = modeList[i];
    }
  }
  SameLine();
  if (Button(u8"セーブ")) {
    Save(u8"mapdata.txt");
  }
  SameLine();
  if (Button(u8"ロード")) {
    Load("mapdata.txt");
  }
  SameLine();

  static glm::ivec2 newMapSize;
  if (Button(u8"新規作成")) {
    OpenPopup(u8"新しいマップの広さ");
    newMapSize = glm::ivec2(21, 21);
  }
  if (BeginPopupModal(u8"新しいマップの広さ")) {
    Text(u8"横");
    SameLine();
    SetNextItemWidth(-1);
    SliderInt("##map size x", &newMapSize.x, 11, 101, "%d", ImGuiSliderFlags_AlwaysClamp);
    newMapSize.x = newMapSize.x | 1;
    Text(u8"縦");
    SameLine();
    SetNextItemWidth(-1);
    SliderInt("##map size y", &newMapSize.y, 11, 101, "%d", ImGuiSliderFlags_AlwaysClamp);
    newMapSize.y = newMapSize.y | 1;
    if (Button(u8"このサイズで作成")) {
      // マップデータを消去
      mapSize = newMapSize;
      map = std::vector<std::shared_ptr<Actor>>(mapSize.x * mapSize.y);
      groundMap = std::vector<uint32_t>(mapSize.x * mapSize.y, 0);
      engine.ResizeGroundMap(mapSize.x, mapSize.y, groundMap.data());
      engine.ClearAllActors();
      InitGroundActor();
      InitEditor();
      CloseCurrentPopup();
    }
    SameLine();
    if (Button(u8"キャンセル")) {
      CloseCurrentPopup();
    }
    EndPopup();
  }

  SameLine();
  if (Button(u8"マップサイズ")) {
    OpenPopup(u8"マップサイズ変更");
    newMapSize = mapSize;
  }
  SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Once);
  if (BeginPopupModal(u8"マップサイズ変更")) {
    Text(u8"横");
    SameLine();
    SliderInt("##map size x", &newMapSize.x, 11, 101, "%d", ImGuiSliderFlags_AlwaysClamp);
    newMapSize.x = newMapSize.x | 1;
    Text(u8"縦");
    SameLine();
    SliderInt("##map size y", &newMapSize.y, 11, 101, "%d", ImGuiSliderFlags_AlwaysClamp);
    newMapSize.y = newMapSize.y | 1;
    if (Button("OK")) {
      Resize(newMapSize);
      CloseCurrentPopup();
    }
    SameLine();
    if (Button(u8"キャンセル")) {
      CloseCurrentPopup();
    }
    EndPopup();
  }

  Text(toolName[static_cast<int>(mode)]);
  End();

  SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Once);
  Begin(u8"アクター選択");
  const ImVec2 actorListBoxSize(-1,
    GetTextLineHeightWithSpacing() * actors.size() + GetStyle().FramePadding.y * 2);
  if (BeginListBox("ActorList", actorListBoxSize)) {
    for (int i = 0; i < actors.size(); ++i) {
      const bool isSelected = cursor == actors[i];
      if (Selectable(actors[i]->name.c_str(), isSelected)) {
        // エディタ上でコライダーの回転を可能にするため、クローンを作る
        cursorBase = actors[i];
        *cursor = *actors[i]->Clone();
        cursor->color = glm::vec4(0.2f, 0.5f, 1.0f, 0.5f);
      }
      if (isSelected) {
        SetItemDefaultFocus();
      }
    }
    EndListBox();
  }
  End();

  Begin(u8"アクター情報");
  Text(u8"名前: %s", cursor->name.c_str());
  Text(u8"回転:");
  SameLine();
  static int rotation = 0;
  static const char* strRotation[] = { "0", "90", "180", "270" };
  SliderInt("rotation", &rotation,
    0, static_cast<int>(std::size(strRotation)) - 1,
    strRotation[rotation], ImGuiSliderFlags_NoInput);
  const float radians = glm::radians(static_cast<float>(rotation) * 90.0f);
  if (radians != cursor->rotation) {
    cursor->rotation = radians;
    cursor->collider = cursorBase->collider->Clone();
    cursor->collider->RotateY(radians);
  }
  End();

  if (mode == Mode::groundPaint) {
    SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Once);
    Begin(u8"地面タイル選択");
    const ImVec2 tileListBoxSize(-1,
      68.0f * groundTiles.size() + GetStyle().FramePadding.y * 2);
    if (BeginListBox("GroundTileList", tileListBoxSize)) {
      const ImVec2 itemSize(
        64.0f + GetFontSize() * 32.0f + GetStyle().FramePadding.x * 2.0f,
        64.0f + GetStyle().FramePadding.y * 2.0f);
      for (int i = 0; i < groundTiles.size(); ++i) {
        std::string id = std::string("##") + groundTiles[i]->GetName();
        const bool isSelected = currentTileNo == i;
        const ImVec2 cursorPos = GetCursorPos();
        if (Selectable(id.c_str(), isSelected, 0, itemSize)) {
          currentTileNo = i;
          cursor->color = glm::vec4(0.2f, 0.5f, 1.0f, 0.5f);
        }
        if (isSelected) {
          SetItemDefaultFocus();
        }
        SetCursorPos(cursorPos);
        const ImTextureID texId =
          reinterpret_cast<ImTextureID>(groundTiles[i]->GetId());
        Image(texId, ImVec2(64, 64));
        SameLine();
        Text(groundTiles[i]->GetName().c_str());
      }
      EndListBox();
    }
    End();
  }

#if 0
  glm::ivec2 newMapSize = mapSize;
  SetNextWindowSize(ImVec2(200, 0), ImGuiCond_Once);
  Begin(u8"マップサイズ");
  Text("X:");
  SameLine();
  SetNextItemWidth(-FLT_MIN);
  if (InputInt("##MapSizeX", &newMapSize.x, 2)) {
    newMapSize.x |= 1;
    newMapSize.x = glm::clamp(newMapSize.x, 11, 101);
  }
  Text("Y:");
  SameLine();
  SetNextItemWidth(-FLT_MIN);
  if (InputInt("##MapSizeY", &newMapSize.y, 2)) {
    newMapSize.y |= 1;
    newMapSize.y = glm::clamp(newMapSize.y, 11, 101);
  }
  End();
  if (mapSize.x != newMapSize.x || mapSize.y != newMapSize.y) {
    std::shared_ptr<Actor> ground = engine.FindActor("Ground");
    if (ground) {
      ground->scale.x = static_cast<float>(newMapSize.x);
      ground->scale.z = static_cast<float>(newMapSize.y);
    }
    std::vector<std::shared_ptr<Actor>> newMap(newMapSize.x * newMapSize.y);
    for (int y = 0; y < std::min(newMapSize.y, mapSize.y); ++y) {
      for (int x = 0; x < std::min(newMapSize.x, mapSize.x); ++x) {
        newMap[x + y * newMapSize.x] = map[x + y * mapSize.x];
      }
    }
    map.swap(newMap);
    mapSize = newMapSize;
  }
#endif

#if 0
  static Actor actor("Dummy", Primitive(), nullptr, glm::vec3(0), glm::vec3(0), 0, glm::vec3(0));

  SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Once);
  Begin(u8"アクターの情報");
  Text(u8"名前:");
  SameLine();
  std::string actorName(1024, '\0');
  std::copy(actor.name.begin(), actor.name.end(), actorName.begin());
  if (InputText("", actorName.data(), actorName.size())) {
    actor.name = actorName;
  }

  Text(u8"プリミティブ:");
  SameLine();
  static const char noPrimitiveName[] = u8"(なし)";
  static const char* currentPrimitive = noPrimitiveName;
  if (BeginCombo("##primitive", currentPrimitive)) {
    const bool isSelected = currentPrimitive == noPrimitiveName;
    if (Selectable(noPrimitiveName, isSelected)) {
      currentPrimitive = noPrimitiveName;
    }
    if (isSelected) {
      SetItemDefaultFocus();
    }
    for (int i = 0; i < engine.GetPrimitiveBuffer().GetCount(); ++i) {
      const char* p = engine.GetPrimitive(i).GetName().c_str();
      const bool isSelected = currentPrimitive == p;
      if (Selectable(p, isSelected)) {
        currentPrimitive = p;
      }
      if (isSelected) {
        SetItemDefaultFocus();
      }
    }
    EndCombo();
  }

  Text(u8"テクスチャ:");
  SameLine();
  static const char noTextureName[] = u8"(なし)";
  static const char* currentTexture = noTextureName;
  static std::shared_ptr<Texture> currentTexturePtr;
  if (BeginCombo("##texture", currentTexture)) {
    const bool isSelected = currentTexture == noTextureName;
    if (Selectable(noTextureName, isSelected)) {
      currentTexture = noTextureName;
      currentTexturePtr.reset();
    }
    if (isSelected) {
      SetItemDefaultFocus();
    }
    for (int i = 0; i < engine.GetTextureCount(); ++i) {
      std::shared_ptr<Texture> tex = engine.GetTexture(i);
      const char* p = tex->GetName().c_str();
      const bool isSelected = currentTexture == p;
      if (Selectable(p, isSelected)) {
        currentTexture = p;
        currentTexturePtr = tex;
      }
      if (isSelected) {
        SetItemDefaultFocus();
      }
    }
    EndCombo();
  }
  if (currentTexturePtr) {
    const ImTextureID id = reinterpret_cast<ImTextureID>(currentTexturePtr->GetId());
    Image(id, ImVec2(64, 64));
  }

  InputFloat3(u8"位置", &actor.position.x);
  InputFloat3(u8"スケール", &actor.scale.x);
  SliderFloat(u8"回転", &actor.rotation, 0, 360);
  InputFloat3(u8"位置補正", &actor.adjustment.x);
  InputFloat(u8"耐久値", &actor.health);
  InputFloat(u8"寿命(秒)", &actor.lifespan);
  InputFloat(u8"重さ(kg)", &actor.mass);

  const char* colliderName[] = { u8"(なし)", u8"ボックス", u8"垂直円柱" };
  static const char* currentCollider = colliderName[0];
  Text(u8"コライダー:");
  SameLine();
  if (BeginCombo("##collider", currentCollider)) {
    for (int i = 0; i < std::size(colliderName); ++i) {
      const bool isSelected = currentCollider == colliderName[i];
      if (Selectable(colliderName[i], isSelected)) {
        currentCollider = colliderName[i];
      }
      if (isSelected) {
        SetItemDefaultFocus();
      }
    }
    EndCombo();
  }

  BeginChildFrame(GetID("collider"), ImVec2(-FLT_MIN, GetTextLineHeightWithSpacing() * 3));
  Box box(glm::vec3(0), glm::vec3(0));
  InputFloat3(u8"最小", &box.min.x);
  InputFloat3(u8"最大", &box.max.x);
  EndChildFrame();
  Checkbox(u8"スタティック", &actor.isStatic);
  Checkbox(u8"衝突", &actor.isBlock);
  InputFloat(u8"摩擦係数", &actor.friction);
  InputFloat(u8"反発係数", &actor.cor);
  End();
#endif
}

/**
* マップデータをファイルに保存する
*/
void MapEditor::Save(const char* filename)
{
  std::ofstream ofs(filename);
  if (!ofs) {
    std::cerr << "[エラー]" << __func__ << filename << "を開けません\n";
    return;
  }

  ofs << "mapSize: [" << mapSize.x << ", " << mapSize.y << "],\n";
  ofs << "map: [\n";
  for (std::shared_ptr<Actor>& e : map) {
    if (!e) {
      continue;
    }
    char tmp[256];
    snprintf(tmp, std::size(tmp),
      "  [ %s, %.03f, %.03f, %.03f, %.03f, %.03f, %.03f, %.03f ],\n",
      e->name.c_str(), e->position.x, e->position.y, e->position.z,
      e->scale.x, e->scale.y, e->scale.z, glm::degrees(e->rotation));
    ofs << tmp;
  }
  ofs << "],\n";

  ofs << "groundMap: [\n";
  for (int y = 0; y < mapSize.y; ++y) {
    ofs << "  ";
    for (int x = 0; x < mapSize.x; ++x) {
      ofs << groundMap[x + y * mapSize.x] << ", ";
    }
    ofs << '\n';
  }
  ofs << "]\n";
}

/**
* アクターリストからアクターを取得する
*/
std::shared_ptr<Actor> MapEditor::GetActor(const char* name, int* no) const
{
  int i = 1;
  for (auto& e : actors) {
    if (e->name == name) {
      if (no) {
        *no = i;
      }
      return e;
    }
    ++i;
  }
  return nullptr;
}

/**
* マップデータをファイルから読み込む
*/
bool MapEditor::Load(const char* filename)
{
  std::ifstream ifs(filename);
  if (!ifs) {
    std::cerr << "[エラー]" << __func__ << ":" << filename << "を開けません\n";
    return false;
  }

  // マップデータ読み込み用変数
  glm::ivec2 tmpMapSize;
  std::vector<std::shared_ptr<Actor>> tmpMap;
  std::vector<int> tmpGameMap;

  // マップサイズを読み込む
  std::string line;
  std::getline(ifs, line);
  if (sscanf(line.data(), "mapSize: [ %d, %d ],",
    &tmpMapSize.x, &tmpMapSize.y) != 2) {
    std::cerr << "[エラー]" << __func__ << ": マップサイズの読み込みに失敗\n";
    return false;
  }
  tmpMap.resize(tmpMapSize.x * tmpMapSize.y);
  tmpGameMap.resize(tmpMapSize.x * tmpMapSize.y, 0);

  // map行を読み飛ばす
  std::getline(ifs, line);
  if (line != "map: [") {
    std::cerr << "[エラー]" << __func__ << ": マップデータの読み込みに失敗\n";
    return false;
  }

  // アクターデータを読み込む
  while (!ifs.eof()) {
    std::getline(ifs, line);

    // データの終了チェック
    if (line[0] == ']') {
      break;
    }

    // 行を解析
    char name[256];
    glm::vec3 position(0);
    glm::vec3 scale(1);
    float rotation = 0;
    if (sscanf(line.data(), " [ %255[^,], %f, %f, %f, %f, %f, %f, %f ], ",
      name, &position.x, &position.y, &position.z,
      &scale.x, &scale.y, &scale.z, &rotation) < 4) {
      std::cerr << "[警告]" << __func__ << ": 配置データの読み込みに失敗\n" <<
        "  " << line << "\n";
    }
    rotation = glm::radians(rotation);
    name[255] = '\0';

    // アクターを取得
    int actorNo = 0;
    std::shared_ptr<Actor> actor = GetActor(name, &actorNo);
    if (!actor) {
      std::cerr << "[警告]" << __func__ << ": " <<
        name << "はアクターリストに存在しません\n";
      continue;
    }

    // ワールド座標をマップ座標に変換
    const int x = static_cast<int>(glm::round(position.x / 4)) + tmpMapSize.x / 2;
    const int y = static_cast<int>(glm::round(position.z / 4)) + tmpMapSize.y / 2;
    if (x < 0 || x >= tmpMapSize.x || y < 0 || y >= tmpMapSize.y) {
      std::cerr << "[警告]" << __func__ << ": " << name <<
        "の座標(" << position.x << ", " << position.z << ")はマップ範囲外です\n";
      continue;
    }

    // アクターをマップに配置
    std::shared_ptr<Actor> newActor = actor->Clone();
    newActor->position = position;
    newActor->scale = scale;
    newActor->rotation = rotation;
    if (rotation) {
      // 衝突判定を回転させる
      newActor->collider->RotateY(rotation);
    }
    tmpMap[x + y * tmpMapSize.x] = newActor;
    tmpGameMap[x + y * tmpMapSize.x] = actorNo;
  }

  // 地面マップを読み込む
  std::vector<uint32_t> tmpGroundMap;
  tmpGroundMap.reserve(tmpMapSize.x * tmpMapSize.y);
  std::getline(ifs, line);
  if (line == "groundMap: [") {
    while (!ifs.eof()) {
      std::getline(ifs, line);

      // データの終了チェック
      if (line[0] == ']') {
        break;
      }
      // 行を解析
      char* p = line.data();
      for (;;) {
        int tileNo = 0;
        int n = 0;
        if (sscanf(p, " %d,%n", &tileNo, &n) == 1) {
          tmpGroundMap.push_back(tileNo);
          p += n;
        } else {
          break;
        }
      }
    }
  }
  tmpGroundMap.resize(tmpMapSize.x * tmpMapSize.y, 0);

  // 読み込んだデータをメンバ変数に反映する
  mapSize = tmpMapSize;
  map.swap(tmpMap);
  groundMap.swap(tmpGroundMap);
  gameMap.swap(tmpGameMap);

  GameEngine& engine = GameEngine::Get();
  engine.ResizeGroundMap(mapSize.x, mapSize.y, groundMap.data());

  // ゲームエンジンのアクターを更新
  engine.ClearAllActors();
  InitGroundActor();
  for (std::shared_ptr<Actor>& e : map) {
    if (e) {
      engine.AddActor(e);
    }
  }

  if (systemType == SystemType::editor) {
    InitEditor();
  }

  std::cerr << "[情報]" << __func__ << ": " << filename << "をロード\n";
  return true;
}

