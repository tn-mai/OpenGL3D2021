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
  MapEditor();
  ~MapEditor() = default;

  void Update(float deltaTime);
  void UpdateCamera(float deltaTime);
  void UpdateUI();
  void Save(const char* filename);
  void Load(const char* filename);

private:
  glm::ivec2 mapSize = glm::ivec2(21, 21); // �}�b�v�̍L��
  std::vector<std::shared_ptr<Actor>> map; // �A�N�^�[�z�u�}�b�v
  std::vector<std::shared_ptr<Actor>> actors; // �z�u�\�ȃA�N�^�[
  std::shared_ptr<Actor> cursor; // �}�b�v����J�[�\��
  glm::vec3 cameraOffset = glm::vec3(0, 30, 30); // �J�����̈ʒu

  // �}�b�v���샂�[�h
  enum class Mode {
    select, // �I�����[�h
    set,    // �z�u���[�h
    remove, // �폜���[�h
  };
  Mode mode = Mode::select;
};

#endif // MAPEDITOR_H_INCLUDED
