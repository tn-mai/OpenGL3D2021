/**
* @file MapEditor.h
*/
#ifndef MAPEDITOR_H_INCLUDED
#define MAPEDITOR_H_INCLUDED
#include "Actor.h"
#include <vector>

/**
* �}�b�v�G�f�B�^
*/
class MapEditor
{
public:
  // ����̎��
  enum class SystemType {
    editor, // �}�b�v�G�f�B�^�p�̃��[�h�������s��
    game,   // �Q�[���p�̃��[�h�������s��
  };

  MapEditor(SystemType type);
  ~MapEditor() = default;

  void Update(float deltaTime);
  void UpdateCamera(float deltaTime);
  void UpdateUI();
  void Save(const char* filename);

  bool Load(const char* filename);
  std::shared_ptr<Actor> GetActor(const char* name, int* no = nullptr) const;

private:
  void LoadCommonPrimitive();
  void InitGroundActor();
  void InitActorList();
  void InitEditor();

  const SystemType systemType;             // ����^�C�v
  glm::vec2 gridSize = glm::vec2(4.0f);    // �}�X�ڂ̃T�C�Y(m)
  glm::ivec2 mapSize = glm::ivec2(21, 21); // �}�b�v�̍L��
  std::vector<std::shared_ptr<Actor>> map; // �A�N�^�[�z�u�}�b�v(�G�f�B�^�p)
  std::vector<int> gameMap;                // �A�N�^�[�z�u�}�b�v(�Q�[���p)
  std::vector<std::shared_ptr<Actor>> actors; // �z�u�\�ȃA�N�^�[
  std::shared_ptr<Actor> cursor; // �}�b�v����J�[�\��
  glm::vec3 cameraOffset = glm::vec3(0, 30, 30); // �J�����̈ʒu

  std::vector<std::shared_ptr<Texture>> groundTiles; // �n�ʗp�e�N�X�`��
  std::vector<uint32_t> groundMap;
  uint8_t currentTileNo = 0;

  // �}�b�v���샂�[�h
  enum class Mode {
    select, // �I�����[�h
    set,    // �z�u���[�h
    remove, // �폜���[�h
    groundPaint, // �n�ʃy�C���g���[�h
  };
  Mode mode = Mode::select;
};

#endif // MAPEDITOR_H_INCLUDED
