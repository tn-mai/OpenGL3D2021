/**
* @file GameManager.h
*/
#ifndef GAMEMANAGER_H_INCLUDED
#define GAMEMANAGER_H_INCLUDED
#include "Actor.h"
#include <vector>

/**
* �Q�[���Ǘ��N���X
*/
class GameManager
{
public:
  static bool Initialize();
  static void Finalize();
  static GameManager& Get();

  // �Q�[���̓�����
  enum class State {
    title,     // �^�C�g�����(19b�Ŏ���. 19�͖�����)
    start,     // �Q�[���J�n
    playing,   // �Q�[���v���C��
    gameclear, // �Q�[���N���A
    gameover,  // �Q�[���I�[�o�[
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

  // 19b�Ŏ���. 19�͖�����.
  void UpdateGameUI();
  void UpdateTitle(float deltaTime);
  void UpdateTitleUI();
  // �����܂�(19b�Ŏ���. 19�͖�����)

  // TODO: �e�L�X�g������
  void UpdateStage2(float deltaTime);

  State state = State::title; // ���݂̓�����
  std::shared_ptr<Actor> playerTank;
  std::vector<std::shared_ptr<Actor>> enemies;
  int score = 0;
  size_t stageNo = 0; // 21b�Ŏ���. 21�͖�����

  // 19b�Ŏ���. 19�͖�����.

  // �^�C�g����ʂ̓�����
  enum class TitleState {
    init,       // �^�C�g����ʂ̏�����
    logoFadein, // ���S�t�F�[�h�C��
    bgFadein,   // �w�i�t�F�[�h�C��
    idle,       // ���[�U�[�̓��͑҂�
    fadeout,    // �^�C�g����ʃt�F�[�h�A�E�g
  };
  TitleState titleState = TitleState::init;
  float titleLogoAlpha = 0; // �^�C�g�����S�̕s�����x
  float titleBgAlpha = 0;   // �^�C�g���w�i�̕s�����x
  float titleEffectPosX[2] = { 0, 0 };
  float fadeAlpha = 0;      // �t�F�[�h�C���E�A�E�g�̕s�����x
  bool startHovered = false;
  bool exitHovered = false;
  // �����܂�(19b�Ŏ���. 19�͖�����)
};

#endif // GAMEMANAGER_H_INCLUDED
