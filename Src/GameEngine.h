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
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <random>

using ActorList = std::vector<std::shared_ptr<Actor>>;
using TextureBuffer = std::unordered_map<std::string, std::shared_ptr<Texture>>;

/**
* ゲームエンジン
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
  void UpdatePhysics(float deltaTime);
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
  std::shared_ptr<Texture> LoadTexture(const char* name, const char** fileList, size_t count);

  void UpdateGroundMap(int x, int y, int width, int height, const void* data);

  /**
  * この関数がtrueを返したらウィンドウを閉じる(=アプリを終了させる)
  */
  bool WindowShouldClose() const
  {
    return glfwWindowShouldClose(window);
  }

  /**
  * キーが押されていたらtrue、押されていなかったらfalse
  */
  bool GetKey(int key) const
  {
    return glfwGetKey(window, key) == GLFW_PRESS;
  }

  /**
  * ボタンが押されていたらtrue、押されていなかったらfalse
  */
  int GetMouseButton(int button) const
  {
    return glfwGetMouseButton(window, button);
  }

  /**
  * ウィンドウサイズを返す
  */
  glm::vec2 GetWindowSize() const
  {
    return windowSize;
  }

  /**
  * フロントバッファとバックバッファを交換する
  */
  void SwapBuffers() const
  {
    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  /**
  * アプリ起動時からの経過時間(秒)を取得する
  */
  double GetTime() const
  {
    return glfwGetTime();
  }

  /**
  * メインカメラを取得する
  */
  Camera& GetCamera() { return mainCamera; }
  const Camera& GetCamera() const { return mainCamera; }

  // TODO: テキスト未追加
  std::shared_ptr<Texture> GetTexture(const char* filename) const;
  std::shared_ptr<Actor> FindActor(const char* name);
  unsigned int GetRandom();
  size_t GetTextureCount() const { return textureBuffer.size(); }
  bool LoadGameMap(const char* filename);
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
  // パイプライン・オブジェクトを作成する.
  std::shared_ptr<ProgramPipeline> pipeline;
  std::shared_ptr<ProgramPipeline> pipelineUI;
  std::shared_ptr<Sampler> sampler;
  std::shared_ptr<Sampler> samplerUI;

  std::shared_ptr<FramebufferObject> fboShadow; // 影描画用FBO

  // 地面描画用
  glm::ivec2 mapSize = glm::ivec2(21, 21);
  std::shared_ptr<ProgramPipeline> pipelineGround;
  std::shared_ptr<Texture> texMap;

  ActorList actors[layerCount]; // アクター配列
  ActorList newActors; // 追加するアクターの配列
  std::shared_ptr<PrimitiveBuffer> primitiveBuffer; // プリミティブ配列
  TextureBuffer textureBuffer;                      // テクスチャ配列
  Camera mainCamera;

  // TODO: テキスト未追加
  std::mt19937 rg;
  std::shared_ptr<FramebufferObject> fbo;
  std::shared_ptr<ProgramPipeline> pipelineShadow;
  std::shared_ptr<Sampler> samplerShadow;
};

#endif // GAMEENGINE_H_INCLUDED
