/**
* @file GameManager.cpp
*/
#include "GameManager.h"
#include "GameEngine.h"
#include "Actor/PlayerActor.h"
#include "Actor/T34TankActor.h"
#include "Actor/ElevatorActor.h"
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace {

GameManager* manager = nullptr;

/// �}�b�v�f�[�^.
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

/// �I�u�W�F�N�g�}�b�v�f�[�^.
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
* �Q�[���}�l�[�W���̏�����
*/
bool GameManager::Initialize()
{
  if (!manager) {
    manager = new GameManager;
  }
  return true;
}

/**
* �Q�[���}�l�[�W���̏I��
*/
void GameManager::Finalize()
{
  if (manager) {
    delete manager;
    manager = nullptr;
  }
}

/**
* �Q�[���G���W�����擾����
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
    "gameover" };
  std::cout << names[static_cast<int>(state)] <<
    "->" << names[static_cast<int>(s)] << "\n";

  state = s;
}

/**
* �Q�[���̓����Ԃ��X�V����
*/
void GameManager::Update(float deltaTime)
{
  GameEngine& engine = GameEngine::Get();
  switch (state) {
  case State::initializeLevel:
    LoadPrimitives();
    LoadTextures();
    SpawnMap();
    SetState(State::start);
    break;

  case State::start:
    SpawnPlayer();
    SpawnEnemies();
    {
      std::shared_ptr<Actor> gamestart(new Actor{ "GameStart",
        engine.GetPrimitive("Res/Plane.obj"),
        engine.LoadTexture("Res/GameStart.tga"),
        glm::vec3(0, 5, 0), glm::vec3(800.0f, 200.0f, 1.0f), 0.0f, glm::vec3(0) });
      gamestart->lifespan = 3;
      gamestart->isStatic = true;
      gamestart->layer = Layer::UI;
      engine.AddActor(gamestart);
    }
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
      SetState(State::gameover);
    }
    else {
      bool allKill = true;
      for (int i = 0; i < enemies.size(); ++i) {
        if (!enemies[i]->isDead) {
          allKill = false;
          break;
        }
      }
      if (allKill) {
        std::shared_ptr<Actor> gameclear(new Actor{ "GameClear",
          engine.GetPrimitive("Res/Plane.obj"),
          engine.LoadTexture("Res/GameClear.tga"),
          glm::vec3(0), glm::vec3(700, 200, 1), 0.0f, glm::vec3(0) });
        gameclear->isStatic = true;
        gameclear->layer = Layer::UI;
        engine.AddActor(gameclear);
        SetState(State::gameclear);
      }
    }
    break;

  case State::gameclear:
    if (engine.GetKey(GLFW_KEY_ENTER)) {
      std::shared_ptr<Actor> gameclear = engine.FindActor("GameClear");
      if (gameclear) {
        gameclear->isDead = true;
      }
      SetState(State::start);
    }
    break;

  case State::gameover:
    if (engine.GetKey(GLFW_KEY_ENTER)) {
      std::shared_ptr<Actor> gameover = engine.FindActor("GameOver");
      if (gameover) {
        gameover->isDead = true;
      }
      SetState(State::start);
    }
    break;
  }
}

/**
* �J�����̏�Ԃ��X�V����
*/
void GameManager::UpdateCamera()
{
  GameEngine& engine = GameEngine::Get();

  // �J�����f�[�^���X�V����
  std::shared_ptr<Actor> target = Find(engine.GetActors(), "Tiger-I");
  if (target) {
    const glm::mat4 matRot = glm::rotate(glm::mat4(1), target->rotation, glm::vec3(0, 1, 0));
    const glm::vec3 tankFront = matRot * glm::vec4(0, 0, 1, 1);
    Camera& camera = engine.GetCamera();
    camera.position = target->position + glm::vec3(0, 20, 20);
    camera.target = target->position;
  }
}

/**
* UI�̏�Ԃ��X�V����
*/
void GameManager::UpdateUI()
{
  /*
  * �\����
  * - �X�R�A
  * - �v���C���[��HP
  * - ���C������̎��
  * - �T�u����̎��/�c��
  * - �X�e�[�W��
  * - �~�j�}�b�v
  * - �G��HP
  */
  ImGuiStyle& style = ImGui::GetStyle();
  ImGuiStyle oldStyle = style;
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

  static bool show_demo_window = true;
  static bool show_another_window = false;
  static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  
  static float f = 0.0f;
  static int counter = 0;
  
  if (show_demo_window)
    ImGui::ShowDemoWindow(&show_demo_window);

  ImGui::Begin("Hello, world!");

//  auto tex = GameEngine::Get().LoadTexture("Res/Green.tga");
//  ImGui::Image(reinterpret_cast<ImTextureID>(tex->GetId()), ImVec2(tex->GetWidth(), tex->GetHeight()),
//    ImVec2(0, 0) , ImVec2(1, 1), ImVec4(1,1,1,1));
//  ImGui::SetCursorPos(ImVec2(10, 10));
//  ImGui::BeginChildFrame(1, ImVec2(400, 400), ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);

  //ImGui::SetWindowFontScale(2);
  ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
  //ImGui::SetWindowFontScale(1);
  ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
  ImGui::Checkbox("Another Window", &show_another_window);
  
  ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
  ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
  
  if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
      counter++;

  ImGui::SameLine();
  ImGui::Text("counter = %d", counter);
  
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
//  ImGui::EndChildFrame();
  ImGui::End();

  if (show_another_window)
  {
    ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::Text("Hello from another window!");
    if (ImGui::Button("Close Me"))
        show_another_window = false;
    ImGui::End();
  }
}

/**
* �`��f�[�^��ǉ�����.
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
* �e�N�X�`�����쐬.
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
* �v���C���[�̐�Ԃ𐶐�����
*/
void GameManager::SpawnPlayer()
{
  // �ȑO�ɍ쐬�����^�C�K�[I��Ԃ��폜
  if (playerTank) {
    playerTank->isDead = true;
  }

  // �V�����^�C�K�[I��Ԃ��쐬
  playerTank.reset(new PlayerActor{ glm::vec3(0), glm::vec3(1), 0.0f });

  // �^�C�K�[I��Ԃ��Q�[���G���W���ɓo�^
  GameEngine::Get().AddActor(playerTank);
}

/**
* �G��Ԃ𐶐�
*/
void GameManager::SpawnEnemies()
{
  GameEngine& engine = GameEngine::Get();

  // �ȑO�ɍ쐬����T-34��Ԃ��폜
  for (int i = 0; i < enemies.size(); ++i) {
    if (enemies[i]) {
      enemies[i]->isDead = true;
    }
  }
  enemies.clear();

  // �V����T-34��Ԃ��쐬
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
    enemy->collider = CreateCylinderShape(glm::vec3(0), 1.5f, 2.5f);
    enemy->mass = 36'000;
    enemies.push_back(enemy);
  }

  // T-34��Ԃ��Q�[���G���W���ɓo�^
  for (int i = 0; i < enemies.size(); ++i) {
    engine.AddActor(enemies[i]);
  }
}

/**
*
*/
void GameManager::SpawnMap()
{
  // �}�b�v�ɔz�u���镨�̂̕\���f�[�^.
  struct ObjectData {
    const char* name;
    Primitive prim;
    const std::shared_ptr<Texture> tex;
    float scale = 1.0f;
    glm::vec3 ajustment = glm::vec3(0);
    std::shared_ptr<Collider> collider;
  };

  GameEngine& engine = GameEngine::Get();

  // ��ʒ[�ɃR���C�_�[��ݒ�
  ActorList& actors = engine.GetActors();
  PrimitiveBuffer& primitiveBuffer = engine.GetPrimitiveBuffer();
  std::shared_ptr<Texture> texTriangle = engine.LoadTexture("Res/Triangle.tga");

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(-36, 0, -34), glm::vec3(1, 2, 32), 0.0f, glm::vec3(0) }));
  actors.back()->collider = CreateBoxShape(glm::vec3(0, 0, 0), glm::vec3(1, 4, 64));
  actors.back()->isStatic = true;

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(30, 0, -34), glm::vec3(1, 2, 32), 0.0f, glm::vec3(0) }));
  actors.back()->collider = CreateBoxShape(glm::vec3(0, 0, 0), glm::vec3(1, 4, 64));
  actors.back()->isStatic = true;

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(-34, 0, -36), glm::vec3(32, 2, 1), 0.0f, glm::vec3(0) }));
  actors.back()->collider = CreateBoxShape(glm::vec3(0, 0, 0), glm::vec3(64, 4, 1));
  actors.back()->isStatic = true;

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(-34, 0, 30), glm::vec3(32, 2, 1), 0.0f, glm::vec3(0) }));
  actors.back()->collider = CreateBoxShape(glm::vec3(0, 0, 0), glm::vec3(64, 4, 1));
  actors.back()->isStatic = true;

  // �`�悷�镨�̂̃��X�g.
  //std::shared_ptr<Box> col1 = CreateBoxCollider(glm::vec3(-1.75f, 0, -1.75f), glm::vec3(1.75f, 2, 1.75f));
  std::shared_ptr<Cylinder> col1 = CreateCylinderShape(glm::vec3(0), 1.75f, 3.0f);
  const ObjectData objectList[] = {
    { "", Primitive(), 0 },    // �Ȃ�
    { "Tree", primitiveBuffer.Get(4), engine.LoadTexture("Res/Tree.tga"), 1, {}, col1 }, // ��
    { "Warehouse", primitiveBuffer.Get(5), engine.LoadTexture("Res/Building.tga"), 1, {},
      CreateBoxShape(glm::vec3(-2.5f, 0, -3.5f), glm::vec3(2.5f, 3, 3.5f)) }, // ����
    { "BrickHouse", primitiveBuffer.Get(8), engine.LoadTexture("Res/house/House38UVTexture.tga"),
      3, glm::vec3(-2.6f, 2.0f, 0.8f), CreateBoxShape(glm::vec3(-3, 0, -2), glm::vec3(3, 3, 2)) }, // ����
    { "House2", primitiveBuffer.Get(10), engine.LoadTexture("Res/house/broken-house.tga"),
      1, {}, CreateBoxShape(glm::vec3(-2.5f, 0, -3.5f), glm::vec3(2.5f, 3, 3.5f)) }, // ����
  };

  // �؂�A����.
  for (int y = 0; y < 16; ++y) {
    for (int x = 0; x < 16; ++x) {
      const int objectNo = objectMapData[y][x];
      if (objectNo <= 0 || objectNo >= std::size(objectList)) {
        continue;
      }
      const ObjectData p = objectList[objectNo];

      // �l�p�`��4x4m�Ȃ̂ŁAx��y��4�{�����ʒu�ɕ\������.
      const glm::vec3 position(x * 4 - 32, 0, y * 4 - 32);

      actors.push_back(std::shared_ptr<Actor>(new Actor{ p.name, p.prim, p.tex,
        position, glm::vec3(p.scale), 0.0f, p.ajustment }));
      actors.back()->collider = p.collider;
      actors.back()->isStatic = true;
      actors.back()->mass = 1'000'000;
    }
  }

  // �}�b�v��(-20,-20)-(20,20)�͈̔͂ɕ`��.
  const std::shared_ptr<Texture> mapTexList[] = {
    engine.LoadTexture("Res/Green.tga"),
    engine.LoadTexture("Res/RoadTiles.tga"),
    engine.LoadTexture("Res/Road.tga") };
  for (int y = 0; y < 16; ++y) {
    for (int x = 0; x < 16; ++x) {
      // �l�p�`��4x4m�Ȃ̂ŁAx��y��4�{�����ʒu�ɕ\������.
      const glm::vec3 position(x * 4 - 32, 0, y * 4 - 32);

      const int textureNo = mapData[y][x];
      actors.push_back(std::shared_ptr<Actor>(new Actor{ "Ground", primitiveBuffer.Get(0), mapTexList[textureNo],
        position, glm::vec3(1), 0.0f, glm::vec3(0) }));
      actors.back()->collider = CreateBoxShape(glm::vec3(-2, -10, -2), glm::vec3(2, 0, 2));
      actors.back()->isStatic = true;
    }
  }

  // �G���x�[�^�[
  {
    const glm::vec3 position(4 * 4 - 20, -2, 4 * 4 - 20);
    actors.push_back(std::shared_ptr<Actor>(new ElevatorActor{
      "Elevator", primitiveBuffer.Get(0), mapTexList[0],
      position, glm::vec3(1), 0.0f, glm::vec3(0) }));
    actors.back()->velocity.y = 1;
    actors.back()->collider = CreateBoxShape(glm::vec3(-2, -10, -2), glm::vec3(2, 0, 2));
    actors.back()->isStatic = true;
  }

  // �O�p�`�̃p�����[�^
  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Triangle", primitiveBuffer.Get(2), texTriangle,
    glm::vec3(0, 0, -5), glm::vec3(1), 0.0f, glm::vec3(0) }));
  // �����̂̃p�����[�^
  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Cube", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(0, 0, -4), glm::vec3(1), 0.0f, glm::vec3(0) }));
}

