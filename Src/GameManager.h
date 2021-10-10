/**
* @file GameManager.h
*/
#ifndef GAMEMANAGER_H_INCLUDED
#define GAMEMANAGER_H_INCLUDED
#include "Actor.h"
#include <vector>

/**
* ÉQÅ[ÉÄä«óùÉNÉâÉX
*/
class GameManager
{
public:
  static bool Initialize();
  static void Finalize();
  static GameManager& Get();

  enum class State {
    initializeLevel,
    start,
    playing,
    gameclear,
    gameover,
  };
  void SetState(State s);

  void Update(float deltaTime);
  void UpdateCamera();
  void UpdateUI();

  void AddScore(int n) { score += n; }

private:
  GameManager() = default;
  ~GameManager() = default;
  GameManager(const GameManager&) = delete;
  GameManager& operator=(const GameManager&) = delete;

  void LoadPrimitives();
  void LoadTextures();
  void SpawnPlayer();
  void SpawnEnemies();
  void SpawnMap();

  State state = State::initializeLevel;
  std::shared_ptr<Actor> playerTank;
  std::vector<std::shared_ptr<Actor>> enemies;
  int score = 0;
};

#endif // GAMEMANAGER_H_INCLUDED
