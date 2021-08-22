/**
* @file Main.cpp
*/
#include <glad/glad.h>
#include "GLContext.h"
#include "GameEngine.h"
#include "Primitive.h"
#include "ProgramPipeline.h"
#include "Texture.h"
#include "Sampler.h"
#include "Actor.h"
#include "GameEngine.h"
#include "GameManager.h"
#include "Actor/PlayerActor.h"
#include "Actor/T34TankActor.h"
#include "Actor/RandomMovingEnemyActor.h"
#include "Actor/ElevatorActor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <iostream>
#include <memory>
#include <algorithm>
#include <unordered_map>
#pragma comment(lib, "opengl32.lib")

/// ���W�f�[�^:�l�p�`
const glm::vec3 posRectangle[] = {
  {-0.2f, -0.5f, 0.1f},
  { 0.3f, -0.5f, 0.1f},
  { 0.3f,  0.3f, 0.1f},
  { 0.3f,  0.3f, 0.1f},
  {-0.2f,  0.3f, 0.1f},
  {-0.2f, -0.5f, 0.1f},
};

/// ���W�f�[�^:�O�p�`
const glm::vec3 posTriangle[] = {
  {(-0.33f / 2.0f) * 10.0f, (0.5f / 2.0f) * 10.0f, 0.6f },
  {(0.33f / 2.0f) * 10.0f, (0.5f / 2.0f) * 10.0f, 0.6f },
  {(0.00f / 2.0f) * 10.0f, (-0.5f / 2.0f) * 10.0f, 0.6f },
  {(-0.33f / 2.0f - 0.165f) * 10.0f, (0.5f / 2.0f + 0.5f) * 10.0f, 0.6f },
  {(0.33f / 2.0f - 0.165f) * 10.0f, (0.5f / 2.0f + 0.5f) * 10.0f, 0.6f },
  {(0.00f / 2.0f - 0.165f) * 10.0f, (-0.5f / 2.0f + 0.5f) * 10.0f, 0.6f },
  {(-0.33f / 2.0f + 0.165f) * 10.0f, (0.5f / 2.0f + 0.5f) * 10.0f, 0.6f },
  {(0.33f / 2.0f + 0.165f) * 10.0f, (0.5f / 2.0f + 0.5f) * 10.0f, 0.6f },
  {(0.00f / 2.0f + 0.165f) * 10.0f, (-0.5f / 2.0f + 0.5f) * 10.0f, 0.6f },
};

/// ���W�f�[�^:������
const glm::vec3 posCube[] = {
  { 0, 0, 0}, { 1, 0, 0}, { 1, 0, 1}, { 0, 0, 1},
  { 0, 1, 0}, { 1, 1, 0}, { 1, 1, 1}, { 0, 1, 1},
};

/// ���W�f�[�^:
const glm::vec3 posTree[] = {
  // ��(�t)
  { 0.0f, 3.0f, 0.0f},
  { 0.0f, 1.0f,-1.0f},
  {-1.0f, 1.0f, 0.0f},
  { 0.0f, 1.0f, 1.0f},
  { 1.0f, 1.0f, 0.0f},
  { 0.0f, 1.0f,-1.0f},

  // ��(��)
  { 0.0f, 2.0f, 0.0f},
  { 0.0f, 0.0f,-0.5f},
  {-0.5f, 0.0f, 0.0f},
  { 0.0f, 0.0f, 0.5f},
  { 0.5f, 0.0f, 0.0f},
  { 0.0f, 0.0f,-0.5f},
};

/// ���W�f�[�^:����
const glm::vec3 posWarehouse[] = {
  {-2, 0,-2}, {-2, 0, 2}, { 2, 0, 2}, { 2, 0,-2}, {-2, 0,-2},
  {-2, 2,-2}, {-2, 2, 2}, { 2, 2, 2}, { 2, 2,-2}, {-2, 2,-2},
  { 2, 2, 2}, { 2, 2,-2},
};

/// �F�f�[�^:�n��
const glm::vec4 colGround[] = {
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
  {1.0f, 1.0f, 1.0f, 1.0f},
};

/// �F�f�[�^:�l�p�`
const glm::vec4 colRectangle[] = {
  {1.0f, 0.0f, 0.0f, 1.0f},
  {1.0f, 1.0f, 0.0f, 1.0f},
  {1.0f, 0.0f, 0.0f, 1.0f},
  {0.0f, 0.0f, 1.0f, 1.0f},
  {0.0f, 1.0f, 1.0f, 1.0f},
  {0.0f, 0.0f, 1.0f, 1.0f},
};

/// �F�f�[�^:�O�p�`
const glm::vec4 colTriangle[] = {
  { 0.0f, 1.0f, 1.0f, 1.0f }, // ���F
  { 1.0f, 1.0f, 0.0f, 1.0f }, // ���F
  { 1.0f, 0.0f, 1.0f, 1.0f }, // ���F
  { 0.0f, 1.0f, 1.0f, 1.0f }, // ���F
  { 1.0f, 1.0f, 0.0f, 1.0f }, // ���F
  { 1.0f, 0.0f, 1.0f, 1.0f }, // ���F
  { 0.0f, 1.0f, 1.0f, 1.0f }, // ���F
  { 1.0f, 1.0f, 0.0f, 1.0f }, // ���F
  { 1.0f, 0.0f, 1.0f, 1.0f }, // ���F
};

/// �F�f�[�^:������
const glm::vec4 colCube[] = {
  { 1, 1, 1, 1}, { 1, 1, 1, 1}, { 1, 1, 1, 1}, { 1, 1, 1, 1},
  { 1, 1, 1, 1}, { 1, 1, 1, 1}, { 1, 1, 1, 1}, { 1, 1, 1, 1},
};

/// �F�f�[�^:��
const glm::vec4 colTree[] = {
  // ��(�t)
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},

  // ��(��)
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
  { 1, 1, 1, 1},
};

/// �F�f�[�^:����
const glm::vec4 colWarehouse[] = {
  { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 },
  { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 }, { 1, 1, 1, 1 },
  { 1, 1, 1, 1 }, { 1, 1, 1, 1 },
};

/// �e�N�X�`�����W�f�[�^:�l�p�`
const glm::vec2 tcRectangle[] = {
  { 0.0f, 0.0f}, { 1.0f, 0.0f}, { 1.0f, 1.0f},
  { 1.0f, 1.0f}, { 0.0f, 1.0f}, { 0.0f, 0.0f},
};

/// �e�N�X�`�����W�f�[�^:�O�p�`
const glm::vec2 tcTriangle[] = {
  { 0.0f, 0.0f}, { 1.0f, 0.0f}, { 0.5f, 1.0f},
  { 0.0f, 0.0f}, { 1.0f, 0.0f}, { 0.5f, 1.0f},
  { 0.0f, 0.0f}, { 1.0f, 0.0f}, { 0.5f, 1.0f},
};

/// �e�N�X�`�����W�f�[�^:������
const glm::vec2 tcCube[] = {
  { 0.0f, 0.0f}, { 0.0f, 0.0f}, { 0.0f, 0.0f}, { 0.0f, 0.0f},
  { 0.0f, 0.0f}, { 0.0f, 0.0f}, { 0.0f, 0.0f}, { 0.0f, 0.0f},
};

/// �e�N�X�`�����W�f�[�^:��
const glm::vec2 tcTree[] = {
  // ��(�t)
  { 0.5f, 1.0f},
  { 0.0f, 0.5f},
  { 0.25f, 0.5f},
  { 0.5f, 0.5f},
  { 0.75f, 0.5f},
  { 1.0f, 0.5f},

  // ��(��)
  { 0.5f, 0.5f},
  { 0.0f, 0.0f},
  { 0.25f, 0.0f},
  { 0.5f, 0.0f},
  { 0.75f, 0.0f},
  { 1.0f, 0.0f},
};

/// �e�N�X�`�����W�f�[�^:����
const glm::vec2 tcWarehouse[] = {
  { 0.0f, 0.0f}, { 0.25f, 0.0f}, { 0.5f, 0.0f}, { 0.75f, 0.0f}, { 1.0f, 0.0f},
  { 0.0f, 0.5f}, { 0.25f, 0.5f}, { 0.5f, 0.5f}, { 0.75f, 0.5f}, { 1.0f, 0.5f},
  { 0.25f, 1.0f}, { 0.0f, 1.0f},
};

/// �C���f�b�N�X�f�[�^:�l�p�`
const GLushort indexRectangle[] = {
  0, 1, 2, 3, 4, 5,
};

/// �C���f�b�N�X�f�[�^:�O�p�`
const GLushort indexTriangle[] = {
  2, 1, 0, 5, 4, 3, 8, 7, 6,
};

/// �C���f�b�N�X�f�[�^:������
const GLushort indexCube[] = {
 0, 1, 2, 2, 3, 0, 4, 5, 1, 1, 0, 4,
 5, 6, 2, 2, 1, 5, 6, 7, 3, 3, 2, 6,
 7, 4, 0, 0, 3, 7, 7, 6, 5, 5, 4, 7,
};

/// �C���f�b�N�X�f�[�^:��
const GLushort indexTree[] = {
 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 1, 4, 3, 3, 2, 1, // �t
 6, 7, 8, 6, 8, 9, 6, 9,10, 6,10,11, 7,10, 9, 9, 8, 7, // ��
};

/// �C���f�b�N�X�f�[�^:����
const GLushort indexWarehouse[] = {
 0, 1, 6, 6, 5, 0,
 1, 2, 7, 7, 6, 1,
 2, 3, 8, 8, 7, 2,
 3, 4, 9, 9, 8, 3,
 5, 6,10,10,11, 5,
};

// �摜�f�[�^.
const int imageGroundWidth = 8; // �摜�̕�.
const int imageGroundHeight = 8; // �摜�̍���.
const GLuint X = 0xff'18'18'18; // ��.
const GLuint W = 0xff'ff'ff'ff; // ��.
const GLuint R = 0xff'10'10'e0; // ��.
const GLuint B = 0xff'e0'10'10; // ��.
const GLuint imageGround[imageGroundWidth * imageGroundHeight] = {
  X, B, B, B, X, W, W, W,
  X, B, B, B, X, W, W, W,
  X, B, B, B, X, W, W, W,
  X, X, X, X, X, X, X, X,
  W, W, X, R, R, R, X, W,
  W, W, X, R, R, R, X, W,
  W, W, X, R, R, R, X, W,
  X, X, X, X, X, X, X, X,
};

const GLuint imageTriangle[6 * 6] = {
  X, W, X, W, X, W,
  X, W, X, W, X, W,
  X, W, X, W, X, W,
  X, W, X, W, X, W,
  X, W, X, W, X, W,
  X, W, X, W, X, W,
};

const GLuint G = 0xff'10'80'10; // ��.
const GLuint D = 0xff'40'a0'40; // ���F.
const GLuint imageGreen[8 * 8] = {
  G, G, G, G, G, G, G, G,
  G, D, G, G, G, G, G, G,
  G, G, G, G, G, G, G, G,
  G, G, G, G, G, D, G, G,
  G, G, G, G, G, G, G, G,
  G, G, G, G, G, G, G, G,
  G, G, G, G, G, G, G, G,
  G, G, G, G, G, G, G, G,
};

const GLuint P = 0xff'60'60'60;
const GLuint imageRoad[8 * 8] = {
  P, P, P, P, P, P, P, P,
  P, W, P, P, P, P, P, P,
  P, P, P, P, P, P, W, P,
  P, P, P, P, P, P, P, P,
  P, P, P, P, P, P, P, P,
  P, P, P, P, P, P, P, P,
  P, P, P, W, P, P, P, P,
  P, P, P, P, P, P, P, P,
};

/// �}�b�v�f�[�^.
int mapData[16][16] = {
  { 2,2,2,2,2,2,2,2,0,0,1,1,0,0,2,2},
  { 2,2,2,2,2,2,2,2,0,0,1,1,0,0,2,2},
  { 2,2,0,0,0,0,2,2,2,2,2,2,0,0,2,2},
  { 2,2,0,0,0,0,2,2,2,2,2,2,0,0,2,2},
  { 2,2,2,2,2,2,0,0,0,0,0,0,2,2,2,2},
  { 2,2,2,2,2,2,0,0,0,0,0,0,2,2,2,2},
  { 2,2,0,0,2,2,2,2,2,2,0,0,2,2,0,0},
  { 2,2,0,0,2,2,2,2,2,2,0,0,2,2,0,0},
  { 2,2,2,2,0,0,0,0,2,2,2,2,2,2,2,2},
  { 2,2,2,2,0,0,0,0,2,2,2,2,2,2,2,2},
  { 0,0,2,2,2,2,0,0,0,0,0,0,0,0,2,2},
  { 0,0,2,2,2,2,0,0,0,0,0,0,0,0,2,2},
  { 2,2,0,0,0,0,1,1,1,1,0,0,2,2,2,2},
  { 2,2,0,0,0,0,1,1,1,1,0,0,2,2,2,2},
  { 2,2,2,2,2,2,1,1,1,1,2,2,2,2,0,0},
  { 2,2,2,2,2,2,1,1,1,1,2,2,2,2,0,0},
};

/// �I�u�W�F�N�g�}�b�v�f�[�^.
int objectMapData[16][16] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 3, 1, 0, 0, 1, 3, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 4, 1, 0, 0},
  { 0, 0, 4, 1, 2, 1, 0, 0, 0, 0, 0, 0, 1, 3, 0, 0},
  { 0, 0, 1, 3, 1, 2, 0, 0, 0, 0, 0, 0, 4, 1, 0, 0},
  { 0, 0, 0, 0, 0, 0, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 2, 3, 1, 3, 1, 4, 0, 0, 0, 0},
  { 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 4, 1, 0, 0, 1, 4},
  { 0, 0, 1, 3, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1},
  { 0, 0, 0, 0, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 1, 4, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  { 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0},
  { 4, 1, 0, 0, 0, 0, 3, 1, 3, 1, 1, 3, 1, 4, 0, 0},
  { 0, 0, 3, 1, 1, 4, 0, 0, 0, 0, 4, 1, 0, 0, 0, 0},
  { 0, 0, 1, 1, 3, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3},
};

//// �A�N�^�[�̔z��.
//std::vector<std::shared_ptr<Actor>> actors;

/**
* �G���g���[�|�C���g.
*/
int main()
{
  GameEngine::Initialize();
  GameEngine& engine = GameEngine::Get();

  // �A�N�^�[�̔z��.
  std::vector<std::shared_ptr<Actor>>& actors = engine.GetActors();

  // VAO���쐬����.
  PrimitiveBuffer& primitiveBuffer = engine.GetPrimitiveBuffer();

  // �`��f�[�^��ǉ�����.
  primitiveBuffer.AddFromObjFile("Res/Ground.obj");
  primitiveBuffer.AddFromObjFile("Res/Rectangle.obj");
  primitiveBuffer.AddFromObjFile("Res/Triangle.obj");
  primitiveBuffer.AddFromObjFile("Res/Cube.obj");
  primitiveBuffer.AddFromObjFile("Res/Tree.obj");
  primitiveBuffer.AddFromObjFile("Res/Warehouse.obj");
  primitiveBuffer.AddFromObjFile("Res/tank/Tiger_I.obj");
  primitiveBuffer.AddFromObjFile("Res/tank/T34.obj");
  primitiveBuffer.AddFromObjFile("Res/house/HouseRender.obj");
  primitiveBuffer.AddFromObjFile("Res/Bullet.obj");
  primitiveBuffer.AddFromObjFile("Res/house/broken-house.obj");
  primitiveBuffer.AddFromObjFile("Res/Plane.obj");

  // �p�C�v���C���E�I�u�W�F�N�g���쐬����.
  ProgramPipeline pipeline("Res/FragmentLighting.vert", "Res/FragmentLighting.frag");
  if (!pipeline.IsValid()) {
    return 1;
  }

  // uniform�ϐ��̈ʒu.
  const GLint locMatTRS = 0;
  const GLint locMatModel = 1;

  // ���W�ϊ��s��̉�]�p�x.
  float degree = 0;

  // �e�N�X�`�����쐬.
  std::shared_ptr<Texture> texGround(new Texture("Res/RoadTiles.tga"));
  std::shared_ptr<Texture> texTriangle(new Texture("Res/Triangle.tga"));
  std::shared_ptr<Texture> texGreen(new Texture("Res/Green.tga"));
  std::shared_ptr<Texture> texRoad(new Texture("Res/Road.tga"));
  std::shared_ptr<Texture> texTree(new Texture("Res/Tree.tga"));
  std::shared_ptr<Texture> texWarehouse(new Texture("Res/Building.tga"));
  std::shared_ptr<Texture> texTank(new Texture("Res/tank/PzVl_Tiger_I.tga"));
  std::shared_ptr<Texture> texTankT34(new Texture("Res/tank/T-34.tga"));
  std::shared_ptr<Texture> texBrickHouse(new Texture("Res/house/House38UVTexture.tga"));
  std::shared_ptr<Texture> texBullet(new Texture("Res/Bullet.tga"));
  std::shared_ptr<Texture> texHouse2(new Texture("Res/house/broken-house.tga"));

  // �T���v�����쐬.
  std::shared_ptr<Sampler> sampler(new Sampler(GL_REPEAT));

  // �}�b�v�ɔz�u���镨�̂̕\���f�[�^.
  struct ObjectData {
    const char* name;
    Primitive prim;
    const std::shared_ptr<Texture> tex;
    float scale = 1.0f;
    glm::vec3 ajustment = glm::vec3(0);
    Box collider;
  };

  // ��ʒ[�ɃR���C�_�[��ݒ�
  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(-36, 0, -34), glm::vec3(1, 2, 32), 0.0f, glm::vec3(0) }));
  actors.back()->collider = Box{ glm::vec3(0, 0, 0), glm::vec3(1, 4, 64) };
  actors.back()->isStatic = true;

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(30, 0, -34), glm::vec3(1, 2, 32), 0.0f, glm::vec3(0) }));
  actors.back()->collider = Box{ glm::vec3(0, 0, 0), glm::vec3(1, 4, 64) };
  actors.back()->isStatic = true;

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(-34, 0, -36), glm::vec3(32, 2, 1), 0.0f, glm::vec3(0) }));
  actors.back()->collider = Box{ glm::vec3(0, 0, 0), glm::vec3(64, 4, 1) };
  actors.back()->isStatic = true;

  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Wall", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(-34, 0, 30), glm::vec3(32, 2, 1), 0.0f, glm::vec3(0) }));
  actors.back()->collider = Box{ glm::vec3(0, 0, 0), glm::vec3(64, 4, 1) };
  actors.back()->isStatic = true;

  // �`�悷�镨�̂̃��X�g.
  const Box col1 = { glm::vec3(-1.75f, 0, -1.75f), glm::vec3(1.75f, 2, 1.75f) };
  const ObjectData objectList[] = {
    { "", Primitive(), 0 },    // �Ȃ�
    { "Tree", primitiveBuffer.Get(4), texTree }, // ��
    { "Warehouse", primitiveBuffer.Get(5), texWarehouse, 1, {}, col1 }, // ����
    { "BrickHouse", primitiveBuffer.Get(8), texBrickHouse, 3, glm::vec3(-2.6f, 2.0f, 0.8f),
      Box{ glm::vec3(-3, 0, -2), glm::vec3(3, 3, 2) } }, // ����
    { "House2", primitiveBuffer.Get(10), texHouse2, 1, {},
      Box{ glm::vec3(-2.5f, 0, -3.5f), glm::vec3(2.5f, 3, 3.5f) } }, // ����
  };

  // �؂�A����.
  for (int y = 0; y < 16; ++y) {
    for (int x = 0; x < 16; ++x) {
      const int objectNo = objectMapData[y][x];
      if (objectNo <= 0 || objectNo >= std::size(objectList)) {
        continue;
      }
      const ObjectData p = objectList[objectNo];

      // �l�p�`��4x4m�Ȃ̂ŁAx��y��4�{�����ʒu�ɕ\������.
      const glm::vec3 position(x * 4 - 32, 0, y * 4 - 32);

      actors.push_back(std::shared_ptr<Actor>(new Actor{ p.name, p.prim, p.tex,
        position, glm::vec3(p.scale), 0.0f, p.ajustment }));
      actors.back()->collider = col1;// p.collider;
      actors.back()->isStatic = true;
    }
  }

  // �}�b�v��(-20,-20)-(20,20)�͈̔͂ɕ`��.
  const std::shared_ptr<Texture> mapTexList[] = { texGreen, texGround, texRoad };
  for (int y = 0; y < 16; ++y) {
    for (int x = 0; x < 16; ++x) {
      // �l�p�`��4x4m�Ȃ̂ŁAx��y��4�{�����ʒu�ɕ\������.
      const glm::vec3 position(x * 4 - 32, 0, y * 4 - 32);

      const int textureNo = mapData[y][x];
      actors.push_back(std::shared_ptr<Actor>(new Actor{ "Ground", primitiveBuffer.Get(0), mapTexList[textureNo],
        position, glm::vec3(1), 0.0f, glm::vec3(0) }));
      actors.back()->collider = Box{ glm::vec3(-2, -10, -2), glm::vec3(2, 0, 2) };
      actors.back()->isStatic = true;
    }
  }

  // �G���x�[�^�[
  {
    const glm::vec3 position(4 * 4 - 20, -1, 4 * 4 - 20);
    actors.push_back(std::shared_ptr<Actor>(new ElevatorActor{
      "Elevator", primitiveBuffer.Get(0), mapTexList[0],
      position, glm::vec3(1), 0.0f, glm::vec3(0) }));
    actors.back()->velocity.y = 1;
    //actors.back()->collider = Box{ glm::vec3(-2, -10, -2), glm::vec3(2, 0, 2) };
    actors.back()->isStatic = true;
  }

  // �O�p�`�̃p�����[�^
  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Triangle", primitiveBuffer.Get(2), texTriangle,
    glm::vec3(0, 0, -5), glm::vec3(1), 0.0f, glm::vec3(0) }));
  // �����̂̃p�����[�^
  actors.push_back(std::shared_ptr<Actor>(new Actor{ "Cube", primitiveBuffer.Get(3), texTriangle,
    glm::vec3(0, 0, -4), glm::vec3(1), 0.0f, glm::vec3(0) }));

  std::shared_ptr<GameMap> gamemap(new GameMap(16, 16, -32, -32, 4, &objectMapData[0][0]));
  std::vector<glm::ivec2> route = gamemap->FindRoute(glm::ivec2(7, 15), glm::ivec2(10, 0));

  // �Q�[���}�l�[�W�����쐬
  GameManager::Initialize();
  GameManager& manager = GameManager::Get();

  // ���C�����[�v.
  double loopTime = engine.GetTime();     // 1/60�b�Ԋu�Ń��[�v�������邽�߂̎���
  double diffLoopTime = 0;             // �����̍���
  const float deltaTime = 1.0f / 60.0f;// ���ԊԊu
  glm::vec3 cameraPosition = glm::vec3(0, 20, 20); // �J�����̍��W
  glm::vec3 cameraTarget = glm::vec3(0, 0, 0);     // �J�����̒����_�̍��W
  while (!engine.WindowShouldClose()) {
    // ���ݎ������擾
    const double curLoopTime = engine.GetTime();
    // ���ݎ����ƑO�񎞍��̍����A�����̍����ɉ��Z
    diffLoopTime += curLoopTime - loopTime;
    // ���������ݎ����ɍX�V
    loopTime = curLoopTime;
    // �����̍�����1/60�b�����Ȃ�A���[�v�̐擪�ɖ߂�
    if (diffLoopTime < deltaTime) {
      continue;
    }
    if (diffLoopTime > 20.0 / 60.0) {
      diffLoopTime = deltaTime;
    }

    //
    // �Q�[����Ԃ��X�V����
    //

    for (; diffLoopTime >= deltaTime; diffLoopTime -= deltaTime) {
      engine.UpdateActors(deltaTime);
      manager.Update(deltaTime);
      engine.PostUpdateActors();

      // �A�N�^�[�̏Փ˔�����s��
#if 0
      std::vector<Actor*> dynamicActors;
      std::vector<Actor*> staticActors;
      dynamicActors.reserve(actors.size());
      staticActors.reserve(actors.size());
      for (int i = 0; i < actors.size(); ++i) {
        if (actors[i]->isStatic) {
          staticActors.push_back(&actors[i]);
        } else {
          dynamicActors.push_back(&actors[i]);
        }
      }
#endif

      for (int i = 0; i < actors.size(); ++i) {
        actors[i]->isOnActor = false;
      }

      std::vector<Contact> contacts;
      contacts.reserve(actors.size());
      for (int a = 0; a < actors.size(); ++a) {
        for (int b = a + 1; b < actors.size(); ++b) {

          // �폜�҂��A�N�^�[�͏Փ˂��Ȃ�
          if (actors[a]->isDead) {
            break;
          } else if (actors[b]->isDead) {
            continue;
          }

          Contact contact;
          if (DetectCollision(*actors[a], *actors[b], contact)) {
#if 1
            // �z��̒��ɁA�쐬�����R���^�N�g�\���̂Ǝ��Ă�����̂����邩���ׂ�
            auto itr = std::find_if(contacts.begin(), contacts.end(),
              [&contact](const Contact& c) { return Equal(contact, c); });

            // ���Ă���R���^�N�g�\���̂�������Ȃ���΁A�쐬�����\���̂�z��ɒǉ�����
            if (itr == contacts.end()) {
              contacts.push_back(contact);
            } else {
              // ���Ă���\���̂����������ꍇ�A�Z�������������ق����c��
              if (contact.penLength > itr->penLength) {
                *itr = contact;
              }
            }
#else
            contacts.push_back(contact);
#endif
          }
        }
      }

      // �d�Ȃ����������
      for (int i = 0; i < contacts.size(); ++i) {
        Contact& c = contacts[i];

        // �Փˏ����֐����Ăяo��
        c.a->OnCollision(c);
        Contact contactBtoA;
        contactBtoA.a = c.b;
        contactBtoA.b = c.a;
        contactBtoA.velocityA = c.velocityB;
        contactBtoA.velocityB = c.velocityA;
        contactBtoA.accelA = c.accelB;
        contactBtoA.accelB = c.accelA;
        contactBtoA.penetration = -c.penetration;
        contactBtoA.normal = -c.normal;
        contactBtoA.position = c.position;
        contactBtoA.penLength = c.penLength;
        c.b->OnCollision(contactBtoA);

        // �d�Ȃ����������
        SolveContact(contacts[i]);
      }

      // �폜�҂��̃A�N�^�[���폜����
      engine.RemoveDeadActors();

      // �J�����f�[�^���X�V����
      {
        std::shared_ptr<Actor> target = Find(actors, "Tiger-I");
        if (target) {
          const glm::mat4 matRot = glm::rotate(glm::mat4(1), target->rotation, glm::vec3(0, 1, 0));
          const glm::vec3 tankFront = matRot * glm::vec4(0, 0, 1, 1);
          cameraPosition = target->position + glm::vec3(0, 20, 20);
          cameraTarget = target->position;
        }
      }
    }

    //
    // �Q�[����Ԃ�`�悷��
    //

    glEnable(GL_DEPTH_TEST); // �[�x�o�b�t�@��L���ɂ���.
    //glEnable(GL_CULL_FACE);
    glClearColor(0.5f, 0.5f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    primitiveBuffer.BindVertexArray();
    pipeline.Bind();
    sampler->Bind(0);

    float s = sin(glm::radians(degree));
    float c = cos(glm::radians(degree));
    degree += 0.01f;
    glm::mat4 matT = glm::mat4(1);
    matT[3] = glm::vec4(-0.3, -0.5, 0.0, 1.0);
    glm::mat4 matS = glm::mat4(1);
    matS[0][0] = 0.5;
    matS[1][1] = 1.5;
    glm::mat4 matR = glm::mat4(1);
    matR[0][0] = c;
    matR[0][1] = -s;
    matR[1][0] = s;
    matR[1][1] = c;

    // �v���W�F�N�V�����s����쐬.
    const glm::vec2 windowSize = engine.GetWindowSize();
    const float aspectRatio = windowSize.x / windowSize.y;
    const glm::mat4 matProj =
      glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 200.0f);

    // �r���[�s����쐬.
    const glm::mat4 matView =
      glm::lookAt(cameraPosition, cameraTarget, glm::vec3(0, 1, 0));

    // �A�N�^�[��`�悷��
    for (int i = 0; i < actors.size(); ++i) {
      Draw(*actors[i], pipeline, matProj, matView);
    }
    engine.RenderUI();

    // �e�N�X�`���̊��蓖�Ă�����.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindSampler(0, 0);
    glBindProgramPipeline(0);
    primitiveBuffer.UnbindVertexArray();

    engine.SwapBuffers();
  }

  GameManager::Finalize();
  GameEngine::Finalize();

  return 0;
}