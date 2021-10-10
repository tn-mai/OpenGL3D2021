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

//// �A�N�^�[�̔z��.
//std::vector<std::shared_ptr<Actor>> actors;

/**
* �G���g���[�|�C���g.
*/
int main()
{
  GameEngine::Initialize();
  GameEngine& engine = GameEngine::Get();

  // �Q�[���}�l�[�W�����쐬
  GameManager::Initialize();
  GameManager& manager = GameManager::Get();

  // ���C�����[�v.
  double loopTime = engine.GetTime();     // 1/60�b�Ԋu�Ń��[�v�������邽�߂̎���
  double diffLoopTime = 0;             // �����̍���
  const float deltaTime = 1.0f / 60.0f;// ���ԊԊu
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
    engine.NewFrame();
    for (; diffLoopTime >= deltaTime; diffLoopTime -= deltaTime) {
      engine.UpdateActors(deltaTime);
      manager.Update(deltaTime);
      engine.PostUpdateActors();
      engine.UpdatePhysics(deltaTime);
      manager.UpdateCamera();
      engine.UpdateCamera();
      engine.RemoveDeadActors();
    }
    manager.UpdateUI();

    //
    // �Q�[����Ԃ�`�悷��
    //
    engine.RenderDefault();
    engine.RenderUI();
    engine.PostRender();
    engine.SwapBuffers();
  }

  GameManager::Finalize();
  GameEngine::Finalize();

  return 0;
}