/**
* @file GameManager.cpp
*/
#include "GameManager.h"
#include "GameEngine.h"
#include "Actor/PlayerActor.h"
#include "Actor/T34TankActor.h"
#include <iostream>

namespace {

GameManager* manager = nullptr;

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
    "start", "playing", "gameclear", "gameover" };
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
    enemy->collider = Box{ glm::vec3(-1.5f, 0, -1.5f), glm::vec3(1.5f, 2.5f, 1.5f) };
    enemy->mass = 36'000;
    enemies.push_back(enemy);
  }

  // T-34��Ԃ��Q�[���G���W���ɓo�^
  for (int i = 0; i < enemies.size(); ++i) {
    engine.AddActor(enemies[i]);
  }
}

