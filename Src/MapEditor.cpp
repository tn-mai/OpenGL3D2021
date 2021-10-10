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

  map.resize(mapSize.x * mapSize.y);

  // �n�ʗp�A�N�^�[���쐬
  engine.LoadPrimitive("Res/Ground.obj");
  std::shared_ptr<Actor> groundActor(new Actor("Ground",
    engine.GetPrimitive("Res/Ground.obj"),
    engine.LoadTexture("Res/Road.tga"),
    glm::vec3(0), glm::vec3(mapSize.x, 1, mapSize.y), 0, glm::vec3(0)));
  engine.AddActor(groundActor);

  // �z�u�p�A�N�^�[���쐬
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

  Begin(u8"�c�[��");
  const char* toolName[] = { u8"�I��", u8"�z�u", u8"�폜" };
  const Mode modeList[] = { Mode::select, Mode::set, Mode::remove };
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
    snprintf(tmp, std::size(tmp), "  [ %s, %.03f, %.03f, %.03f ],",
      e->name.c_str(), e->position.x, e->position.y, e->position.z);
    ofs << tmp;
  }
  ofs << "\n]\n";
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
  std::string line;
  line.reserve(4096);
  std::getline(ifs, line);
  glm::ivec2 tmpMapSize;
  if (sscanf(line.data(), "mapSize: [ %d, %d ],", &tmpMapSize.x, &tmpMapSize.y) != 2) {
    std::cerr << "[�G���[]" << __func__ << ": �}�b�v�T�C�Y�̓ǂݍ��݂Ɏ��s\n";
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


