/**
* @file MapEditor.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "MapEditor.h"
#include "Actor/PlayerActor.h"
#include "Actor/T34TankActor.h"
#include "GameEngine.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <math.h>
#include <fstream>
#include <filesystem>
#include <iostream>

/**
* �X�N���[�����W���n�_�Ƃ��A��ʂ̉��֌������������v�Z����
*
* @param screenPos �X�N���[�����W
* @param matVP     �r���[�v���W�F�N�V�����s��
* @param start     �����̎n�_����������
* @param end       �����̏I�_����������
*/
void ScreenPosToLine(const ImVec2& screenPos, const glm::mat4& matVP, glm::vec3& start, glm::vec3& end)
{
  // �X�N���[�����W��NDC���W�ɕϊ�
  const glm::vec2 windowSize = GameEngine::Get().GetWindowSize();
  glm::vec4 ndcPos(screenPos.x / windowSize.x * 2.0f - 1.0f,
    1.0f - screenPos.y / windowSize.y * 2.0f, -1, 1);

  // �����̎n�_���W���v�Z
  const glm::mat4 matInvVP = glm::inverse(matVP);
  glm::vec4 worldPos0 = matInvVP * ndcPos;
  start = worldPos0 / worldPos0.w;

  // �����̏I�_���W���v�Z
  ndcPos.z = 1;
  glm::vec4 worldPos1 = matInvVP * ndcPos;
  end = worldPos1 / worldPos1.w;
}

/**
* �����ƕ��ʂ�����������W�����߂�
*
* @param start  �����̎n�_
* @param end    �����̏I�_
* @param q      ���ʏ�̔C�ӂ̓_
* @param normal ���ʂ̖@��
* @param p      ��_�̍��W����������
*
* @retval true  �������Ă���
* @retval false �������Ă��Ȃ�
*/
bool Intersect(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& q, const glm::vec3& normal, glm::vec3& p)
{
  const float distance = glm::dot(normal, q - p0);
  const glm::vec3 v = p1 - p0;

  // ���ꂪ�ق�0�̏ꍇ�A�����͕��ʂƕ��s�Ȃ̂Ō������Ȃ�.
  const float denom = glm::dot(normal, v);
  if (std::abs(denom) < 0.0001f) {
    return false;
  }

  // ��_�܂ł̋���t��0�����܂���1���傫���ꍇ�A��_�͐����̊O���ɂ���̂Ŏ��ۂɂ͌������Ȃ�.
  const float t = distance / denom;
  if (t < 0 || t > 1) {
    return false;
  }

  // ��_�͐�����ɂ���.
  p = p0 + v * t;
  return true;
}

/**
* �R���X�g���N�^
*/
MapEditor::MapEditor()
{
  GameEngine& engine = GameEngine::Get();

  // �A�N�^�[�z�u�}�b�v�̃T�C�Y���}�b�v�T�C�Y�ɍ��킹��
  map.resize(mapSize.x * mapSize.y);

  engine.LoadPrimitive("Res/Plane.obj");

  const char* texList[] = {
    "Res/Green.tga",
    "Res/Road.tga",
    "Res/RoadTiles.tga",
  };

  // �n�ʗp�A�N�^�[���쐬
  engine.LoadPrimitive("Res/Ground.obj");
  std::shared_ptr<Actor> groundActor(new Actor("Ground",
    engine.GetPrimitive("Res/Ground.obj"),
    engine.LoadTexture("GroundTiles", texList, std::size(texList)),
    //engine.LoadTexture("Res/Image1.tga"),
    glm::vec3(0), glm::vec3(mapSize.x, 1, mapSize.y), 0, glm::vec3(0)));
  groundActor->shader = Shader::Ground;
  engine.AddActor(groundActor);

  // �}�b�v�f�[�^�e�N�X�`������p�̕ϐ���������
  groundMap.resize(mapSize.x * mapSize.y, 0);
  groundTiles.reserve(std::size(texList));
  for (const char* filename : texList) {
    groundTiles.push_back(engine.LoadTexture(filename));
  }

  // �z�u�p�A�N�^�[���쐬
  enum class ActorType {
    player,
    t34tank,
    other,
  };
  struct ObjectData {
    ActorType type;
    const char* name;
    const char* primitiveFilename;
    const char* textureFilename;
    Box collider;
    glm::vec3 scale = glm::vec3(1);
    float rotation = 0;
    glm::vec3 adjustment = glm::vec3(0);
  };
  const ObjectData objectList[] = {
    { ActorType::other, "Cube", "Res/Cube.obj", "Res/Triangle.tga",
      Box(glm::vec3(0, 0, 0), glm::vec3(2, 2, 2)) },
    { ActorType::other, "Tree", "Res/Tree.obj", "Res/Tree.tga",
      Box(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2, 1.5f)) },
    { ActorType::other, "Warehouse", "Res/Warehouse.obj", "Res/Building.tga",
      Box(glm::vec3(-2, 0, -2), glm::vec3(2, 3, 2)) },
    { ActorType::other, "BrickHouse", "Res/house/HouseRender.obj", "Res/house/House38UVTexture.tga",
      Box(glm::vec3(-3, 0, -2), glm::vec3(3, 3, 2)),
      glm::vec3(2.0f), 0, glm::vec3(-2.6f, 2.0f, 0.8f) },
    { ActorType::other, "BrokenHouse", "Res/house/broken-house.obj", "Res/house/broken-house.tga",
      Box(glm::vec3(-2, 0, -2), glm::vec3(2, 2, 2)),
      glm::vec3(0.75f) },
    { ActorType::player, "Tiger-I", "Res/tank/Tiger_I.obj", "Res/tank/PzVl_Tiger_I.tga",
      Box(glm::vec3(-1, 0, -1), glm::vec3(1, 2, 1)) },
    { ActorType::t34tank, "T-34", "Res/tank/T34.obj", "Res/tank/T-34.tga",
      Box(glm::vec3(-1, 0, -1), glm::vec3(1, 2, 1)) },
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
      break;

    case ActorType::other:
      actor.reset(new Actor(
        e.name,
        engine.GetPrimitive(e.primitiveFilename),
        engine.LoadTexture(e.textureFilename),
        glm::vec3(0), e.scale, e.rotation, e.adjustment));
      actor->collider = std::shared_ptr<Box>(new Box(e.collider.min, e.collider.max));
      break;
    }
    actors.push_back(actor);
  }

  // �ʒu�\���A�N�^�[���쐬
  cursor.reset(new Actor(*actors[0]));
  cursor->color = glm::vec4(0.2f, 0.5f, 1.0f, 0.5f);
  engine.AddActor(cursor);

  // �J������ݒ�
  Camera& camera = engine.GetCamera();
  camera.target = glm::vec3(0, 0, 0);
  camera.position = camera.target + cameraOffset;

  // �����̃A�N�^�[�����ׂč폜
  engine.GetActors().clear();
}

/**
* �}�b�v�G�f�B�^�̏�Ԃ��X�V����
*/
void MapEditor::Update(float deltaTime)
{
  GameEngine& engine = GameEngine::Get();

  // ImGui���}�E�X������������Ă���ꍇ�A�}�b�v�G�f�B�^���ł̓}�E�X������s��Ȃ�
  if (ImGui::GetIO().WantCaptureMouse) {
    return;
  }

  // ESC�L�[�������ꂽ��u�I�����[�h�v�ɖ߂�
  if (engine.GetKey(GLFW_KEY_ESCAPE)) {
    mode = Mode::select;
  }

  // �E�N���b�N
  if (engine.GetMouseButton(GLFW_MOUSE_BUTTON_LEFT)) {
    // �}�b�v�͈͊O�̏ꍇ�͉������Ȃ�
    const int x = static_cast<int>(glm::round(cursor->position.x / 4)) + mapSize.x / 2;
    const int y = static_cast<int>(glm::round(cursor->position.z / 4)) + mapSize.y / 2;
    if (x < 0 || x >= mapSize.x || y < 0 || y >= mapSize.y) {
      return;
    }

    // �}�b�v����J�[�\���ʒu�ɑΉ�����A�N�^�[���擾
    std::shared_ptr<Actor>& target = map[x + y * mapSize.x];

    switch (mode) {
    case Mode::set:
      // �I���A�N�^�[�Ǝ�ނ������ꍇ�͔z�u���Ȃ�
      if (target && target->name == cursor->name) {
        break;
      }
      // �����̃A�N�^�[������΍폜����
      if (target) {
        target->isDead = true;
      }
      // �I�𒆂̃A�N�^�[��z�u����
      target.reset(new Actor(*cursor));
      target->color = glm::vec4(1);
      engine.AddActor(target);
      break;

    case Mode::remove:
      // �J�[�\���ʒu�̃A�N�^�[���폜
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
* �J������Ԃ��X�V����
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

  // �J�[�\���ʒu��ݒ�
  const glm::mat4 matVP = camera.GetProjectionMatrix() * camera.GetViewMatrix();
  glm::vec3 start, end, p;
  ScreenPosToLine(ImGui::GetMousePos(), matVP, start, end);
  if (Intersect(start, end, glm::vec3(0), glm::vec3(0, 1, 0), p)) {
    cursor->position = glm::round(p / 4.0f) * 4.0f;
  }
}

/**
* UI���X�V����
*/
void MapEditor::UpdateUI()
{
  using namespace ImGui;
  GameEngine& engine = GameEngine::Get();

  //ShowDemoWindow();

  Begin(u8"�c�[��");
  const char* toolName[] = { u8"�I��", u8"�z�u", u8"�폜", u8"�n�ʃy�C���g" };
  const Mode modeList[] = { Mode::select, Mode::set, Mode::remove, Mode::groundPaint };
  for (int i = 0; i < std::size(toolName); ++i) {
    SameLine();
    if (Button(toolName[i])) {
      mode = modeList[i];
    }
  }
  SameLine();
  if (Button(u8"�Z�[�u")) {
    Save(u8"mapdata.txt");
  }
  SameLine();
  if (Button(u8"���[�h")) {
    Load("mapdata.txt");
  }
  Text(toolName[static_cast<int>(mode)]);
  End();

  SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Once);
  Begin(u8"�A�N�^�[�I��");
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

  if (mode == Mode::groundPaint) {
    SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Once);
    Begin(u8"�n�ʃ^�C���I��");
    const ImVec2 tileListBoxSize(-1,
      68.0f * groundTiles.size() + GetStyle().FramePadding.y * 2);
    if (BeginListBox("GroundTileList", tileListBoxSize)) {
      const float itemWidth = 64.0f + GetFontSize() * 32.0f + GetStyle().FramePadding.x * 2.0f;
      for (int i = 0; i < groundTiles.size(); ++i) {
        std::string id = std::string("##") + groundTiles[i]->GetName();
        const bool isSelected = currentTileNo == i;
        const ImVec2 cursorPos = GetCursorPos();
        if (Selectable(id.c_str(), isSelected, 0, ImVec2(itemWidth, 68))) {
          currentTileNo = i;
          cursor->color = glm::vec4(0.2f, 0.5f, 1.0f, 0.5f);
        }
        if (isSelected) {
          SetItemDefaultFocus();
        }
        SetCursorPos(cursorPos);
        const ImTextureID texId = reinterpret_cast<ImTextureID>(groundTiles[i]->GetId());
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
  Begin(u8"�}�b�v�T�C�Y");
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
  Begin(u8"�A�N�^�[�̏��");
  Text(u8"���O:");
  SameLine();
  std::string actorName(1024, '\0');
  std::copy(actor.name.begin(), actor.name.end(), actorName.begin());
  if (InputText("", actorName.data(), actorName.size())) {
    actor.name = actorName;
  }

  Text(u8"�v���~�e�B�u:");
  SameLine();
  static const char noPrimitiveName[] = u8"(�Ȃ�)";
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

  Text(u8"�e�N�X�`��:");
  SameLine();
  static const char noTextureName[] = u8"(�Ȃ�)";
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

  InputFloat3(u8"�ʒu", &actor.position.x);
  InputFloat3(u8"�X�P�[��", &actor.scale.x);
  SliderFloat(u8"��]", &actor.rotation, 0, 360);
  InputFloat3(u8"�ʒu�␳", &actor.adjustment.x);
  InputFloat(u8"�ϋv�l", &actor.health);
  InputFloat(u8"����(�b)", &actor.lifespan);
  InputFloat(u8"�d��(kg)", &actor.mass);

  const char* colliderName[] = { u8"(�Ȃ�)", u8"�{�b�N�X", u8"�����~��" };
  static const char* currentCollider = colliderName[0];
  Text(u8"�R���C�_�[:");
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
  InputFloat3(u8"�ŏ�", &box.min.x);
  InputFloat3(u8"�ő�", &box.max.x);
  EndChildFrame();
  Checkbox(u8"�X�^�e�B�b�N", &actor.isStatic);
  Checkbox(u8"�Փ�", &actor.isBlock);
  InputFloat(u8"���C�W��", &actor.friction);
  InputFloat(u8"�����W��", &actor.cor);
  End();
#endif
}

/**
* �}�b�v�f�[�^���t�@�C���ɕۑ�����
*/
void MapEditor::Save(const char* filename)
{
  std::ofstream ofs(filename);
  if (!ofs) {
    std::cerr << "[�G���[]" << __func__ << filename << "���J���܂���\n";
    return;
  }

  ofs << "mapSize: [" << mapSize.x << ", " << mapSize.y << "],\n";
  ofs << "map: [\n";
  for (std::shared_ptr<Actor>& e : map) {
    if (!e) {
      continue;
    }
    char tmp[256];
    snprintf(tmp, std::size(tmp), "  [ %s, %.03f, %.03f, %.03f ],\n",
      e->name.c_str(), e->position.x, e->position.y, e->position.z);
    ofs << tmp;
  }
  ofs << "],\n";

  ofs << "groundMap: [\n";
  for (int y = 0; y < mapSize.y; ++y) {
    ofs << "  ";
    for (int x = 0; x < mapSize.x; ++x) {
      ofs << groundMap[x + y * mapSize.y] << ", ";
    }
    ofs << '\n';
  }
  ofs << "]\n";
}

/**
* �A�N�^�[���X�g����A�N�^�[���擾����
*/
std::shared_ptr<Actor> MapEditor::GetActor(const char* name) const
{
  for (auto& e : actors) {
    if (e->name == name) {
      return e;
    }
  }
  return nullptr;
}

/**
* �}�b�v�f�[�^���t�@�C������ǂݍ���
*/
void MapEditor::Load(const char* filename)
{
  std::ifstream ifs(filename);
  if (!ifs) {
    std::cerr << "[�G���[]" << __func__ << ":" << filename << "���J���܂���\n";
    return;
  }

  // �}�b�v�f�[�^�ǂݍ��ݗp�ϐ�
  glm::ivec2 tmpMapSize;
  std::vector<std::shared_ptr<Actor>> tmpMap;

  // �}�b�v�T�C�Y��ǂݍ���
  std::string line;
  std::getline(ifs, line);
  if (sscanf(line.data(), "mapSize: [ %d, %d ],",
    &tmpMapSize.x, &tmpMapSize.y) != 2) {
    std::cerr << "[�G���[]" << __func__ << ": �}�b�v�T�C�Y�̓ǂݍ��݂Ɏ��s\n";
    return;
  }
  tmpMap.resize(tmpMapSize.x * tmpMapSize.y);

  // map�s��ǂݔ�΂�
  std::getline(ifs, line);
  if (line != "map: [") {
    std::cerr << "[�G���[]" << __func__ << ": �}�b�v�f�[�^�̓ǂݍ��݂Ɏ��s\n";
    return;
  }

  // �A�N�^�[�f�[�^��ǂݍ���
  while (!ifs.eof()) {
    std::getline(ifs, line);

    // �f�[�^�̏I���`�F�b�N
    if (line[0] == ']') {
      break;
    }

    // �s�����
    char name[256];
    glm::vec3 position(0);
    if (sscanf(line.data(), " [ %255[^,], %f, %f, %f ], ",
      name, &position.x, &position.y, &position.z) != 4) {
      std::cerr << "[�x��]" << __func__ << ": �z�u�f�[�^�̓ǂݍ��݂Ɏ��s\n" <<
        "  " << line << "\n";
    }
    name[255] = '\0';

    // �A�N�^�[���擾
    std::shared_ptr<Actor> actor = GetActor(name);
    if (!actor) {
      std::cerr << "[�x��]" << __func__ << ": " <<
        name << "�̓A�N�^�[���X�g�ɑ��݂��܂���\n";
      continue;
    }

    // ���[���h���W���}�b�v���W�ɕϊ�
    const int x = static_cast<int>(glm::round(position.x / 4)) + tmpMapSize.x / 2;
    const int y = static_cast<int>(glm::round(position.z / 4)) + tmpMapSize.y / 2;
    if (x < 0 || x >= tmpMapSize.x || y < 0 || y >= tmpMapSize.y) {
      std::cerr << "[�x��]" << __func__ << ": " << name <<
        "�̍��W(" << position.x << ", " << position.z << ")�̓}�b�v�͈͊O�ł�\n";
      continue;
    }

    // �A�N�^�[���}�b�v�ɔz�u
    std::shared_ptr<Actor> newActor(new Actor(*actor));
    newActor->position = position;
    tmpMap[x + y * mapSize.x] = newActor;
  }

  // �n�ʃ}�b�v��ǂݍ���
  std::vector<uint32_t> tmpGroundMap;
  tmpGroundMap.reserve(tmpMapSize.x * tmpMapSize.y);
  std::getline(ifs, line);
  if (line == "groundMap: [") {
    while (!ifs.eof()) {
      std::getline(ifs, line);

      // �f�[�^�̏I���`�F�b�N
      if (line[0] == ']') {
        break;
      }
      // �s�����
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

  // �ǂݍ��񂾃f�[�^�������o�ϐ��ɔ��f����
  mapSize = tmpMapSize;
  map.swap(tmpMap);
  groundMap.swap(tmpGroundMap);

  GameEngine& engine = GameEngine::Get();
  engine.UpdateGroundMap(0, 0, mapSize.x, mapSize.y, groundMap.data());

  // �Q�[���G���W���̃A�N�^�[���X�V
  engine.GetActors().resize(2);
  for (std::shared_ptr<Actor>& e : map) {
    if (e) {
      engine.AddActor(e);
    }
  }

  std::cerr << "[���]" << __func__ << ": " << filename << "�����[�h\n";
}

