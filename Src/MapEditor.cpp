/**
* @file MapEditor.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "MapEditor.h"
#include "Actor.h"
#include "GameEngine.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <math.h>
#include <fstream>
#include <filesystem>
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
MapEditor::MapEditor()
{
  GameEngine& engine = GameEngine::Get();

  map.resize(mapSize.x * mapSize.y);

  // 地面用アクターを作成
  engine.LoadPrimitive("Res/Ground.obj");
  std::shared_ptr<Actor> groundActor(new Actor("Ground",
    engine.GetPrimitive("Res/Ground.obj"),
    engine.LoadTexture("Res/Road.tga"),
    glm::vec3(0), glm::vec3(mapSize.x, 1, mapSize.y), 0, glm::vec3(0)));
  engine.AddActor(groundActor);

  // 配置用アクターを作成
  struct ObjectData {
    const char* name;
    const char* primitiveFilename;
    const char* textureFilename;
    Box collider;
    glm::vec3 scale = glm::vec3(1);
    float rotation = 0;
    glm::vec3 adjustment = glm::vec3(0);
  };
  const ObjectData objectList[] = {
    { "Cube", "Res/Cube.obj", "Res/Triangle.tga",
      Box(glm::vec3(0, 0, 0), glm::vec3(2, 2, 2)) },
    { "Tree", "Res/Tree.obj", "Res/Tree.tga",
      Box(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2, 1.5f)) },
    { "Warehouse", "Res/Warehouse.obj", "Res/Building.tga",
      Box(glm::vec3(-2, 0, -2), glm::vec3(2, 3, 2)) },
    { "BrickHouse", "Res/house/HouseRender.obj", "Res/house/House38UVTexture.tga",
      Box(glm::vec3(-3, 0, -2), glm::vec3(3, 3, 2)),
      glm::vec3(2.0f), 0, glm::vec3(-2.6f, 2.0f, 0.8f) },
    { "BrokenHouse", "Res/house/broken-house.obj", "Res/house/broken-house.tga",
      Box(glm::vec3(-2, 0, -2), glm::vec3(2, 2, 2)),
      glm::vec3(0.75f) },
    { "Tiger-I", "Res/tank/Tiger_I.obj", "Res/tank/PzVl_Tiger_I.tga",
      Box(glm::vec3(-1, 0, -1), glm::vec3(1, 2, 1)) },
    { "T-34", "Res/tank/T34.obj", "Res/tank/T-34.tga",
      Box(glm::vec3(-1, 0, -1), glm::vec3(1, 2, 1)) },
  };
  for (const auto& e : objectList) {
    engine.LoadPrimitive(e.primitiveFilename);
    std::shared_ptr<Actor> actor(new Actor(
      e.name,
      engine.GetPrimitive(e.primitiveFilename),
      engine.LoadTexture(e.textureFilename),
      glm::vec3(0), e.scale, e.rotation, e.adjustment));
    actor->collider = std::shared_ptr<Box>(new Box(e.collider.min, e.collider.max));
    actors.push_back(actor);
  }

  // 位置表示アクターを作成
  cursor.reset(new Actor(*actors[0]));
  cursor->color = glm::vec4(0.2f, 0.5f, 1.0f, 0.5f);
  engine.AddActor(cursor);

  // カメラを設定
  Camera& camera = engine.GetCamera();
  camera.target = glm::vec3(0, 0, 0);
  camera.position = camera.target + cameraOffset;

  // 既存のアクターをすべて削除
  engine.GetActors().clear();
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

  Begin(u8"ツール");
  const char* toolName[] = { u8"選択", u8"配置", u8"削除" };
  const Mode modeList[] = { Mode::select, Mode::set, Mode::remove };
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
        *cursor = *actors[i];
        cursor->color = glm::vec4(0.2f, 0.5f, 1.0f, 0.5f);
      }
      if (isSelected) {
        SetItemDefaultFocus();
      }
    }
    EndListBox();
  }
  End();

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
    snprintf(tmp, std::size(tmp), "  [ %s, %.03f, %.03f, %.03f ],",
      e->name.c_str(), e->position.x, e->position.y, e->position.z);
    ofs << tmp;
  }
  ofs << "\n]\n";
}

/**
* マップデータをファイルから読み込む
*/
void MapEditor::Load(const char* filename)
{
  std::ifstream ifs(filename);
  if (!ifs) {
    std::cerr << "[エラー]" << __func__ << ":" << filename << "を開けません\n";
    return;
  }
  std::string line;
  line.reserve(4096);
  std::getline(ifs, line);
  glm::ivec2 tmpMapSize;
  if (sscanf(line.data(), "mapSize: [ %d, %d ],", &tmpMapSize.x, &tmpMapSize.y) != 2) {
    std::cerr << "[エラー]" << __func__ << ": マップサイズの読み込みに失敗\n";
    return;
  }

  std::getline(ifs, line);
  if (line != "map: [") {
    return;
  }

  std::vector<std::shared_ptr<Actor>> tmpMap;
  tmpMap.resize(tmpMapSize.x * tmpMapSize.y);
  while (!ifs.eof()) {
    std::string line;
    std::getline(ifs, line);
    char name[256];
    glm::vec3 position(0);
    if (sscanf(line.data(), " [ %255[^,], %f, %f, %f ], ",
      name, &position.x, &position.y, &position.z) != 4) {
      continue;
    }
    name[255] = '\0';

    const int x = static_cast<int>(glm::round(position.x / 4)) + tmpMapSize.x / 2;
    const int y = static_cast<int>(glm::round(position.z / 4)) + tmpMapSize.y / 2;
    if (x < 0 || x >= tmpMapSize.x || y < 0 || y >= tmpMapSize.y) {
      continue;
    }
    for (auto& e : actors) {
      if (e->name == name) {
        std::shared_ptr<Actor> actor(new Actor(*e));
        actor->position = position;
        tmpMap[x + y * mapSize.x] = actor;
        break;
      }
    }
  }

  mapSize = tmpMapSize;
  map.swap(tmpMap);

  GameEngine& engine = GameEngine::Get();
  engine.GetActors().resize(2);
  for (std::shared_ptr<Actor>& e : map) {
    if (e) {
      engine.AddActor(e);
    }
  }
}


