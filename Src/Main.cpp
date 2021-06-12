/**
* @file Main.cpp
*/
#include <glad/glad.h>
#include "GLContext.h"
#include "Primitive.h"
#include "ProgramPipeline.h"
#include "Texture.h"
#include "Sampler.h"
#include "Actor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <memory>
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
  {(-0.33f/2.0f)        * 10.0f, ( 0.5f/2.0f)      * 10.0f, 0.6f },
  {( 0.33f/2.0f)        * 10.0f, ( 0.5f/2.0f)      * 10.0f, 0.6f },
  {( 0.00f/2.0f)        * 10.0f, (-0.5f/2.0f)      * 10.0f, 0.6f },
  {(-0.33f/2.0f-0.165f) * 10.0f, ( 0.5f/2.0f+0.5f) * 10.0f, 0.6f },
  {( 0.33f/2.0f-0.165f) * 10.0f, ( 0.5f/2.0f+0.5f) * 10.0f, 0.6f },
  {( 0.00f/2.0f-0.165f) * 10.0f, (-0.5f/2.0f+0.5f) * 10.0f, 0.6f },
  {(-0.33f/2.0f+0.165f) * 10.0f, ( 0.5f/2.0f+0.5f) * 10.0f, 0.6f },
  {( 0.33f/2.0f+0.165f) * 10.0f, ( 0.5f/2.0f+0.5f) * 10.0f, 0.6f },
  {( 0.00f/2.0f+0.165f) * 10.0f, (-0.5f/2.0f+0.5f) * 10.0f, 0.6f },
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
int mapData[10][10] = {
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 2, 2, 2, 2, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 2, 2, 2, 2},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
  { 0, 0, 0, 1, 2, 2, 1, 0, 0, 0},
};

/// �I�u�W�F�N�g�}�b�v�f�[�^.
int objectMapData[10][10] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 1, 1, 1, 0, 0, 0, 0, 2, 0, 0},
  { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 2, 0, 0, 0, 0, 2, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

/**
* OpenGL����̃��b�Z�[�W����������.
*
* @param source    ���b�Z�[�W�̔��M��(OpenGL�AWindows�A�V�F�[�_�[�Ȃ�).
* @param type      ���b�Z�[�W�̎��(�G���[�A�x���Ȃ�).
* @param id        ���b�Z�[�W����ʂɎ��ʂ���l.
* @param severity  ���b�Z�[�W�̏d�v�x(���A���A��A�Œ�).
* @param length    ���b�Z�[�W�̕�����. �����Ȃ烁�b�Z�[�W��0�I�[����Ă���.
* @param message   ���b�Z�[�W�{��.
* @param userParam �R�[���o�b�N�ݒ莞�Ɏw�肵���|�C���^.
*/
void GLAPIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
  if (length < 0) {
    std::cerr << message << "\n";
  } else {
    const std::string s(message, message + length);
    std::cerr << s << "\n";
  }
}

/**
* �G���g���[�|�C���g.
*/
int main()
{
  // GLFW�̏�����.
  if (glfwInit() != GLFW_TRUE) {
    return 1;
  }

  // �`��E�B���h�E�̍쐬.
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  GLFWwindow* window =
    glfwCreateWindow(1280, 720, "OpenGLGame", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);

  // OpenGL�֐��̃A�h���X���擾����.
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    glfwTerminate();
    return 1;
  }

  glDebugMessageCallback(DebugCallback, nullptr);

  // VAO���쐬����.
  PrimitiveBuffer primitiveBuffer(200'000, 800'000);

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

  // �T���v�����쐬.
  std::shared_ptr<Sampler> sampler(new Sampler(GL_REPEAT));

  // ��Ԃ̃p�����[�^
  Actor tank = { primitiveBuffer.Get(6), texTank,
    glm::vec3(0), glm::vec3(1), 0.0f, glm::vec3(0) };

  // T-34��Ԃ̃p�����[�^
  Actor tankT34 = { primitiveBuffer.Get(7), texTankT34,
    glm::vec3(-5, 0, 0), glm::vec3(1), 0.0f, glm::vec3(0) };

  // �����̃p�����[�^
  Actor brickHouse = { primitiveBuffer.Get(8), texBrickHouse,
    glm::vec3(-8, 0, 0), glm::vec3(2, 2, 2), 0.0f, glm::vec3(-2.6f, 2.0f, 0.8f) };

  // ���C�����[�v.
  double loopTime = glfwGetTime(); // 1/60�b�Ԋu�Ń��[�v�������邽�߂̎���
  double diffLoopTime = 0;         // �����̍���
  while (!glfwWindowShouldClose(window)) {
    // ���ݎ������擾
    const double curLoopTime = glfwGetTime();
    // ���ݎ����ƑO�񎞍��̍����A�����̍����ɉ��Z
    diffLoopTime += curLoopTime - loopTime;
    // ���������ݎ����ɍX�V
    loopTime = curLoopTime;
    // �����̍�����1/60�b�����Ȃ�A���[�v�̐擪�ɖ߂�
    if (diffLoopTime < 1.0 / 60.0) {
      continue;
    }

    //
    // �Q�[����Ԃ��X�V����
    //

    for (; diffLoopTime >= 1.0 / 60.0; diffLoopTime -= 1.0 / 60.0) {
      // ��Ԃ��ړ�������
      if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        tank.rotation += glm::radians(90.0f) / 60.0f;
      } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        tank.rotation -= glm::radians(90.0f) / 60.0f;
      }

      // tank.rotation��0�̂Ƃ��̐�Ԃ̌����x�N�g��
      glm::vec3 tankFront(0, 0, 1);
      // tank.rotation���W�A��������]�������]�s������
      const glm::mat4 matRot = glm::rotate(glm::mat4(1), tank.rotation, glm::vec3(0, 1, 0));
      // �����x�N�g����tank.rotation������]������
      tankFront = matRot * glm::vec4(tankFront, 1);

      if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        tank.position += tankFront * 4.0f / 60.0f;
      } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        tank.position -= tankFront * 4.0f / 60.0f;
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
    matT[3] = glm::vec4(-0.3,-0.5, 0.0, 1.0);
    glm::mat4 matS = glm::mat4(1);
    matS[0][0] = 0.5;
    matS[1][1] = 1.5;
    glm::mat4 matR = glm::mat4(1);
    matR[0][0] = c;
    matR[0][1] = -s;
    matR[1][0] = s;
    matR[1][1] = c;

    // �v���W�F�N�V�����s����쐬.
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    const float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
    const glm::mat4 matProj =
      glm::perspective(glm::radians(45.0f),  aspectRatio, 0.1f, 200.0f);

    // �r���[�s����쐬.
    const glm::mat4 matView =
      glm::lookAt(glm::vec3(0, 20, 20), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    // �O�p�`��`�悷��
    Actor actTriangle = { primitiveBuffer.Get(2), texTriangle,
      glm::vec3(0), glm::vec3(1), 0.0f, glm::vec3(0),
    };
    Draw(actTriangle, pipeline, matProj, matView);

    // �����̂�`�悷��
    Actor actCube = { primitiveBuffer.Get(3), texTriangle,
      glm::vec3(0), glm::vec3(1), 0.0f, glm::vec3(0),
    };
    Draw(actCube, pipeline, matProj, matView);

    // ������\��(�ۑ�12)
    Draw(brickHouse, pipeline, matProj, matView);

    // ��Ԃ�\��
    Draw(tank, pipeline, matProj, matView);

    // T-34��\��(�ۑ�05)
    Draw(tankT34, pipeline, matProj, matView);

    // �}�b�v�ɔz�u���镨�̂̕\���f�[�^.
    struct ObjectData {
      Primitive prim;
      const std::shared_ptr<Texture> tex;
    };

    // �`�悷�镨�̂̃��X�g.
    const ObjectData objectList[] = {
      { Primitive(), 0 },    // �Ȃ�
      { primitiveBuffer.Get(4), texTree }, // ��
      { primitiveBuffer.Get(5), texWarehouse }, // ����
    };
    // �؂�A����.
    //glBindTextureUnit(0, texTree); // �e�N�X�`�������蓖�Ă�.
    //primTree.Draw();
#if 1
    for (int y = 0; y < 10; ++y) {
      for (int x = 0; x < 10; ++x) {
        const int objectNo = objectMapData[y][x];
        if (objectNo <= 0 || objectNo >= std::size(objectList)) {
          continue;
        }
        auto p = objectList[objectNo];

        // �l�p�`��4x4m�Ȃ̂ŁAx��y��4�{�����ʒu�ɕ\������.
        const glm::vec3 position(x * 4 - 20, 0, y * 4 - 20);

        // �s����V�F�[�_�ɓ]������ 
        const glm::mat4 matModel = glm::translate(glm::mat4(1), position);
        const glm::mat4 matMVP = matProj * matView * matModel;
        pipeline.SetUniform(locMatTRS, matMVP);
        pipeline.SetUniform(locMatModel, matModel);

        p.tex->Bind(0); // �e�N�X�`�������蓖�Ă�.
        p.prim.Draw();
      }
    }
#endif

    // �}�b�v��(-20,-20)-(20,20)�͈̔͂ɕ`��.
    const std::shared_ptr<Texture> mapTexList[] = { texGreen, texGround, texRoad };
    for (int y = 0; y < 10; ++y) {
      for (int x = 0; x < 10; ++x) {
        // �l�p�`��4x4m�Ȃ̂ŁAx��y��4�{�����ʒu�ɕ\������.
        const glm::vec3 position(x * 4 - 20, 0, y * 4 - 20);

        // �s����V�F�[�_�ɓ]������ 
        const glm::mat4 matModel = glm::translate(glm::mat4(1), position);
        const glm::mat4 matMVP = matProj * matView * matModel;
        pipeline.SetUniform(locMatTRS, matMVP);
        pipeline.SetUniform(locMatModel, matModel);

        const int textureNo = mapData[y][x];
        mapTexList[textureNo]->Bind(0); // �e�N�X�`�������蓖�Ă�.
        primitiveBuffer.Get(0).Draw();
      }
    }

    // �e�N�X�`���̊��蓖�Ă�����.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindSampler(0, 0);
    glBindProgramPipeline(0);
    primitiveBuffer.UnbindVertexArray();

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  // GLFW�̏I��.
  glfwTerminate();

  return 0;
}