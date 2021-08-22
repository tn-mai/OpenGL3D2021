/**
* @file GameManager.h
*/
#ifndef GAMEMANAGER_H_INCLUDED
#define GAMEMANAGER_H_INCLUDED
#include "Actor.h"
#include <vector>

/**
* ƒQ[ƒ€ŠÇ—ƒNƒ‰ƒX
*/
class GameManager
{
public:
  static bool Initialize();
  static void Finalize();
  static GameManager& Get();

  enum class State {
    start,
    playing,
    gameclear,
    gameover,
  };
  void SetState(State s);

  void Update(float deltaTime);

private:
  GameManager() = default;
  ~GameManager() = default;
  GameManager(const GameManager&) = delete;
  GameManager& operator=(const GameManager&) = delete;

  void SpawnPlayer();
  void SpawnEnemies();

  State state = State::start;
  std::shared_ptr<Actor> playerTank;
  std::vector<std::shared_ptr<Actor>> enemies;
};

#endif // GAMEMANAGER_H_INCLUDED
