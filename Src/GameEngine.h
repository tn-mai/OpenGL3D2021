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
  void AddActor(std::shared_ptr<Actor> actor) { newActors.push_back(actor); }
  void UpdateActors(float deltaTime);
  void PostUpdateActors();
  void UpdatePysics(float deltaTime);
  void UpdateCamera();
  void NewFrame();
  void RemoveDeadActors();
  void RenderDefault();
  void RenderUI();
  void PostRender();

  PrimitiveBuffer& GetPrimitiveBuffer() { return *primitiveBuffer; }
  bool LoadPrimitive(const char* filename);
  const Primitive& GetPrimitive(const char* filename) const;
  const Primitive& GetPrimitive(int n) const { return primitiveBuffer->Get(n); }

  std::shared_ptr<Texture> LoadTexture(const char* filename);

  /**
  * ���̊֐���true��Ԃ�����E�B���h�E�����(=�A�v�����I��������)
  */
  bool WindowShouldClose() const
  {
    return glfwWindowShouldClose(window);
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
  * �E�B���h�E�T�C�Y��Ԃ�
  */
  glm::vec2 GetWindowSize() const
  {
    return windowSize;
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
  * ���C���J�������擾����
  */
  Camera& GetCamera() { return mainCamera; }
  const Camera& GetCamera() const { return mainCamera; }

  // �e�L�X�g�ō쐬���Ă��Ȃ������o�֐�
  std::shared_ptr<Texture> GetTexture(const char* filename) const;
  std::shared_ptr<Actor> FindActor(const char* name);
  unsigned int GetRandom();

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
  std::shared_ptr<Sampler> sampler;
  std::shared_ptr<Sampler> samplerUI;
  ActorList actors[layerCount]; // �A�N�^�[�z��
  ActorList newActors; // �ǉ�����A�N�^�[�̔z��
  std::shared_ptr<PrimitiveBuffer> primitiveBuffer; // �v���~�e�B�u�z��
  TextureBuffer textureBuffer;                      // �e�N�X�`���z��
  Camera mainCamera;

  // �e�L�X�g�ō쐬���Ă��Ȃ������o�ϐ�
  std::mt19937 rg;
};

#endif // GAMEENGINE_H_INCLUDED