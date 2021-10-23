/**
* @file GameMap.h
*/
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

/**
* ゲーム内の配置データを管理するクラス.
*/
class GameMap
{
public:
  GameMap(int width, int height,
    float offsetX, float offsetZ, float tileSize, const int* data);
  ~GameMap() = default;

  std::vector<glm::vec2> FindRoute(const glm::vec2& start, const glm::vec2& goal) const;
  int GetObjectNo(float x, float z) const;
  std::vector<int> Get3x3(float x, float z) const;

private:
  int width;
  int height;
  float offsetX;
  float offsetZ;
  float tileSize;
  std::vector<int> objectMapData;
};