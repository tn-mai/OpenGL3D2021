/**
* @file GameEngine.h
*/
#ifndef GAMEENGINE_H_INCLUDED
#define GAMEENGINE_H_INCLUDED
#include "Primitive.h"
#include "Texture.h"
#include "ProgramPipeline.h"
#include "Sampler.h"
#include "Actor.h"
#include "Camera.h"
#include "FramebufferObject.h"
#include "Sprite.h"
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <random>

using ActorList = std::vector<std::shared_ptr<Actor>>;
using TextureBuffer = std::unordered_map<std::string, std::shared_ptr<Texture>>;

/**
* �Q�[���G���W��
*/
class GameEngine
{
public:
  static bool Initialize();
  static void Finalize();
  static GameEngine& Get();

  ActorList& GetActors(Layer layer = Layer::Default)
  {
    return actors[static_cast<int>(layer)];
  }
  ActorList& GetNewActors() { return newActors; }
  void AddActor(std::shared_ptr<Actor> actor) { newActors.push_back(actor); }
  std::shared_ptr<Actor> FindActor(const char* name);
  void ClearAllActors();
  void UpdateActors(float deltaTime);
  void PostUpdateActors();
  void UpdatePhysics(float deltaTime);
  void UpdateCamera();
  void NewFrame();
  void RemoveDeadActors();
  void RenderDefault();
  void RenderSprite();
  void RenderUI();
  void PostRender();

  PrimitiveBuffer& GetPrimitiveBuffer() { return *primitiveBuffer; }
  bool LoadPrimitive(const char* filename);
  const Primitive& GetPrimitive(const char* filename) const;
  const Primitive& GetPrimitive(int n) const { return primitiveBuffer->Get(n); }
  const MeshPtr& LoadMesh(const char* name);

  std::shared_ptr<Texture> LoadTexture(const char* filename);
  std::shared_ptr<Texture> LoadTexture(const char* name, const char** fileList, size_t count);

  void UpdateGroundMap(int x, int y, int width, int height, const void* data);
  void ResizeGroundMap(int width, int height, const void* data);

  /**
  * ���̊֐���true��Ԃ�����E�B���h�E�����(=�A�v�����I��������)
  */
  bool WindowShouldClose() const
  {
    return glfwWindowShouldClose(window);
  }

  // 19b�Ŏ���. 19�͖�����
  /**
  * �A�v���I���t���O���Z�b�g����
  */
  void SetWindowShouldClose(bool isClose)
  {
    glfwSetWindowShouldClose(window, isClose);
  }

  /**
  * �L�[��������Ă�����true�A������Ă��Ȃ�������false
  */
  bool GetKey(int key) const
  {
    return glfwGetKey(window, key) == GLFW_PRESS;
  }

  /**
  * �{�^����������Ă�����true�A������Ă��Ȃ�������false
  */
  int GetMouseButton(int button) const
  {
    return glfwGetMouseButton(window, button);
  }

  /**
  * �}�E�X���W���擾
  *
  * 21�Ŏ���. 21b�͖�����
  */
  glm::vec2 GetMousePosition() const;

  /**
  * �E�B���h�E�T�C�Y��Ԃ�
  */
  glm::vec2 GetWindowSize() const
  {
    return windowSize;
  }

  /**
  * �n�ʃ}�b�v�̃T�C�Y��Ԃ�
  */
  glm::ivec2 GetMapSize() const
  {
    return mapSize;
  }

  /**
  * �t�����g�o�b�t�@�ƃo�b�N�o�b�t�@����������
  */
  void SwapBuffers() const
  {
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  /**
  * �A�v���N��������̌o�ߎ���(�b)���擾����
  */
  double GetTime() const
  {
    return glfwGetTime();
  }

  /**
  * �������擾����
  *
  * 21b�Ŏ���. 21�͖�����.
  */
  int GetRandomInt(int min, int max)
  {
    return std::uniform_int_distribution<>(min, max)(random);
  }
  float GetRandomFloat(float min, float max)
  {
    return std::uniform_real_distribution<float>(min, max)(random);
  }
  float GetRandomNormal(float a, float b = 1.0f)
  {
    return std::normal_distribution<float>(a, b)(random);
  }

  /**
  * ���C���J�������擾����
  */
  Camera& GetCamera() { return mainCamera; }
  const Camera& GetCamera() const { return mainCamera; }

  // �ۑ�
  void ShowCollider(bool flag) { showCollider = flag; }

  // TODO: �e�L�X�g���ǉ�
  std::shared_ptr<Texture> GetTexture(const char* filename) const;
  unsigned int GetRandom();
  size_t GetTextureCount() const { return textureBuffer.size(); }
  std::shared_ptr<Texture> GetTexture(int n) const
  {
    const size_t count = textureBuffer.bucket_count();
    for (size_t i = 0; i < count; ++i) {
      const size_t size = textureBuffer.bucket_size(i);
      if (n < size) {
        auto itr = textureBuffer.begin(i);
        std::advance(itr, n);
        return itr->second;
      }
      n -= static_cast<int>(size);
    }
    return nullptr;
  }

private:
  GameEngine() = default;
  ~GameEngine() = default;
  GameEngine(const GameEngine&) = delete;
  GameEngine& operator=(const GameEngine&) = delete;

  GLFWwindow* window = nullptr;
  glm::vec2 windowSize = glm::vec2(0);
  // �p�C�v���C���E�I�u�W�F�N�g���쐬����.
  std::shared_ptr<ProgramPipeline> pipeline;
  std::shared_ptr<ProgramPipeline> pipelineUI;
  std::shared_ptr<ProgramPipeline> pipelineDoF;
  std::shared_ptr<ProgramPipeline> pipelineInstancedMesh;
  std::shared_ptr<Sampler> sampler;
  std::shared_ptr<Sampler> samplerUI;
  std::shared_ptr<Sampler> samplerDoF;

  std::shared_ptr<FramebufferObject> fboColor0; // ���{FBO
  std::shared_ptr<FramebufferObject> fboColor1; // �k���pFBO
  std::shared_ptr<FramebufferObject> fboShadow; // �e�`��pFBO

  // �n�ʕ`��p
  glm::ivec2 mapSize = glm::ivec2(21, 21);
  std::shared_ptr<ProgramPipeline> pipelineGround;
  std::shared_ptr<Texture> texMap;

  ActorList actors[layerCount]; // �A�N�^�[�z��
  ActorList newActors; // �ǉ�����A�N�^�[�̔z��
  std::shared_ptr<PrimitiveBuffer> primitiveBuffer; // �v���~�e�B�u�z��
  TextureBuffer textureBuffer;                      // �e�N�X�`���z��
  Camera mainCamera;

  std::mt19937 random; // 21b�Ŏ���. 21�͖�����.

  // �R���C�_�[�\���p�ϐ�(�f�o�b�O�p)
  bool showCollider = false; // �R���C�_�[�\���t���O
  std::shared_ptr<ProgramPipeline> pipelineCollider;
  std::shared_ptr<Texture> texCollider;

  // �X�v���C�g�`��p
  SpriteRenderer spriteRenderer;

  // TODO: �e�L�X�g���ǉ�
  std::shared_ptr<Sampler> samplerShadow;
};

#endif // GAMEENGINE_H_INCLUDED
