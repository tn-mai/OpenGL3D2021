/**
* @file GameMap.cpp
*/
#include "GameMap.h"
#include <queue>
#include <iostream>
#include <iomanip>

/**
* コンストラクタ
*/
GameMap::GameMap(int width, int height,
  float offsetX, float offsetZ, float tileSize, const int* data) :
  width(width),
  height(height),
  offsetX(offsetX),
  offsetZ(offsetZ),
  tileSize(tileSize),
  objectMapData()
{
  objectMapData.resize(width * height);
  for (int i = 0; i < width * height; ++i) {
    objectMapData[i] = data[i];
  }
}

/**
*
*/
int GameMap::GetObjectNo(float x, float z) const
{
  x -= offsetX - tileSize * 0.5f;
  x /= tileSize;
  const int ix = static_cast<int>(std::floor(x));
  if (ix < 0 || ix >= width) {
    return -1;
  }

  z -= offsetZ - tileSize * 0.5f;
  z /= tileSize;
  const int iz = static_cast<int>(std::floor(z));
  if (iz < 0 || iz >= height) {
    return -1;
  }

  return objectMapData[static_cast<size_t>(ix + iz * width)];
}

struct Node
{
  enum class Status { unlisted, close, open };

  glm::vec2 position;
  float f = 0;
  float h = 0;
  Status status = Status::unlisted;
  Node* parent = nullptr; // 親ノード
  std::vector<Node*> neighbors; // 隣接ノード
};

namespace std {
  template <> struct less<Node*> {
    bool operator()(const Node* lhs, const Node* rhs) const { return lhs->f > rhs->f; }
  };
} // namespace std

/**
* 2点間のマンハッタン距離を計算する
*/
float ManhattanDistance(const glm::vec2& a, const glm::vec2& b)
{
  return std::abs(b.x - a.x) + std::abs(b.y - a.y);
}

/**
* (デバッグ用)A*計算用ノードの評価値を出力する
*/
void PrintNodeMap(
  const Node* startNode, const Node* goalNode,
  int width, int height,
  const std::vector<Node>& nodeMap,
  const std::vector<int>& objectMapData, bool hasRoute = false)
{
  std::vector<const Node*> route;
  if (hasRoute) {
    route.reserve(128);
    const Node* p = goalNode;
    do {
      route.push_back(p);
      p = p->parent;
    } while (p && p != startNode);
  }

  std::cout << "   ";
  for (int i = 0; i < width; ++i) {
    std::cout << '-' << std::setw(2) << i;
  }
  std::cout << "\n";
  for (int z = 0; z < height; ++z) {
    std::cout << std::setw(2) << z << '|';
    for (int x = 0; x < width; ++x) {
      const int current = x + z * width;
      const Node* node = &nodeMap[current];
      const bool isRoute = std::find(route.begin(), route.end(), node) != route.end();
      if (isRoute) {
        std::cout << "*";
      } else {
        std::cout << " ";
      }
      if (node == startNode) {
        std::cout << "St";
      } else if (node == goalNode) {
        std::cout << "Go";
      } else {
        if (objectMapData[current] != 0) {
          std::cout << "--";
        } else {
          std::cout << std::setw(2) << node->f;
        }
      }
    }
    std::cout << "\n";
  }
}

/**
*
*/
std::vector<glm::vec2> GameMap::FindRoute(const glm::vec2& start, const glm::vec2& goal) const
{
  // マップに対応するノード配列を作成する
  std::vector<Node> nodeMap(width * height);
  for (int z = 0; z < height; ++z) {
    for (int x = 0; x < width; ++x) {
      Node& node = nodeMap[x + z * width];
      node.position = glm::vec2(x, z);
      const float g = ManhattanDistance(node.position, start);
      node.h = ManhattanDistance(node.position, goal);
      node.f = g + node.h;

      // 隣接ノードを設定
      static const glm::vec2 neighborOffset[] = { { 1, 0}, {0, -1}, {-1, 0}, {0, 1} };
      for (int i = 0; i < std::size(neighborOffset); ++i) {

        // マップ範囲外なら追加しない
        const glm::ivec2 pos = node.position + neighborOffset[i];
        if (pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height) {
          continue;
        }

        // 進入不可なら何もしない
        const int index = pos.x + pos.y * width;
        if (objectMapData[index] != 0) {
          continue;
        }
        node.neighbors.push_back(&nodeMap[index]);
      }
    }
  }

  Node* startNode = &nodeMap[static_cast<size_t>(start.x + start.y * width)];
  Node* goalNode = &nodeMap[static_cast<size_t>(goal.x + goal.y * width)];

  std::cout << "\n[estimated node cost]\n";
  PrintNodeMap(startNode, goalNode, width, height, nodeMap, objectMapData, false);

  std::priority_queue<Node*> openList;

  openList.push(startNode);
  while (!openList.empty()) {
    Node* n = openList.top();
    openList.pop();

    // ゴールに到達したら探索終了
    if (n == goalNode) {
      break;
    }

    // 「探索済み」ノードなら何もしない
    if (n->status == Node::Status::close) {
      continue;
    }

    // ノードを「探索済み」にする
    n->status = Node::Status::close;

    // 隣接ノードをオープンリストに追加
    for (Node* m : n->neighbors) {
      const float f = (n->f - n->h) + 1.0f + m->h;
      // 現在のルートのほうが短い場合は情報を上書きする
      if (m->status == Node::Status::unlisted|| f < m->f) {
        m->f = f;
        m->parent = n;
        m->status = Node::Status::open;
      }
      openList.push(m);
    }
  }

  std::cout << "[actual node cost]\n";
  PrintNodeMap(startNode, goalNode, width, height, nodeMap, objectMapData, true);

  // オープンリストが空の場合は到達可能なルートが存在しない
  if (openList.empty()) {
    return {};
  }

  // 最短経路を作成
  std::vector<glm::vec2> route;
  route.reserve(static_cast<size_t>(ManhattanDistance(start, goal)));
  const Node* p = goalNode;
  do {
    route.push_back(p->position);
    p = p->parent;
  } while (p && p != startNode);
  return route;
}

/**
*
*/
std::vector<int> GameMap::Get3x3(float x, float z) const
{
  std::vector<int> result(3 * 3, 0);

  return result;
}

