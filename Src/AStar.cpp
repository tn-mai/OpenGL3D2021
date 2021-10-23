/**
* @file AStar.cpp
*/
#include "AStar.h"
#include <glm/glm.hpp>
#include <queue>
#include <algorithm>
#include <iostream>

namespace AStar {

/**
* 2点間のマンハッタン距離を計算する
*/
float ManhattanDistance(const Node* a, const Node* b)
{
  const glm::vec3 v = glm::abs(b->position - a->position);
  return v.x + v.y + v.z;
}

/**
* Nodeポインタの優先順判定を行うクラス
*/
struct CompareNodePointer
{
  bool operator()(const Node* lhs, const Node* rhs) const
  {
    if (lhs->f == rhs->f) {
      return lhs->g > rhs->g;
    }
    return lhs->f > rhs->f;
  }
};

/**
* 最短経路を探索する
*
* @param graph     探索対象のノードグラフ
* @param startNode 探索開始ノード
* @param goalNode  探索目標ノード
*
* @return startNodeからgoalNodeへの最短経路を返す
*/
Route Graph::SearchRoute(Node* startNode, Node* goalNode)
{
  // 1. ノードの状態を初期化
  for (auto& e : nodes) {
    //e.g = FLT_MAX;
    e.g = FLT_MAX;
    //e.parent = nullptr;
    e.status = Node::Status::open;
  }
  //startNode->f = ManhattanDistance(startNode, goalNode) * averageCost;
  startNode->g = 0;

  // 2. オープンリストを初期化
  std::priority_queue<Node*, std::vector<Node*>, CompareNodePointer> openList;
  openList.push(startNode);

  // 3. オープンリストに含まれるノードを、fが小さい順にチェックする
  while (!openList.empty()) {
    Node* n = openList.top();

    // ゴールに到達したら探索終了
    if (n == goalNode) {
      break;
    }
    openList.pop();

    // 「探索済み」ノードなら何もしない
    if (n->status == Node::Status::close) {
      continue;
    }

    // ノードを「探索済み」にする
    n->status = Node::Status::close;

    // 隣接ノードをオープンリストに追加
    for (const Edge& e : n->neighbors) {
      Node* m = e.node;
      const float g = n->g + e.cost;
      // 現在の経路のほうが早く隣接ノードmに着く場合、mの情報を更新してオープンリストに追加
      if (g < m->g) {
        m->g = g;
        m->f = g + ManhattanDistance(m, goalNode) * averageCost;
        m->parent = n;
        m->status = Node::Status::open;
        openList.push(m);
      }
    }
  }

  // 4. オープンリストが空の場合は到達可能な経路が存在しない
  if (openList.empty()) {
    return Route(); // 空の経路を返す
  }

  // 5. 親ノードをたどってゴールからスタートへの最短経路を作成
  Route route;
  const Node* p = goalNode;
  do {
    route.push_back(p);
    p = p->parent;
  } while (p && p != startNode);
  route.push_back(startNode);
  return route;
}

/**
* 2Dの格子状マップからグラフを作成する
*/
void Graph::InitFromGridMap(int width, int height, const int* gridMap)
{
  this->width = width;
  this->height = height;

  // ノード配列を初期化
  nodes.clear();
  nodes.resize(width * height);

  // 隣接ノードの位置を表す配列
  const glm::ivec2 offset[] = { {1, 0}, {0,-1},{-1, 0}, {0, 1} };

  // 格子状マップをノードに変換する
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      // 座標を設定
      Node& node = nodes[x + y * width];
      node.position = glm::vec3(x, 0, y);

      // 隣接ノードへのエッジを設定
      for (auto& e : offset) {
        // 座標がマップ範囲外の場合は移動不可とする
        const glm::ivec2 pos(x + e.x, y + e.y);
        if (pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height) {
          continue;
        }

        // マスの値が0以外の場合は移動不可とする(壁など)
        const int i = pos.x + pos.y * width;
        if (gridMap[i]!= 0) {
          continue;
        }

        // 0なら移動可能なマスなのでエッジを作成
        node.neighbors.push_back(Edge{ &nodes[i], 1.0f });
      }
    } // for x
  } // for y
}

/**
* 座標に対応するノードを取得する
*/
Node* Graph::GetNode(const glm::vec3& pos)
{
  if (pos.x < 0 || pos.x >= width || pos.z < 0 || pos.z >= height) {
    return nullptr;
  }
  return &nodes[static_cast<size_t>(pos.x + pos.z * width)];
}

/**
* A*アルゴリズムの動作テスト
*/
void Test()
{
  // マップデータ
  const int mapSizeX = 5;
  const int mapSizeY = 5;
  const int objectMapData[mapSizeY][mapSizeX] = {
    { 0, 0, 0, 0, 0 },
    { 0, 0, 1, 1, 0 },
    { 0, 0, 0, 1, 0 },
    { 0, 0, 0, 1, 0 },
    { 0, 0, 0, 0, 0 },
  };
  const glm::vec3 start(2, 0, 2); // スタート地点
  const glm::vec3 goal(4, 0, 0);  // ゴール地点

  // マップデータからグラフを作成
  Graph graph;
  graph.InitFromGridMap(mapSizeX, mapSizeY, objectMapData[0]);

  // startからgoalまでの最短経路を探索
  Node* startNode = graph.GetNode(start);
  Node* goalNode = graph.GetNode(goal);
  Route route = graph.SearchRoute(startNode, goalNode);
  if (route.empty()) {
    std::cout << "[情報]" << __func__ << ": 経路が見つかりません\n";
    return;
  }

  // 最短経路を出力
  std::cout << "[情報]" << __func__ << ": 最短経路が見つかりました\n";
  for (int y = 0; y < mapSizeY; ++y) {
    for (int x = 0; x < mapSizeX; ++x) {
      if (start.x == x && start.z == y) {
        std::cout << "S ";
        continue;
      } else if (goal.x == x && goal.z == y) {
        std::cout << "G ";
        continue;
      }
      auto i = std::find_if(route.begin(), route.end(), [x, y](const Node* n) {
        return n->position.x == x && n->position.z == y; });
      if (i == route.end()) {
        std::cout << objectMapData[y][x] << " ";
      } else {
        std::cout << "* ";
      }
    }
    std::cout << "\n";
  }
  std::cout << "\n";
}

} // namespace AStar
