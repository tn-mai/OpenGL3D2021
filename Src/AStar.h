/**
* @file AStar.h
*/
#ifndef ASTAR_H_INCLUDED
#define ASTAR_H_INCLUDED
#include <glm/vec3.hpp>
#include <vector>

/**
* A*�o�H�T��
*/
namespace AStar {

struct Node;

/**
* �O���t�̃G�b�W(��)��\���^
*/
struct Edge
{
  Node* node = nullptr; // �אڐ�m�[�h
  float cost = 1.0f; // �G�b�W�ʉ߂ɕK�v�ȃR�X�g
};

/**
* �O���t�̃m�[�h(���_)��\���^
*
* �L���ȃm�[�h���쐬����ɂ�position, neighbors��ݒ肷��.
*/
struct Node
{
  glm::vec3 position; // �m�[�h�̍��W
  std::vector<Edge> neighbors; // �אڃm�[�h�z��

  float f; // ���̃m�[�h��ʂ�ꍇ�̃X�^�[�g����S�[���܂ł̗\�z����
  float g; // �X�^�[�g���炱�̃m�[�h�܂ł̗\�z����
  Node* parent; // �e�m�[�h

  // �m�[�h�̏��
  enum class Status {
    open,  // �������
    close, // �����ς�
  };
  Status status = Status::open;
};

// �o�H��\���^
using Route = std::vector<const Node*>;

/**
* �O���t��\���^
*/
struct Graph
{
  Route SearchRoute(Node* startNode, Node* goalNode);
  void InitFromGridMap(int width, int height, const int* gridMap);
  Node* GetNode(const glm::vec3& pos);

  std::vector<Node> nodes; // �m�[�h�z��
  int width = 0;  // �i�q��}�b�v�̕�
  int height = 0; // �i�q��}�b�v�̍���
  float averageCost = 1.0f; // �G�b�W�̕��σR�X�g
};

void Test();

} // namespace AStar

#endif // ASTAR_H_INCLUDED
