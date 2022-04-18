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
  * この関数がtrueを返したらウィンドウを閉じる(=アプリを終了させる)
  */
  bool WindowShouldClose() const
  {
    return glfwWindowShouldClose(window);
  }

  // 19bで実装. 19は未実装
  /**
  * アプリ終了フラグをセットする
  */
  void SetWindowShouldClose(bool isClose)
  {
    glfwSetWindowShouldClose(window, isClose);
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
  * マウス座標を取得
  *
  * 21で実装. 21bは未実装
  */
  glm::vec2 GetMousePosition() const;

  /**
  * ウィンドウサイズを返す
  */
  glm::vec2 GetWindowSize() const
  {
    return windowSize;
  }

  /**
  * 地面マップのサイズを返す
  */
  glm::ivec2 GetMapSize() const
  {
    return mapSize;
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
  * 乱数を取得する
  *
  * 21bで実装. 21は未実装.
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
  * メインカメラを取得する
  */
  Camera& GetCamera() { return mainCamera; }
  const Camera& GetCamera() const { return mainCamera; }

  // 課題
  void ShowCollider(bool flag) { showCollider = flag; }

  // TODO: テキスト未追加
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
  // パイプライン・オブジェクトを作成する.
  std::shared_ptr<ProgramPipeline> pipeline;
  std::shared_ptr<ProgramPipeline> pipelineUI;
  std::shared_ptr<ProgramPipeline> pipelineDoF;
  std::shared_ptr<ProgramPipeline> pipelineInstancedMesh;
  std::shared_ptr<Sampler> sampler;
  std::shared_ptr<Sampler> samplerUI;
  std::shared_ptr<Sampler> samplerDoF;

  std::shared_ptr<FramebufferObject> fboColor0; // 等倍FBO
  std::shared_ptr<FramebufferObject> fboColor1; // 縮小用FBO
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

  std::mt19937 random; // 21bで実装. 21は未実装.

  // コライダー表示用変数(デバッグ用)
  bool showCollider = false; // コライダー表示フラグ
  std::shared_ptr<ProgramPipeline> pipelineCollider;
  std::shared_ptr<Texture> texCollider;

  // スプライト描画用
  SpriteRenderer spriteRenderer;

  // TODO: テキスト未追加
  std::shared_ptr<Sampler> samplerShadow;
};

#endif // GAMEENGINE_H_INCLUDED
