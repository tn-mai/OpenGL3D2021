/**
* @file AStar.h
*/
#ifndef ASTAR_H_INCLUDED
#define ASTAR_H_INCLUDED
#include <glm/vec3.hpp>
#include <vector>

/**
* A*経路探索
*/
namespace AStar {

struct Node;

/**
* グラフのエッジ(辺)を表す型
*/
struct Edge
{
  Node* node = nullptr; // 隣接先ノード
  float cost = 1.0f; // エッジ通過に必要なコスト
};

/**
* グラフのノード(頂点)を表す型
*
* 有効なノードを作成するにはposition, neighborsを設定する.
*/
struct Node
{
  glm::vec3 position; // ノードの座標
  std::vector<Edge> neighbors; // 隣接ノード配列

  float f; // このノードを通る場合のスタートからゴールまでの予想距離
  float g; // スタートからこのノードまでの予想距離
  Node* parent; // 親ノード

  // ノードの状態
  enum class Status {
    open,  // 調査候補
    close, // 調査済み
  };
  Status status = Status::open;
};

// 経路を表す型
using Route = std::vector<const Node*>;

/**
* グラフを表す型
*/
struct Graph
{
  Route SearchRoute(Node* startNode, Node* goalNode);
  void InitFromGridMap(int width, int height, const int* gridMap);
  Node* GetNode(const glm::vec3& pos);

  std::vector<Node> nodes; // ノード配列
  int width = 0;  // 格子状マップの幅
  int height = 0; // 格子状マップの高さ
  float averageCost = 1.0f; // エッジの平均コスト
};

void Test();

} // namespace AStar

#endif // ASTAR_H_INCLUDED
