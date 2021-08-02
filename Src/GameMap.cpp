/**
* @file GameMap.cpp
*/
#include "GameMap.h"
#include <queue>
#include <iostream>
#include <iomanip>

/**
* �R���X�g���N�^
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

  int id = 0;
  int f = 0;
  int h = 0;
  Status status = Status::unlisted;
  Node* parent = nullptr;
  int neighborCount = 0;
  int neighbors[4] = { 0, 0, 0, 0 };
};

struct DistanceAndId {
  DistanceAndId(const Node& node) : f(node.f), id(node.id) {}

  int f;
  int id;
};
bool operator>(const DistanceAndId& lhs, const DistanceAndId& rhs) { return lhs.f > rhs.f; }

/**
* 2�_�Ԃ̃}���n�b�^���������v�Z����
*/
int ManhattanDistance(const glm::ivec2& a, const glm::ivec2& b)
{
  return std::abs(b.x - a.x) + std::abs(b.y - a.y);
}

/**
* (�f�o�b�O�p)A*�v�Z�p�m�[�h�̕]���l���o�͂���
*/
void PrintNodeMap(
  int start, int goal,
  int width, int height,
  const std::vector<Node>& nodeMap,
  const std::vector<int>& objectMapData, bool hasRoute = false)
{
  std::vector<int> route;
  if (hasRoute) {
    route.reserve(128);
    const Node* p = &nodeMap[goal];
    do {
      route.push_back(p->id);
      p = p->parent;
    } while (p && p->id != start);
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
      const Node& node = nodeMap[current];
      const bool isRoute = std::find(route.begin(), route.end(), current) != route.end();
      if (isRoute) {
        std::cout << "*";
      } else {
        std::cout << " ";
      }
      if (node.id == start) {
        std::cout << "St";
      } else if (node.id == goal) {
        std::cout << "Go";
      } else {
        if (objectMapData[current] != 0) {
          std::cout << "--";
        } else {
          std::cout << std::setw(2) << node.f;
        }
      }
    }
    std::cout << "\n";
  }
}

/**
*
*/
std::vector<glm::ivec2> GameMap::FindRoute(const glm::ivec2& start, const glm::ivec2& goal) const
{
  // �}�b�v�ɑΉ�����m�[�h�z����쐬����
  std::vector<Node> nodeMap(width * height);
  for (int z = 0; z < height; ++z) {
    for (int x = 0; x < width; ++x) {
      Node& node = nodeMap[x + z * width];
      node.id = x + z * width;
      const glm::ivec2 position(x, z);
      const int g = ManhattanDistance(position, start);
      node.h = ManhattanDistance(position, goal);
      node.f = g + node.h;

      // �אڃm�[�h��ݒ�
      static const glm::ivec2 neighborOffset[] = { { 1, 0}, {0, -1}, {-1, 0}, {0, 1} };
      for (int i = 0; i < std::size(neighborOffset); ++i) {

        // �}�b�v�͈͊O�Ȃ�ǉ����Ȃ�
        const glm::ivec2 pos = position + neighborOffset[i];
        if (pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height) {
          continue;
        }

        // �i���s�Ȃ牽�����Ȃ�
        const int id = pos.x + pos.y * width;
        if (objectMapData[id] != 0) {
          continue;
        }
        node.neighbors[node.neighborCount] = id;
        ++node.neighborCount;
      }
    }
  }

  const int idStart = start.x + start.y * width;
  const int idGoal = goal.x + goal.y * width;

  std::cout << "\n[estimated node cost]\n";
  PrintNodeMap(idStart, idGoal, width, height, nodeMap, objectMapData, false);

  std::vector<DistanceAndId> tmp;
  tmp.reserve(width * height);
  std::priority_queue<DistanceAndId, std::vector<DistanceAndId>, std::greater<DistanceAndId>> openList(std::greater<DistanceAndId>(), tmp);

  openList.push(nodeMap[start.x + start.y * width]);
  while (!openList.empty()) {
    const int idCurrent = openList.top().id;
    openList.pop();

    // �S�[���ɓ��B������T���I��
    Node& n = nodeMap[idCurrent];
    if (n.id == idGoal) {
      break;
    }

    // �u�T���ς݁v�m�[�h�Ȃ牽�����Ȃ�
    if (n.status == Node::Status::close) {
      continue;
    }

    // �m�[�h���u�T���ς݁v�ɂ���
    n.status = Node::Status::close;

    // ����4�����̃m�[�h���I�[�v�����X�g�ɒǉ�
    const int nx = n.id % width;
    const int ny = n.id / width;
    for (int i = 0; i < n.neighborCount; ++i) {
      const int id = n.neighbors[i];
      Node& m = nodeMap[id];
      const int f = (n.f - n.h) + 1 + m.h;
      // ���݂̃��[�g�̂ق����Z���ꍇ�͏����㏑������
      if (m.status == Node::Status::unlisted|| f < m.f) {
        m.f = f;
        m.parent = &n;
        m.status = Node::Status::open;
      }
      openList.push(m);
    }
  }

  std::cout << "[actual node cost]\n";
  PrintNodeMap(idStart, idGoal, width, height, nodeMap, objectMapData, true);

  // �I�[�v�����X�g����̏ꍇ�͓��B�\�ȃ��[�g�����݂��Ȃ�
  if (openList.empty()) {
    return {};
  }

  std::vector<glm::ivec2> route;
  route.reserve(ManhattanDistance(start, goal));
  const Node* p = &nodeMap[goal.x + goal.y * width];
  do {
    route.push_back(glm::ivec2(p->id % width, p->id / width));
    p = p->parent;
  } while (p && p->id != idStart);
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

