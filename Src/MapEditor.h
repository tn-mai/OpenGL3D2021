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
  // 動作の種類
  enum class SystemType {
    editor, // マップエディタ用のロード処理を行う
    game,   // ゲーム用のロード処理を行う
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

  const SystemType systemType;             // 動作タイプ
  glm::vec2 gridSize = glm::vec2(4.0f);    // マス目のサイズ(m)
  glm::ivec2 mapSize = glm::ivec2(21, 21); // マップの広さ
  std::vector<std::shared_ptr<Actor>> map; // アクター配置マップ(エディタ用)
  std::vector<int> gameMap;                // アクター配置マップ(ゲーム用)
  std::vector<std::shared_ptr<Actor>> actors; // 配置可能なアクター
  std::shared_ptr<Actor> cursor; // マップ操作カーソル
  glm::vec3 cameraOffset = glm::vec3(0, 30, 30); // カメラの位置

  std::vector<std::shared_ptr<Texture>> groundTiles; // 地面用テクスチャ
  std::vector<uint32_t> groundMap;
  uint8_t currentTileNo = 0;

  // マップ操作モード
  enum class Mode {
    select, // 選択モード
    set,    // 配置モード
    remove, // 削除モード
    groundPaint, // 地面ペイントモード
  };
  Mode mode = Mode::select;
};

#endif // MAPEDITOR_H_INCLUDED
