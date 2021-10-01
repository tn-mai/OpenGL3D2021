/**
* @file GameEngine.cpp
*/
#include "GameEngine.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace {

GameEngine* engine = nullptr;

/**
* OpenGLからのメッセージを処理する.
*
* @param source    メッセージの発信者(OpenGL、Windows、シェーダーなど).
* @param type      メッセージの種類(エラー、警告など).
* @param id        メッセージを一位に識別する値.
* @param severity  メッセージの重要度(高、中、低、最低).
* @param length    メッセージの文字数. 負数ならメッセージは0終端されている.
* @param message   メッセージ本体.
* @param userParam コールバック設定時に指定したポインタ.
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

}

/**
* ゲームエンジンの初期化
*/
bool GameEngine::Initialize()
{
  if (!engine) {
    engine = new GameEngine;
    if (!engine) {
      return false;
    }

    // GLFWの初期化.
    if (glfwInit() != GLFW_TRUE) {
      return false;
    }

    // 描画ウィンドウの作成.
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

    // OpenGL関数のアドレスを取得する.
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
      glfwTerminate();
      return false;
    }

    glDebugMessageCallback(DebugCallback, nullptr);

    engine->window = window;
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    engine->windowSize = glm::vec2(w, h);

    engine->pipeline.reset(new ProgramPipeline("Res/FragmentLighting.vert", "Res/FragmentLighting.frag"));
    engine->pipelineUI.reset(new ProgramPipeline("Res/Simple.vert", "Res/Simple.frag"));
    engine->sampler = std::shared_ptr<Sampler>(new Sampler(GL_REPEAT));
    engine->samplerUI.reset(new Sampler(GL_CLAMP_TO_EDGE));

    for (int layer = 0; layer < layerCount; ++layer) {
      engine->actors[layer].reserve(1000);
    }

    engine->newActors.reserve(1000);
    engine->primitiveBuffer.reset(new PrimitiveBuffer(1'000'000, 4'000'000));
    engine->textureBuffer.reserve(1000);

    // カメラのアスペクト比を設定
    Camera& camera = engine->GetCamera();
    camera.aspectRatio = engine->windowSize.x / engine->windowSize.y;

    // ImGuiの初期化
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char glsl_version[] = "#version 450";
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGuiIO& io = ImGui::GetIO();

    // ウィンドウ状態のセーブ機能を無効化
    io.IniFilename = nullptr;

    // GUIの大きさを設定
    const float guiScale = 1.5f;
    ImGui::GetStyle().ScaleAllSizes(guiScale);

    // デフォルトフォントを指定
    const float defaultFontPixels = 13.0f;
    const float fontPixels = 32.0f;
    ImFont* font = io.Fonts->AddFontFromFileTTF(
      "Res/font/Makinas-4-Flat.otf",
      fontPixels, nullptr, io.Fonts->GetGlyphRangesJapanese());
    if (font) {
      io.FontDefault = font;
      io.FontGlobalScale = defaultFontPixels / fontPixels * guiScale;
    }

    std::random_device rd;
    engine->rg.seed(rd());
  }
  return true;
}

/**
* ゲームエンジンの終了
*/
void GameEngine::Finalize()
{
  if (engine) {
    // GUIの終了
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // GLFWの終了.
    glfwTerminate();

    delete engine;
    engine = nullptr;
  }
}

/**
* ゲームエンジンを取得する
*/
GameEngine& GameEngine::Get()
{
  return *engine;
}

/**
*
*/
std::shared_ptr<Actor> GameEngine::FindActor(const char* name)
{
  for (int layer = 0; layer < layerCount; ++layer) {
    std::shared_ptr<Actor> actor = Find(actors[layer], name);
    if (actor) {
      return actor;
    }
  }
  return nullptr;
}

/**
* ゲームエンジンを更新する
*/
void GameEngine::UpdateActors(float deltaTime)
{
  for (int layer = 0; layer < layerCount; ++layer) {
    ActorList& actors = this->actors[layer];
    // 以前の速度を更新
    for (int i = 0; i < actors.size(); ++i) {
      actors[i]->oldVelocity = actors[i]->velocity;
    }

    // アクターの状態を更新する
    for (int i = 0; i < actors.size(); ++i) {
      // アクターの寿命を減らす
      if (actors[i]->lifespan > 0) {
        actors[i]->lifespan -= deltaTime;

        // 寿命の尽きたアクターを「削除待ち」状態にする
        if (actors[i]->lifespan <= 0) {
          actors[i]->isDead = true;
          continue; // 削除待ちアクターは更新をスキップ
        }
      }

      actors[i]->OnUpdate(deltaTime);

      // 速度に重力加速度を加える
      if (!actors[i]->isStatic) {
        actors[i]->velocity.y += -9.8f * deltaTime * actors[i]->gravityScale;
      }

      // アクターの位置を更新する
      actors[i]->position += actors[i]->velocity * deltaTime;
    }
  }
}

/**
* ゲームエンジンを更新する(後処理)
*/
void GameEngine::PostUpdateActors()
{
  // 新規に作成されたアクターをアクター配列に追加する
  for (int i = 0; i < newActors.size(); ++i) {
    const int layer = static_cast<int>(newActors[i]->layer);
    if (layer >= 0 && layer < layerCount) {
      actors[layer].push_back(newActors[i]);
    }
  }

  // 新規アクター配列を空にする
  newActors.clear();
}

/**
* アクターの衝突判定を行う
*/
void GameEngine::UpdatePysics(float deltaTime)
{
  ActorList& actors = GetActors(Layer::Default);

  // 接地していない状態にする
  for (int i = 0; i < actors.size(); ++i) {
    actors[i]->contactCount = 0;
    actors[i]->isOnActor = false;
  }

#if 1
  // 非スタティックなアクターをリストアップ
  ActorList nonStaticActors;
  nonStaticActors.reserve(actors.size());
  for (std::shared_ptr<Actor>& e : actors) {
    if (!e->isStatic) {
      nonStaticActors.push_back(e);
    }
  }
#endif

  std::vector<Contact> contacts;
  contacts.reserve(actors.size());
  for (std::shared_ptr<Actor>& a : nonStaticActors) {
    for (std::shared_ptr<Actor>& b : actors) {
      // 同じアクターは衝突しない
      if (a == b) {
        continue;
      }

      // 削除待ちアクターは衝突しない
      if (a->isDead) {
        break;
      } else if (b->isDead) {
        continue;
      }

      Contact contact[2];
      if (DetectCollision(*a, *b, contact[0])) {
        contact[1] = Reverse(contact[0]);
        // 配列の中に、作成したコンタクト構造体と似ているものがあるか調べる
        for (const auto& e : contact) {
          auto itr = std::find_if(contacts.begin(), contacts.end(),
            [&e](const Contact& c) { return Equal2(e, c); });

          // 似ているコンタクト構造体が見つからなければ、作成した構造体を配列に追加する
          if (itr == contacts.end()) {
            contacts.push_back(e);
            ++e.a->contactCount;
          } else {
            // 似ている構造体が見つかった場合、浸透距離が長いほうを残す
            if (e.penLength > itr->penLength) {
              float massB = itr->massB + e.massB;
              *itr = e;
              itr->massB = massB;
            }
          }
        }
      }
    }
  }

  // 重なりを解決する
  for (int i = 0; i < contacts.size(); ++i) {
    Contact& c = contacts[i];

    // 重なりを解決する
    if (!c.a->isStatic) {
      SolveContact(c);
    }

    // 衝突処理関数を呼び出す
    c.a->OnCollision(c);
#if 0
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
#endif
  }
}

/**
* カメラの状態を更新する
*/
void GameEngine::UpdateCamera()
{
  mainCamera.Update();
}

/**
* 新しいフレームを開始する
*/
void GameEngine::NewFrame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

/**
* 削除待ちのアクターを削除する
*/
void GameEngine::RemoveDeadActors()
{
  for (int layer = 0; layer < layerCount; ++layer) {
    ActorList& a = actors[layer];
    a.erase(std::remove_if(a.begin(), a.end(),
      [](std::shared_ptr<Actor>& a) { return a->isDead; }),
      a.end());
  }
}

/**
* デフォルトアクターを描画する
*/
void GameEngine::RenderDefault()
{
  glEnable(GL_DEPTH_TEST); // 深度バッファを有効にする.
  //glEnable(GL_CULL_FACE);
  glClearColor(0.5f, 0.5f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //glEnable(GL_BLEND);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  primitiveBuffer->BindVertexArray();
  pipeline->Bind();
  sampler->Bind(0);

  const glm::mat4& matProj = mainCamera.GetProjectionMatrix();
  const glm::mat4 matView = mainCamera.GetViewMatrix();

  // アクターを描画する
  const int layer = static_cast<int>(Layer::Default);
  ActorList& defaultActors = actors[layer];
  for (int i = 0; i < defaultActors.size(); ++i) {
    Draw(*defaultActors[i], *pipeline, matProj, matView);
  }
}

/**
* UIアクターを描画する
*/
void GameEngine::RenderUI()
{
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  primitiveBuffer->BindVertexArray();
  pipelineUI->Bind();
  samplerUI->Bind(0);

  // プロジェクション行列を作成.
  const glm::vec2 halfSize = windowSize * 0.5f;
  const glm::mat4 matProj =
    glm::ortho(-halfSize.x, halfSize.x, -halfSize.y, halfSize.y, 1.0f, 200.0f);

  // ビュー行列を作成.
  const glm::mat4 matView =
    glm::lookAt(glm::vec3(0, 0, 100), glm::vec3(0), glm::vec3(0, 1, 0));

  // Z座標の降順で2Dアクターを描画する
  ActorList a = actors[static_cast<int>(Layer::UI)];
  std::sort(a.begin(), a.end(),
    [](std::shared_ptr<Actor>& a, std::shared_ptr<Actor>& b) {
      return a->position.z < b->position.z; });
  for (int i = 0; i < a.size(); ++i) {
    Draw(*a[i], *pipelineUI, matProj, matView);
  }

  pipelineUI->Unbind();
  samplerUI->Unbind(0);
  primitiveBuffer->UnbindVertexArray();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/**
* 描画の後始末をする
*/
void GameEngine::PostRender()
{
  // テクスチャの割り当てを解除.
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindSampler(0, 0);
  glBindProgramPipeline(0);
  primitiveBuffer->UnbindVertexArray();
}

/**
*
*/
bool GameEngine::LoadPrimitive(const char* filename)
{
  return primitiveBuffer->AddFromObjFile(filename);
}

/**
* 名前の一致するプリミティブを取得する
*
* @param filename プリミティブ名
*
* @return filenameと名前が一致するプリミティブ
*/
const Primitive& GameEngine::GetPrimitive(const char* filename) const
{
  return primitiveBuffer->Find(filename);
}

/**
* テクスチャを読み込む
*/
std::shared_ptr<Texture> GameEngine::LoadTexture(const char* filename)
{
  TextureBuffer::iterator itr = textureBuffer.find(filename);
  if (itr == textureBuffer.end()) {
    std::shared_ptr<Texture> tex(new Texture(filename));
    textureBuffer.insert(std::make_pair(std::string(filename), tex));
    return tex;
  }
  return itr->second;
}

/**
* テクスチャを取得する
*
* @param filename テクスチャファイル名
*
* @return filenameから作成されたテクスチャ
*/
std::shared_ptr<Texture> GameEngine::GetTexture(const char* filename) const
{
  TextureBuffer::const_iterator itr = textureBuffer.find(filename);
  if (itr == textureBuffer.end()) {
    static std::shared_ptr<Texture> tex(new Texture("[Dummy for GetTexture]"));
    return tex;
  }
  return itr->second;
}

/**
* 乱数を取得する
*/
unsigned int GameEngine::GetRandom()
{
  return rg();
}

