/**
* @file MapEditor.h
*/
#ifndef MAPEDITOR_H_INCLUDED
#define MAPEDITOR_H_INCLUDED
#include "Actor.h"
#include <vector>

/**
* マップエディタ
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
  glm::ivec2 mapSize = glm::ivec2(21, 21); // マップの広さ
  std::vector<std::shared_ptr<Actor>> map; // アクター配置マップ
  std::vector<std::shared_ptr<Actor>> actors; // 配置可能なアクター
  std::shared_ptr<Actor> cursor; // マップ操作カーソル
  glm::vec3 cameraOffset = glm::vec3(0, 30, 30); // カメラの位置

  // マップ操作モード
  enum class Mode {
    select, // 選択モード
    set,    // 配置モード
    remove, // 削除モード
  };
  Mode mode = Mode::select;
};

#endif // MAPEDITOR_H_INCLUDED
