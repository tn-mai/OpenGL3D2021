/**
* @file MapEditor.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "MapEditor.h"
#include "Actor/PlayerActor.h"
#include "Actor/T34TankActor.h"
#include "Actor/RepairItem.h"
#include "Actor/Boss01.h"
#include "Actor/FlagIfDied.h"
#include "Actor/MoveIfFlagged.h"
#include "GameEngine.h"
#include "GameManager.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <fstream>
#include <iostream>
#include <filesystem>

/**
* �R���X�g���N�^
*/
MapEditor::MapEditor(SystemType type) : systemType(type)
{
  GameEngine& engine = GameEngine::Get();

  // �����̃A�N�^�[�����ׂč폜
  engine.ClearAllActors();

  LoadCommonPrimitive();
  InitActorList();
  if (systemType == SystemType::editor) {
    InitGroundActor();
    InitEditor();
  }
}

/**
* �ėp�v���~�e�B�u��ǂݍ���
*/
void MapEditor::LoadCommonPrimitive()
{
  GameEngine& engine = GameEngine::Get();

  engine.LoadPrimitive("Res/Plane.obj");
  engine.LoadPrimitive("Res/Bullet.obj");
  engine.LoadPrimitive("Res/Explosion.obj");
}

/**
* �n�ʗp�A�N�^�[���쐬
*/
void MapEditor::InitGroundActor()
{
  GameEngine& engine = GameEngine::Get();

  // �A�N�^�[�z�u�}�b�v�̃T�C�Y���}�b�v�T�C�Y�ɍ��킹��
  map.resize(mapSize.x * mapSize.y);

  const char* texList[] = {
    "Res/Green.tga",
    "Res/Road.tga",
    "Res/RoadTiles.tga",
    "Res/soil.tga",
    "Res/soil_LT.tga",
    "Res/soil_LB.tga",
    "Res/soil_RT.tga",
    "Res/soil_RB.tga",
  };

  // �n�ʗp�A�N�^�[���쐬
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

  // �}�b�v�f�[�^�e�N�X�`������p�̕ϐ���������
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
* �z�u�p�A�N�^�[���쐬
*/
void MapEditor::InitActorList()
{
  GameEngine& engine = GameEngine::Get();

  actors.clear();

  // �A�N�^�[�̌^
  enum class ActorType {
    actor,
    player,
    t34tank,
    boss01,
    flagIfDied,
    moveIfFlagged,
  };

  // �z�u�p�A�N�^�[���쐬
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
      ActorType::actor, "Cube",
      "Res/Cube.obj", "Res/Triangle.tga",
      Box::Create(glm::vec3(0, 0, 0), glm::vec3(2, 2, 2)) },
    {
      ActorType::actor, "Tree",
      "Res/Tree.obj", "Res/Tree.tga",
      Box::Create(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2, 1.5f)) },
    {
      ActorType::actor, "Tree2",
      "Res/tree/fir.obj", "Res/tree/branch.tga",
      Box::Create(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2, 1.5f)) },
    {
      ActorType::actor, "Tree3",
      "Res/tree/LowPolyTree.obj", "Res/tree/LowPolyLeaves.tga",
      Box::Create(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2, 1.5f)) },
    {
      ActorType::actor, "MobleTree1",
      "Res/mobile_trees/baum hd med.obj", "Res/mobile_trees/ast4.tga",
      Box::Create(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2, 1.5f)), glm::vec3(0.5) },
    {
      ActorType::actor, "MobleTree2",
      "Res/mobile_trees/baum hd pine.obj", "Res/mobile_trees/ast5.tga",
      Box::Create(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2, 1.5f)), glm::vec3(0.5) },
    {
      ActorType::actor, "MobleTree3",
      "Res/mobile_trees/baum hd.obj", "Res/mobile_trees/ast3.tga",
      Box::Create(glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2, 1.5f)), glm::vec3(0.5) },
    {
      ActorType::actor, "House-1-5(2)",
      "Res/house/test/House-1-5.obj", "Res/house/test/Houses Colorscheme 2.tga",
      Box::Create(glm::vec3(-3, 0, -5), glm::vec3(3, 8, 5)) },
    {
      ActorType::actor, "House-1-5(3)",
      "Res/house/test/House-1-5.obj", "Res/house/test/Houses Colorscheme 3.tga",
      Box::Create(glm::vec3(-3, 0, -5), glm::vec3(3, 8, 5)) },
    {
      ActorType::actor, "House-1-5(4)",
      "Res/house/test/House-1-5.obj", "Res/house/test/Houses Colorscheme 4.tga",
      Box::Create(glm::vec3(-3, 0, -5), glm::vec3(3, 8, 5)) },
    {
      ActorType::actor, "House-1-5(5)",
      "Res/house/test/House-1-5.obj", "Res/house/test/Houses Colorscheme 5.tga",
      Box::Create(glm::vec3(-3, 0, -5), glm::vec3(3, 8, 5)) },
    {
      ActorType::actor, "Lamppost",
      "Res/house/test/Lamppost.obj", "Res/house/test/Houses Colorscheme.tga",
      Box::Create(glm::vec3(-0.5f, 0, -0.5f), glm::vec3(0.5f, 2, 0.5f)) },
    {
      ActorType::actor, "Bench",
      "Res/house/test/Bench.obj", "Res/house/test/Houses Colorscheme.tga",
      Box::Create(glm::vec3(-0.5f, 0, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f)) },
    {
      ActorType::actor, "Chair",
      "Res/house/test/Chair.obj", "Res/house/test/Houses Colorscheme.tga",
      Box::Create(glm::vec3(-0.5f, 0, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f)) },
    {
      ActorType::actor, "Warehouse",
      "Res/Warehouse.obj", "Res/Building.tga",
      Box::Create(glm::vec3(-2, 0, -2), glm::vec3(2, 3, 2)), glm::vec3(1.0f) },
    {
      ActorType::actor, "Fortress0",
      "Res/sci-fi-rts/Structure_v1.obj", "Res/sci-fi-rts/Structure_v1.tga",
      Box::Create(glm::vec3(-2, 0, -2), glm::vec3(2, 3, 2)), glm::vec3(1.0f) },
    {
      ActorType::moveIfFlagged, "Gate",
      "Res/sci-fi-rts/Structure_v2.obj", "Res/sci-fi-rts/Structure_v2.tga",
      Box::Create(glm::vec3(-1, 0, -1.75f), glm::vec3(2, 3, 1.75f)), glm::vec3(1.0f) },
    {
      ActorType::flagIfDied, "TargetObject",
      "Res/sci-fi-rts/Structure_v3.obj", "Res/sci-fi-rts/Structure_v3.tga",
      Box::Create(glm::vec3(-5, 0, -3), glm::vec3(5, 6, 3)), glm::vec3(1.0f) },
    {
      ActorType::actor, "RocketLauncher",
      "Res/sci-fi-rts/SciFi-Tower_Rocket Launcher.obj", "Res/sci-fi-rts/SciFi_Props-Pack03-diffuse.tga",
      Box::Create(glm::vec3(-2, 0, -2), glm::vec3(2, 3, 2)), glm::vec3(0.5f) },

    { ActorType::actor, "BrickHouse",
      "Res/house/HouseRender.obj", "Res/house/House38UVTexture.tga",
      Box::Create(glm::vec3(-3, 0, -2), glm::vec3(3, 3, 2)),
      glm::vec3(2.0f), 0, glm::vec3(-2.6f, 2.0f, 0.8f) },

    { ActorType::actor, "BrokenHouse",
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
      actor->tag = ActorTag::player;
      break;

    case ActorType::t34tank:
      actor.reset(new T34TankActor(
        e.name,
        engine.GetPrimitive(e.primitiveFilename),
        engine.LoadTexture(e.textureFilename),
        glm::vec3(0), e.scale, e.rotation, e.adjustment, nullptr));
      actor->collider = e.collider;
      actor->tag = ActorTag::enemy;
      actor->mass = 36'000;
      break;

    case ActorType::boss01:
      actor.reset(new Boss01(
        glm::vec3(0), e.scale, e.rotation, nullptr));
      actor->collider = e.collider;
      actor->tag = ActorTag::boss;
      break;

    case ActorType::flagIfDied:
      actor.reset(new FlagIfDied(
        e.name,
        engine.LoadMesh(e.primitiveFilename),
        glm::vec3(0), e.scale, e.rotation, e.adjustment));
      static_cast<MeshRenderer&>(*actor->renderer).SetMaterial(0,
        Mesh::Material{ "", glm::vec4(1),  engine.LoadTexture(e.textureFilename) });
      actor->collider = e.collider;
      actor->isStatic = true;
      break;

    case ActorType::moveIfFlagged:
      actor.reset(new MoveIfFlagged(
        e.name,
        engine.LoadMesh(e.primitiveFilename),
        glm::vec3(0), e.scale, e.rotation, e.adjustment));
      static_cast<MeshRenderer&>(*actor->renderer).SetMaterial(0,
        Mesh::Material{ "", glm::vec4(1),  engine.LoadTexture(e.textureFilename) });
      actor->collider = e.collider;
      actor->isStatic = true;
      break;

    case ActorType::actor:
      actor.reset(new Actor(
        e.name,
        engine.LoadMesh(e.primitiveFilename),
        glm::vec3(0), e.scale, e.rotation, e.adjustment));
      static_cast<MeshRenderer&>(*actor->renderer).SetMaterial(0,
        Mesh::Material{ "", glm::vec4(1),  engine.LoadTexture(e.textureFilename) });
      actor->collider = e.collider;
      actor->isStatic = true;
      break;
    }
    actors.push_back(actor);
  }
  actors.push_back(std::make_shared<RepairItem>(glm::vec3(0)));
}

/**
* �G�f�B�^�𓮂������߂ɕK�v�ȕϐ�������������
*/
void MapEditor::InitEditor()
{
  GameEngine& engine = GameEngine::Get();

  // �ʒu�\���A�N�^�[���쐬
  cursor.reset(new Actor(*actors[0]));
  cursor->color = glm::vec4(0.2f, 0.5f, 1.0f, 0.5f);
  engine.AddActor(cursor);

  cursorOriginal = actors[0];

  // �f�t�H���g�̃Q�[���t���O����20�Ƃ���
  const size_t defaultGameFlagCount = 20;
  GameManager::Get().SetGameFlagCount(defaultGameFlagCount);

  // �J������ݒ�
  Camera& camera = engine.GetCamera();
  camera.target = glm::vec3(0, 0, 0);
  camera.position = camera.target + cameraOffset;
}

/**
* �}�b�v�T�C�Y��ύX����
*/
void MapEditor::Resize(const  glm::ivec2& newMapSize)
{
  if (mapSize.x == newMapSize.x && mapSize.y == newMapSize.y) {
    return;
  }

  GameEngine& engine = GameEngine::Get();

  // �n�ʃ��f���̑傫����ύX
  std::shared_ptr<Actor> groundActor = engine.FindActor("Ground");
  if (groundActor) {
    groundActor->scale = glm::vec3(newMapSize.x, 1.0f, newMapSize.y);
    const glm::vec2 colliderSize = glm::vec2(newMapSize) * gridSize * 0.5f;
    groundActor->collider = Box::Create(
      glm::vec3(-colliderSize.x, -10.0f, -colliderSize.y),
      glm::vec3(colliderSize.x, 0.0f, colliderSize.y));
  }

  // �}�b�v�f�[�^�e�N�X�`���̑傫����ύX
  std::vector<uint32_t> newGroundMap(newMapSize.x * newMapSize.y, 0);
  for (int y = 0; y < std::min(newMapSize.y, mapSize.y); ++y) {
    for (int x = 0; x < std::min(newMapSize.x, mapSize.x); ++x) {
      newGroundMap[x + y * newMapSize.x] = groundMap[x + y * mapSize.x];
    }
  }
  groundMap.swap(newGroundMap);
  engine.ResizeGroundMap(newMapSize.x, newMapSize.y, groundMap.data());

  // �}�b�v�f�[�^�̑傫����ύX
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
* �t�@�C���I�����X�g�{�b�N�X��\��
*/
bool MapEditor::ShowFileListBox(std::string& filename)
{
  using namespace ImGui;

  // �g���q��".txt"�̃t�@�C�������X�g�A�b�v
  std::vector<std::string> files;
  files.reserve(100);
  for (const std::filesystem::directory_entry& e : std::filesystem::directory_iterator(".")) {
    if (e.path().extension() == ".txt" && e.is_regular_file()) {
      files.push_back(e.path().filename().string());
    }
  }

  bool selected = false;

  const ImVec2 listBoxSize(GetWindowContentRegionWidth(),
    GetTextLineHeightWithSpacing() * files.size() + GetStyle().FramePadding.y * 2);
  if (BeginListBox("##FileListBox", listBoxSize)) {
    for (const std::string& e : files) {
      const bool isSelected = e == filename;
      if (Selectable(e.c_str(), isSelected)) {
        selected = true;
        filename = e;
      }
    }
    EndListBox();
  }

  return selected;
}

/**
* �A�N�^�[�̏���\������
*/
void MapEditor::ShowActorInfo(std::shared_ptr<Actor> actor)
{
  using namespace ImGui;

  SetNextWindowSize(ImVec2(400, GetFrameHeightWithSpacing() * 6), ImGuiCond_Once);

  Begin(u8"�A�N�^�[���");
  Text(u8"���O: %s", actor->name.c_str());

  Text(u8"�w���X:");
  SameLine();
  InputFloat("##health", &actor->health);

  Text(u8"�^�O:");
  SameLine();
  char previewTag[256];
  snprintf(previewTag, 255, "%s", ActorTagToString(actor->tag));
  if (BeginCombo("##actorTag", previewTag)) {
    for (int i = 0; i < actorTagCount; ++i) {
      const ActorTag tag = static_cast<ActorTag>(i);
      char label[256];
      snprintf(label, 255, "%s", ActorTagToString(tag));
      const bool isSelected = actor->tag == tag;
      if (ImGui::Selectable(label, isSelected)) {
        actor->tag = tag;
      }
      if (isSelected) {
        SetItemDefaultFocus();
      }
    }
    EndCombo();
  }

  Text(u8"��]:");
  SameLine();
  static const char* strRotation[] = { "0", "90", "180", "270" };
  SetNextItemWidth(GetWindowContentRegionWidth() - 150);
  SliderInt("##rotation", &gui.rotation,
    0, static_cast<int>(std::size(strRotation)) - 1,
    strRotation[gui.rotation], ImGuiSliderFlags_NoInput);
  const float radians = glm::radians(static_cast<float>(gui.rotation) * 90.0f);
  if (radians != actor->rotation) {
    actor->rotation = radians;
    actor->collider = cursorOriginal->collider->Clone();
    actor->collider->Scale(gui.scale);
    actor->collider->RotateY(radians);
  }
  SameLine();
  Text(u8"�����_��:");
  SameLine();
  Checkbox("##randomRotation", &randomRotationFlag);

  Text(u8"�X�P�[��:");
  SameLine();
  SetNextItemWidth(GetWindowContentRegionWidth() - 150);
  SliderFloat3("##scale", &gui.scale.x, 0.5f, 5.0f);
  if (actor->scale != cursorOriginal->scale * gui.scale) {
    actor->scale = cursorOriginal->scale * gui.scale;
    actor->collider = cursorOriginal->collider->Clone();
    actor->collider->Scale(gui.scale);
    actor->collider->RotateY(radians);
  }
  SameLine();
  Text(u8"�����_��:");
  SameLine();
  Checkbox("##randomScale", &randomScaleFlag);

  // �t���O�A�N�^�[�̃t���O�����o��\������
  std::function<int()> getFlagNo;
  std::function<void(int)> setFlagNo;
  {
    FlagIfDied* p = dynamic_cast<FlagIfDied*>(actor.get());
    if (p) {
      Text(u8"���삷��t���O:");
      getFlagNo = [p]() { return p->GetFlagNo(); };
      setFlagNo = [p](int no) { p->SetFlagNo(no); };
    }
  }
  {
    MoveIfFlagged* p = dynamic_cast<MoveIfFlagged*>(actor.get());
    if (p) {
      Text(u8"�Q�[�g���J���t���O:");
      getFlagNo = [p]() { return p->GetFlagNo(); };
      setFlagNo = [p](int no) { p->SetFlagNo(no); };
    }
  }

  // �t���O�I���R���{�{�b�N�X��\��
  if (getFlagNo && setFlagNo) {
    GameManager& manager = GameManager::Get();
    SameLine();
    const int flagNo = getFlagNo();
    char preview[256];
    snprintf(preview, 255, "%04d: %s", flagNo, manager.GetGameFlagDesc(flagNo).c_str());
    if (BeginCombo("##flag", preview)) {
      const size_t size = manager.GetGameFlagCount();
      for (int i = 0; i < size; ++i) {
        char label[256];
        snprintf(label, 255, "%04d: %s", i, manager.GetGameFlagDesc(i).c_str());
        const bool isSelected = flagNo == i;
        if (ImGui::Selectable(label, isSelected)) {
          setFlagNo(i);
        }
        if (isSelected) {
          SetItemDefaultFocus();
        }
      }
      EndCombo();
    }
  }

  End();
}

/**
* �t���O�ݒ�GUI��\������
*/
void MapEditor::ShowGameFlag()
{
  using namespace ImGui;
  GameManager& manager = GameManager::Get();
  static std::string gameFlagDesc;
  static int gameFlagNo = 0;
  static int gameFlagCount =
    static_cast<int>(manager.GetGameFlagCount());

  SetNextWindowPos(ImVec2(1000, 30), ImGuiCond_Once);
  SetNextWindowSize(ImVec2(250, 200), ImGuiCond_Once);
  Begin(u8"�Q�[���t���O");

  // �Q�[���t���O�̑I��
  const float windowHeight =
    GetWindowContentRegionMax().y - GetWindowContentRegionMin().y;
  const float itemHeight = GetFrameHeightWithSpacing();
  const ImVec2 listBoxSize(-1, windowHeight - itemHeight * 2);
  if (BeginListBox("##gameFlags", listBoxSize)) {
    const size_t size = manager.GetGameFlagCount();
    for (int i = 0; i < size; ++i) {
      const std::string desc = manager.GetGameFlagDesc(i);
      char label[256];
      snprintf(label, 255, "%04d: %s", i, desc.c_str());
      const bool isSelected = i == gameFlagNo;
      if (Selectable(label, isSelected)) {
        gameFlagNo = i;
        gameFlagDesc = desc;
      }
    }
    EndListBox();
  }

  // �������̐ݒ�
  Text(u8"����:");
  SameLine();
  SetNextItemWidth(-1);
  gameFlagDesc.resize(255); // ���͗p�ɒ������m��
  if (InputText("##desc", gameFlagDesc.data(), gameFlagDesc.size())) {
    gameFlagDesc.resize(gameFlagDesc.find_first_of('\0')); // ������߂�
    manager.SetGameFlagDesc(gameFlagNo, gameFlagDesc);
  }

  // �t���O���̕ύX(�{�^��)
  if (Button(u8"�t���O���̕ύX")) {
    gameFlagCount = static_cast<int>(manager.GetGameFlagCount());
    OpenPopup(u8"�t���O���̕ύX##popup");
  }

  // �t���O���̕ύX(�|�b�v�A�b�v�E�B���h�E)
  SetNextWindowSize(ImVec2(-1, -1));
  if (BeginPopupModal(u8"�t���O���̕ύX##popup")) {
    InputInt("##gameFlagCount", &gameFlagCount);
    if (Button(u8"����")) {
      gameFlagNo = std::min(gameFlagNo, gameFlagCount - 1);
      manager.SetGameFlagCount(gameFlagCount);
      CloseCurrentPopup();
    }
    if (Button(u8"�L�����Z��")) {
      CloseCurrentPopup();
    }
    EndPopup();
  }

  End();
}

/**
*
*/
void MapEditor::ShowGameRule()
{
  using namespace ImGui;
  GameManager& manager = GameManager::Get();

  Begin(u8"�Q�[�����[��");
  Text(u8"�j��Ώ�:");
  static const ActorTag tags[] = {
    ActorTag::enemy,
    ActorTag::boss,
    ActorTag::destructionTarget,
  };
  for (auto tag : tags) {
    bool flag = manager.GetTargetFlag(tag);
    if (Checkbox(ActorTagToString(tag), &flag)) {
      manager.SetTargetFlag(tag, flag);
    }
  }
  End();
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
    case Mode::select:
      if (select) {
        select->color = glm::vec4(1);
      }
      select = target;
      if (select) {
        select->color = glm::vec4(1.0f, 0.5f, 0.2f, 0.5f);
        gui.rotation = static_cast<int>(select->rotation / glm::radians(90.0f) + 0.5f);
        gui.scale = select->scale;
      }
      break;

    case Mode::set:
      if (randomScaleFlag || randomRotationFlag) {
        cursor->collider = cursorOriginal->collider->Clone();
      }

      // �����_���X�P�[������̏ꍇ�A�����_���ȃX�P�[����ݒ肷��
      if (randomScaleFlag) {
        glm::vec3 scale;
        scale.x = glm::clamp(engine.GetRandomNormal(1.0f, 0.2f), 0.75f, 1.5f);
        scale.y = glm::clamp(engine.GetRandomNormal(1.0f, 0.2f), 0.75f, 1.5f);
        scale.z = scale.x;
        cursor->scale = cursorOriginal->scale * scale;
        cursor->collider->Scale(scale);
      }

      // �����_����]����̏ꍇ�A�����_���ȕ����ɉ�]������
      if (randomRotationFlag) {
        const int rotation = engine.GetRandomInt(0, 3);
        cursor->rotation = glm::radians(static_cast<float>(rotation) * 90.0f);
        cursor->collider->RotateY(cursor->rotation);
      }

      // �I���A�N�^�[�Ǝ�ނ������ꍇ�͔z�u���Ȃ�
      if (target && target->name == cursor->name) {
        target->rotation = cursor->rotation;
        target->scale = cursor->scale;
        target->collider = cursor->collider->Clone();
        break;
      }
      // �����̃A�N�^�[������΍폜����
      if (target) {
        target->isDead = true;
      }
      // �I�𒆂̃A�N�^�[��z�u����
      target = cursor->Clone();
      target->collider = cursor->collider->Clone();
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
  if (ImGui::GetIO().WantCaptureKeyboard) {
    return;
  }

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

  // �J�����ʒu�𑀍삷��
  ImGuiIO& io = ImGui::GetIO();
  if (!io.WantCaptureMouse) {
    if (io.MouseWheel >= 1) {
      if (cameraOffset.y >= 10) {
        cameraOffset -= glm::vec3(0, 3, 3);
      }
    } else if (io.MouseWheel <= -1) {
      cameraOffset += glm::vec3(0, 3, 3);
    }
  }
  camera.position = camera.target + cameraOffset;

  // �J�[�\���ʒu��ݒ�
  const glm::mat4 matVP = camera.GetProjectionMatrix() * camera.GetViewMatrix();
  glm::vec3 start, end, p;
  const ImVec2 mousePos = ImGui::GetMousePos();
  ScreenPosToLine(glm::vec2(mousePos.x, mousePos.y), matVP, start, end);
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
      // �c�[�����ύX���ꂽ��I����Ԃ���������
      if (mode != modeList[i]) {
        if (select) {
          select->color = glm::vec4(1);
          select.reset();
        }
      }
      mode = modeList[i];
    }
  }

  SameLine();
  if (Button(u8"�Z�[�u")) {
    OpenPopup(u8"�Z�[�u�t�@�C���I��");
  }
  SetNextWindowSize(ImVec2(400, -1), ImGuiCond_Once);
  if (BeginPopupModal(u8"�Z�[�u�t�@�C���I��")) {
    static std::string buf(256, '\0');
    static std::string filename;
    if (ShowFileListBox(filename)) {
      buf = filename;
      buf.resize(256);
    }
    Text(u8"�t�@�C����");
    SameLine();
    InputText("##filename", buf.data(), buf.size());
    if (Button(u8"�Z�[�u")) {
      Save(buf.c_str());
      CloseCurrentPopup();
    }
    SameLine();
    if (Button(u8"�L�����Z��")) {
      CloseCurrentPopup();
    }
    EndPopup();
  }

  SameLine();
  if (Button(u8"���[�h")) {
    OpenPopup(u8"���[�h�t�@�C���I��");
  }
  SetNextWindowSize(ImVec2(400, -1), ImGuiCond_Once);
  if (BeginPopupModal(u8"���[�h�t�@�C���I��")) {
    static std::string filename;
    ShowFileListBox(filename);
    if (Button(u8"���[�h") && !filename.empty()) {
      Load(filename.c_str());
      CloseCurrentPopup();
    }
    SameLine();
    if (Button(u8"�L�����Z��")) {
      CloseCurrentPopup();
    }
    EndPopup();
  }

  SameLine();
  static glm::ivec2 newMapSize;
  if (Button(u8"�V�K�쐬")) {
    OpenPopup(u8"�V�����}�b�v�̍L��");
    newMapSize = glm::ivec2(21, 21);
  }
  if (BeginPopupModal(u8"�V�����}�b�v�̍L��")) {
    Text(u8"��");
    SameLine();
    SetNextItemWidth(-1);
    SliderInt("##map size x", &newMapSize.x, 11, 101, "%d", ImGuiSliderFlags_AlwaysClamp);
    newMapSize.x = newMapSize.x | 1;
    Text(u8"�c");
    SameLine();
    SetNextItemWidth(-1);
    SliderInt("##map size y", &newMapSize.y, 11, 101, "%d", ImGuiSliderFlags_AlwaysClamp);
    newMapSize.y = newMapSize.y | 1;
    if (Button(u8"���̃T�C�Y�ō쐬")) {
      // �}�b�v�f�[�^������
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
    if (Button(u8"�L�����Z��")) {
      CloseCurrentPopup();
    }
    EndPopup();
  }

  SameLine();
  if (Button(u8"�}�b�v�T�C�Y")) {
    OpenPopup(u8"�}�b�v�T�C�Y�ύX");
    newMapSize = mapSize;
  }
  SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Once);
  if (BeginPopupModal(u8"�}�b�v�T�C�Y�ύX")) {
    Text(u8"��");
    SameLine();
    SliderInt("##map size x", &newMapSize.x, 11, 101, "%d", ImGuiSliderFlags_AlwaysClamp);
    newMapSize.x = newMapSize.x | 1;
    Text(u8"�c");
    SameLine();
    SliderInt("##map size y", &newMapSize.y, 11, 101, "%d", ImGuiSliderFlags_AlwaysClamp);
    newMapSize.y = newMapSize.y | 1;
    if (Button("OK")) {
      Resize(newMapSize);
      CloseCurrentPopup();
    }
    SameLine();
    if (Button(u8"�L�����Z��")) {
      CloseCurrentPopup();
    }
    EndPopup();
  }

  Text(toolName[static_cast<int>(mode)]);
  SameLine(GetWindowWidth() - 140);
  Text(u8"�J�[�\�����W(%d, %d)",
    static_cast<int>(cursor->position.x / 4),
    static_cast<int>(cursor->position.z / 4));
  End();

  SetNextWindowSize(ImVec2(200, 0), ImGuiCond_Once);
  Begin(u8"�A�N�^�[�I��");
  const ImVec2 actorListBoxSize(-1,
    GetTextLineHeightWithSpacing() * actors.size() + GetStyle().FramePadding.y * 2);
  if (BeginListBox("ActorList", actorListBoxSize)) {
    for (int i = 0; i < actors.size(); ++i) {
      const bool isSelected = cursor == actors[i];
      if (Selectable(actors[i]->name.c_str(), isSelected)) {
        cursorOriginal = actors[i]; // ���ɂȂ����A�N�^�[���L�^���Ă���
        // �^�����ێ����邽�߂ɐV�����N���[�������
        cursor->isDead = true; // �Â��N���[���͍폜����
        cursor = actors[i]->Clone();
        cursor->collider = cursor->collider->Clone();
        cursor->color = glm::vec4(0.2f, 0.5f, 1.0f, 0.5f);
        engine.AddActor(cursor);
      }
      if (isSelected) {
        SetItemDefaultFocus();
      }
    }
    EndListBox();
  }
  End();

  if (mode == Mode::select && select) {
    ShowActorInfo(select);
  } else {
    ShowActorInfo(cursor);
  }
  ShowGameFlag();
  ShowGameRule();

  if (mode == Mode::groundPaint) {
    SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Once);
    Begin(u8"�n�ʃ^�C���I��");
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
    snprintf(tmp, std::size(tmp),
      "  [ %s, %.03f, %.03f, %.03f, %.03f, %.03f, %.03f, %.03f",
      e->name.c_str(), e->position.x, e->position.y, e->position.z,
      e->scale.x, e->scale.y, e->scale.z, glm::degrees(e->rotation));
    ofs << tmp;

    snprintf(tmp, std::size(tmp), ", { health: %.03f }", e->health);
    ofs << tmp;

    snprintf(tmp, std::size(tmp), ", { actorTag: %s }", ActorTagToString(e->tag));
    ofs << tmp;

    // �t���O�ԍ��������o��
    FlagIfDied* p0 = dynamic_cast<FlagIfDied*>(e.get());
    if (p0) {
      ofs << ", { flagNo: " << p0->GetFlagNo() << " }";
    }
    MoveIfFlagged* p1 = dynamic_cast<MoveIfFlagged*>(e.get());
    if (p1) {
      ofs << ", { flagNo: " << p1->GetFlagNo() << " }";
    }
    ofs << " ],\n";
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

  ofs << "gameFlags: {\n";
  GameManager& manager = GameManager::Get();
  const size_t gameFlagCount = manager.GetGameFlagCount();
  for (int i = 0; i < gameFlagCount; ++i) {
    char tmp[256];
    snprintf(tmp, 255, "  %d: \"%s\",\n", i,
      manager.GetGameFlagDesc(i).c_str());
    ofs << tmp;
  }
  ofs << "}\n";

  ofs << "targetFlags: {\n";
  for (int i = 0; i < actorTagCount; ++i) {
    const ActorTag tag = static_cast<ActorTag>(i);
    ofs << "  " << ActorTagToString(tag) << ": " <<
      std::boolalpha << manager.GetTargetFlag(tag) << ",\n";
  }
  ofs << "}\n";
}

/**
* �A�N�^�[���X�g����A�N�^�[���擾����
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
* �}�b�v�f�[�^���t�@�C������ǂݍ���
*/
bool MapEditor::Load(const char* filename)
{
  std::ifstream ifs(filename);
  if (!ifs) {
    std::cerr << "[�G���[]" << __func__ << ":" << filename << "���J���܂���\n";
    return false;
  }

  // �}�b�v�f�[�^�ǂݍ��ݗp�ϐ�
  glm::ivec2 tmpMapSize;
  std::vector<std::shared_ptr<Actor>> tmpMap;
  std::vector<int> tmpGameMap;

  // �}�b�v�T�C�Y��ǂݍ���
  std::string line;
  std::getline(ifs, line);
  if (sscanf(line.data(), "mapSize: [ %d, %d ],",
    &tmpMapSize.x, &tmpMapSize.y) != 2) {
    std::cerr << "[�G���[]" << __func__ << ": �}�b�v�T�C�Y�̓ǂݍ��݂Ɏ��s\n";
    return false;
  }
  tmpMap.resize(tmpMapSize.x * tmpMapSize.y);
  tmpGameMap.resize(tmpMapSize.x * tmpMapSize.y, 0);

  // map�s��ǂݔ�΂�
  std::getline(ifs, line);
  if (line != "map: [") {
    std::cerr << "[�G���[]" << __func__ << ": �}�b�v�f�[�^�̓ǂݍ��݂Ɏ��s\n";
    return false;
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
    glm::vec3 scale(1);
    float rotation = 0;
    int n = 0; // �ǂݍ��񂾃o�C�g��
    if (sscanf(line.data(), " [ %255[^,], %f, %f, %f, %f, %f, %f, %f%n",
      name, &position.x, &position.y, &position.z,
      &scale.x, &scale.y, &scale.z, &rotation, &n) < 4) {
      std::cerr << "[�x��]" << __func__ << ": �z�u�f�[�^�̓ǂݍ��݂Ɏ��s\n" <<
        "  " << line << "\n";
    }
    rotation = glm::radians(rotation);
    name[255] = '\0';

    const char* p = line.data() + n;
    float health = -1;
    int flagNo = 0;
    int tag = -1;
    for (;;) {
      char dataName[100];
      char data[100];
      if (sscanf(p, " , { %99[^: ] : %99[^} ] }%n", dataName, &data, &n) < 2) {
        break;
      }

      dataName[99] = '\0';
      data[99] = '\0';
      p += n;

      if (strcmp("health", dataName) == 0) {
        health = std::stof(data);
      }
      else if (strcmp("flagNo", dataName) == 0) {
        flagNo = std::stoi(data);
      }
      else if (strcmp("actorTag", dataName) == 0) {
        tag = static_cast<int>(StringToActorTag(data));
      }
    }

    // �A�N�^�[���擾
    int actorNo = 0;
    std::shared_ptr<Actor> actor = GetActor(name, &actorNo);
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
    std::shared_ptr<Actor> newActor = actor->Clone();
    newActor->collider = actor->collider->Clone();
    newActor->collider->Scale(scale / actor->scale);
    newActor->collider->RotateY(rotation);
    if (health >= 0) {
      newActor->health = health;
    }
    if (tag >= 0) {
      newActor->tag = static_cast<ActorTag>(tag);
    }
    newActor->position = position;
    newActor->scale = scale;
    newActor->rotation = rotation;
    FlagIfDied* p0 = dynamic_cast<FlagIfDied*>(newActor.get());
    if (p0) {
      p0->SetFlagNo(flagNo);
    }
    MoveIfFlagged* p1 = dynamic_cast<MoveIfFlagged*>(newActor.get());
    if (p1) {
      p1->SetFlagNo(flagNo);
      p1->SetPosition(false, position);
      p1->SetPosition(true, position + glm::vec3(0, -100, 0));
    }

    tmpMap[x + y * tmpMapSize.x] = newActor;
    tmpGameMap[x + y * tmpMapSize.x] = actorNo;
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

  // �Q�[���t���O��ǂݍ���
  std::vector<std::string> tmpGameFlags;
  tmpGameFlags.reserve(20);
  std::getline(ifs, line);
  if (line == "gameFlags: {") {
    while (!ifs.eof()) {
      std::getline(ifs, line);

      // �f�[�^�̏I���`�F�b�N
      if (line[0] == '}') {
        break;
      }
      // �s�����
      char* p = line.data();
      int no = 0; // ���܂̂Ƃ��떢�g�p
      char name[256];
      name[0] = '\0';
      if (sscanf(p, " %d : \"%255[^\"]\" ,", &no, name) < 1) {
        std::cerr << "[�x��]" << __func__ << ": " <<
          tmpGameFlags.size() << "�Ԗڂ̃Q�[���t���O�𐳂����ǂݍ��߂܂���ł���\n";
      }
      name[255] = '\0';
      tmpGameFlags.push_back(name);
    }
  }

  // �j��Ώۃt���O��ǂݍ���
  bool tmpTargetFlags[actorTagCount] = {};
  bool hasTargetFlags = false;
  std::getline(ifs, line);
  if (line == "targetFlags: {") {
    hasTargetFlags = true;
    while (!ifs.eof()) {
      std::getline(ifs, line);

      // �f�[�^�̏I���`�F�b�N
      if (line[0] == '}') {
        break;
      }

      // �s�����
      char name[256];
      char flag[10];
      if (sscanf(line.data(), " %255[^: ] : %9[^, ] ,", name, flag) < 2) {
        std::cerr << "[�x��]" << __func__ << ": " <<
          "�^�[�Q�b�g�^�O�𐳂����ǂݍ��߂܂���ł���\n";
        continue;
      }
      name[255] = '\0';
      flag[9] = '\0';
      const ActorTag tag = StringToActorTag(name);
      if (strcmp(flag, "true") == 0) {
        tmpTargetFlags[static_cast<int>(tag)] = true;
      }
    }
  }

  // �ǂݍ��񂾃f�[�^�������o�ϐ��ɔ��f����
  mapSize = tmpMapSize;
  map.swap(tmpMap);
  groundMap.swap(tmpGroundMap);
  gameMap.swap(tmpGameMap);

  GameEngine& engine = GameEngine::Get();
  engine.ResizeGroundMap(mapSize.x, mapSize.y, groundMap.data());

  // �Q�[���t���O��ݒ�
  GameManager& manager = GameManager::Get();
  manager.SetGameFlagCount(tmpGameFlags.size());
  for (int i = 0; i < tmpGameFlags.size(); ++i) {
    manager.SetGameFlagDesc(i, tmpGameFlags[i]);
  }

  // �j��Ώۃt���O��ݒ�
  manager.ClearAllTargetFlags();
  if (hasTargetFlags) {
    for (int i = 0; i < actorTagCount; ++i) {
      manager.SetTargetFlag(static_cast<ActorTag>(i), tmpTargetFlags[i]);
    }
  } else {
    // �j��Ώۃt���O���̂Ȃ��Z�[�u�f�[�^�̏ꍇ�A�G�ƃ{�X��j��Ώۂɂ���
    manager.SetTargetFlag(ActorTag::enemy, true);
    manager.SetTargetFlag(ActorTag::boss, true);
  }

  // �Q�[���G���W���̃A�N�^�[���X�V
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

  std::cerr << "[���]" << __func__ << ": " << filename << "�����[�h\n";
  return true;
}

