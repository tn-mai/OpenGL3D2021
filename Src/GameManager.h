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
    ending,    // �G���f�B���O���
  };
  void SetState(State s);

  void Update(float deltaTime);
  void UpdateCamera();
  void UpdateUI();

  void AddScore(int n) { score += n; }

  // �Q�[���t���O�̑���
  void SetGameFlagCount(size_t size);
  size_t GetGameFlagCount() const;
  void SetGameFlag(size_t no, bool value);
  bool GetGameFlag(size_t no) const;
  void SetGameFlagDesc(size_t no, std::string desc);
  std::string GetGameFlagDesc(size_t no) const;

  // �j��Ώۃt���O�̑���
  bool GetTargetFlag(ActorTag tag) const;
  void SetTargetFlag(ActorTag tag, bool flag);
  void ClearAllTargetFlags();

private:
  GameManager() = default;
  ~GameManager() = default;
  GameManager(const GameManager&) = delete;
  GameManager& operator=(const GameManager&) = delete;

  void LoadPrimitives();
  void LoadTextures();
  void SpawnPlayer();
  void SpawnEnemies();
  void SpawnGrass();
  void SpawnMap();

  // 19b�Ŏ���. 19�͖�����.
  void UpdateGameUI();
  void UpdateTitle(float deltaTime);
  void UpdateTitleUI();
  // �����܂�(19b�Ŏ���. 19�͖�����)

  // �G���f�B���O�p�����o�֐�
  void InitializeEnding();
  void UpdateEnding(float deltaTime);
  void UpdateEndingUI();

  State state = State::title; // ���݂̓�����
  std::shared_ptr<Actor> playerTank;
  std::vector<std::shared_ptr<Actor>> enemies;
  int score = 0;
  size_t stageNo = 0; // 21b�Ŏ���. 21�͖�����

  // �Q�[���t���O
  struct GameFlag
  {
    std::string description; // ������
    bool value = false; // �t���O�̒l
  };
  std::vector<GameFlag> gameFlags;

  bool targetTagFlags[actorTagCount] = {}; // �j��ΏۂƂȂ�^�O
  std::vector<std::shared_ptr<Actor>> targets; // �j��ΏۃA�N�^�[

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

  // �G���f�B���O�p�f�[�^
  struct EndingText {
    uint32_t color; // �F
    std::string text; // ����(1�s��)
  };
  std::vector<EndingText> endingText;
  bool isScrollFinished = false; // false=�X�N���[���� true=��~
  float endingPosY = 0.0f; // �X�N���[����
  float fontSize = 1.0f;   // �G���f�B���O�̕����T�C�Y
};

#endif // GAMEMANAGER_H_INCLUDED
