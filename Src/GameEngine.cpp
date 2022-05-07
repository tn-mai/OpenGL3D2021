/**
* @file GameEngine.cpp
*/
#include "GameEngine.h"
#include "GltfMesh.h"
#include "Audio.h"
#include "Audio/OpenGLGame_acf.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>

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
  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
    return;
  }

  if (length < 0) {
    std::cerr << message << "\n";
  } else {
    const std::string s(message, message + length);
    std::cerr << s << "\n";
  }
}

/**
* 直方体(ライン)を作成(デバッグ用)
*/
void CreateBoxPrimitive(PrimitiveBuffer& buffer, const glm::vec3& min, const glm::vec3& max)
{
  const int count = 4;
  std::vector<glm::vec3> positions(count * 2);
  std::vector<glm::vec3> normals(count * 2);
  std::vector<glm::vec2> texcoords(count * 2, glm::vec2(0));
  std::vector<glm::vec4> colors(count * 2, glm::vec4(0.8f, 0.4f, 0.1f, 1.0f));
  positions[0] = glm::vec3(min.x, min.y, min.z);
  positions[1] = glm::vec3(max.x, min.y, min.z);
  positions[2] = glm::vec3(max.x, min.y, max.z);
  positions[3] = glm::vec3(min.x, min.y, max.z);
  for (int i = 0; i < count; ++i) {
    positions[i + 4] = glm::vec3(positions[i].x, max.y, positions[i].z);
  }
  for (int i = 0; i < count * 2; ++i) {
    normals[i] = glm::normalize(positions[i]);
  }

  std::vector<GLushort> indices(count * 6);
  for (int i = 0; i < count; ++i) {
    indices[(i + count * 0) * 2] = i;
    indices[(i + count * 0) * 2 + 1] = (i + 1) % count;
    indices[(i + count * 1) * 2] = i + count;
    indices[(i + count * 1) * 2 + 1] = (i + 1) % count + count;
    indices[(i + count * 2) * 2] = i;
    indices[(i + count * 2) * 2 + 1] = i + count;
  }

  buffer.Add(positions.size(),
    positions.data(), colors.data(), texcoords.data(), normals.data(),
    nullptr, indices.size(), indices.data(), "Collider(Box)", GL_LINES);
}

/**
*
*/
void CreateSpherePrimitive(PrimitiveBuffer& buffer, float radius, int longitude, int latitude)
{
  longitude = std::max(3, longitude);
  latitude = std::max(3, latitude);
  const int count2 = longitude * latitude;
  std::vector<glm::vec3> positions(count2);
  std::vector<glm::vec3> normals(count2);
  std::vector<glm::vec2> texcoords(count2, glm::vec2(0));
  std::vector<glm::vec4> colors(count2, glm::vec4(0.8f, 0.4f, 0.1f, 1.0f));

  int i = 0;
  for (int lon = 0; lon < longitude; ++lon) {
    const float ry = glm::radians(180.0f / static_cast<float>(longitude - 1) * lon);
    const float y = std::cos(ry);
    const float scaleXZ = std::sin(ry);
    for (int lat = 0; lat < latitude; ++lat) {
      const float rx = glm::radians(360.0f / static_cast<float>(latitude) * lat);
      const float x = std::cos(rx) * scaleXZ;
      const float z = -std::sin(rx) * scaleXZ;
      normals[i] = glm::vec3(x, y, z);
      positions[i] = glm::vec3(x, y, z) * radius;
      ++i;
    }
  }

  std::vector<GLushort> indices((longitude - 1) * latitude * 8);
  i = 0;
  for (int lon = 0; lon < longitude - 1; ++lon) {
    for (int lat = 0; lat < latitude; ++lat) {
      const int base = lon * latitude + lat;
      const int next = lon * latitude + (lat + 1) % latitude;
      indices[base * 8 + 0] = base;
      indices[base * 8 + 1] = next;

      indices[base * 8 + 2] = next;
      indices[base * 8 + 3] = next + latitude;

      indices[base * 8 + 4] = next + latitude;
      indices[base * 8 + 5] = base + latitude;

      indices[base * 8 + 6] = base + latitude;
      indices[base * 8 + 7] = base;
    }
  }

  buffer.Add(positions.size(),
    positions.data(), colors.data(), texcoords.data(), normals.data(),
    nullptr, indices.size(), indices.data(), "Collider(Sphere)", GL_LINES);
}

/**
* 円柱(ライン)を作成(デバッグ用)
*/
void CreateCylinderPrimitive(PrimitiveBuffer& buffer, float top, float bottom, float radius, int count)
{
  count = std::max(3, count);
  std::vector<glm::vec3> positions(count * 2);
  std::vector<glm::vec3> normals(count * 2);
  std::vector<glm::vec2> texcoords(count * 2, glm::vec2(0));
  std::vector<glm::vec4> colors(count * 2, glm::vec4(0.8f, 0.4f, 0.1f, 1.0f));
  for (int i = 0; i < count; ++i) {
    const float r = glm::radians(360.0f / static_cast<float>(count) * i);
    float x = std::cos(r);
    float z = -std::sin(r);
    normals[i] = glm::vec3(x, 0, z);
    normals[i + count] = glm::vec3(x, 0, z);
    x *= radius;
    z *= radius;
    positions[i] = glm::vec3(x, top, z);
    positions[i + count] = glm::vec3(x, bottom, z);
  }

  std::vector<GLushort> indices(count * 2 * 3);
  for (int i = 0; i < count; ++i) {
    indices[(i + count * 0) * 2] = i;
    indices[(i + count * 0) * 2 + 1] = (i + 1) % count;
    indices[(i + count * 1) * 2] = i + count;
    indices[(i + count * 1) * 2 + 1] = (i + 1) % count + count;
    indices[(i + count * 2) * 2] = i;
    indices[(i + count * 2) * 2 + 1] = i + count;
  }

  buffer.Add(positions.size(),
    positions.data(), colors.data(), texcoords.data(), normals.data(),
    nullptr, indices.size(), indices.data(), "Collider(Cylinder)", GL_LINES);
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

    // 乱数を初期化する
    std::random_device rd;
    engine->random.seed(rd());

    engine->pipeline.reset(new ProgramPipeline(
      "Res/FragmentLighting.vert", "Res/FragmentLighting.frag"));
    engine->pipelineUI.reset(new ProgramPipeline(
      "Res/Simple.vert", "Res/Simple.frag"));
    engine->pipelineDoF.reset(new ProgramPipeline(
      "Res/DepthOfField.vert", "Res/DepthOfField.frag"));
    engine->pipelineInstancedMesh.reset(new ProgramPipeline(
      "Res/InstancedMesh.vert", "Res/FragmentLighting.frag"));
    engine->pipelineStaticMesh.reset(new ProgramPipeline(
      "Res/StaticMesh.vert", "Res/StaticMesh.frag"));

    engine->sampler = std::shared_ptr<Sampler>(new Sampler(GL_REPEAT));
    engine->samplerUI.reset(new Sampler(GL_CLAMP_TO_EDGE));
    engine->samplerDoF.reset(new Sampler(GL_CLAMP_TO_EDGE));

    engine->samplerShadow.reset(new Sampler(GL_CLAMP_TO_EDGE));

    // 地面マップ用データを作成
    engine->pipelineGround.reset(new ProgramPipeline(
      "Res/FragmentLighting.vert", "Res/GroundShader.frag"));
    std::vector<uint32_t> mapData(engine->mapSize.x * engine->mapSize.y, 0);
    engine->ResizeGroundMap(
      engine->mapSize.x, engine->mapSize.y,
      mapData.data());

    for (int layer = 0; layer < layerCount; ++layer) {
      engine->actors[layer].reserve(1000);
    }

    engine->newActors.reserve(1000);
    engine->primitiveBuffer.reset(new PrimitiveBuffer(1'000'000, 4'000'000));
    engine->textureBuffer.reserve(1000);

    // コライダー表示用データを作成(デバッグ用)
    engine->pipelineCollider = engine->pipelineUI; // UIシェーダを流用
    const std::vector<uint32_t> imageWhite(4 * 4, 0xff1080f0);
    engine->texCollider = std::make_shared<Texture>(
      "Debug(Collider)", 4, 4, imageWhite.data(), GL_RGBA, GL_UNSIGNED_BYTE);
    CreateBoxPrimitive(*engine->primitiveBuffer, glm::vec3(-1.0f), glm::vec3(1.0f));
    CreateSpherePrimitive(*engine->primitiveBuffer, 1, 7, 12);
    CreateCylinderPrimitive(*engine->primitiveBuffer, 1, 0, 1, 12);

    // スプライト描画オブジェクトを初期化
    engine->spriteRenderer.Allocate(1000);

    // カメラのアスペクト比を設定
    Camera& camera = engine->GetCamera();
    camera.aspectRatio = engine->windowSize.x / engine->windowSize.y;
    
    // 画面のピクセル数を設定
    camera.screenSize = engine->windowSize;

    // FBOを初期化する
    engine->fboColor0.reset(new FramebufferObject(w, h, FboType::colorDepth));
    engine->fboColor1.reset(new FramebufferObject(w / 2, h / 2, FboType::color));
    engine->fboShadow.reset(new FramebufferObject(1024, 1024, FboType::depth));
    for (auto p : { engine->fboColor0.get(),
      engine->fboColor1.get(), engine->fboShadow.get() }) {
      if (!p || !p->GetId()) {
        return false;
      }
    }

    // glTFファイル用バッファを初期化
    engine->gltfFileBuffer = std::make_shared<GitfFileBuffer>(256 * 1024 * 1024);

    // ImGuiの初期化
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char glsl_version[] = "#version 450";
    ImGui_ImplOpenGL3_Init(glsl_version);

    // ImGuiの入出力を管理するオブジェクトを取得
    ImGuiIO& io = ImGui::GetIO();

    // ウィンドウ状態のセーブ機能を無効化
    io.IniFilename = nullptr;

    // デフォルトフォントを指定
    const float defaultFontPixels = 13.0f; // ImGui標準のフォントサイズ(ピクセル)
    const float fontPixels = 32.0f; // 作成するフォントのサイズ(ピクセル)
    ImFont* font = io.Fonts->AddFontFromFileTTF(
      "Res/font/Makinas-4-Flat.otf",
      fontPixels, nullptr, io.Fonts->GetGlyphRangesJapanese());
    if (font) {
      io.FontDefault = font;
      io.FontGlobalScale = defaultFontPixels / fontPixels;
      io.Fonts->Build();
    }

    // 音声を初期化する
#ifdef USE_EASY_AUDIO
    Audio::Initialize();
#else
    Audio::Initialize("Res/Audio/OpenGLGame.acf",
      CRI_OPENGLGAME_ACF_DSPSETTING_DSPBUSSETTING_0);
    Audio& audio = Audio::Get();
    audio.Load(0, "Res/Audio/MainWorkUnit/SE.acb", nullptr);
    audio.Load(1, "Res/Audio/MainWorkUnit/BGM.acb", "Res/Audio/MainWorkUnit/BGM.awb");
#endif // USE_EASY_AUDIO
  }
  return true;
}

/**
* ゲームエンジンの終了
*/
void GameEngine::Finalize()
{
  if (engine) {
    // 音声の終了.
    Audio::Finalize();

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
* 名前の一致するアクターを検索する
*/
std::shared_ptr<Actor> GameEngine::FindActor(const char* name)
{
  for (int layer = 0; layer < layerCount; ++layer) {
    for (std::shared_ptr<Actor>& e : actors[layer]) {
      if (e->name == name) {
        return e;
      }
    }
  }

  for (std::shared_ptr<Actor>& e : newActors) {
    if (e->name == name) {
      return e;
    }
  }

  return nullptr;
}

/**
* すべてのアクターを削除する
*/
void GameEngine::ClearAllActors()
{
  for (int layer = 0; layer < layerCount; ++layer) {
    actors[layer].clear();
  }
  newActors.clear();
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

      // TODO: テキスト未実装
      if (actors[i]->animation) {
        actors[i]->animation->Update(deltaTime);
      }

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
void GameEngine::UpdatePhysics(float deltaTime)
{
  ActorList& actors = GetActors(Layer::Default);

  // 接地していない状態にする
  for (int i = 0; i < actors.size(); ++i) {
    //actors[i]->contactCount = 0;
    actors[i]->isOnActor = false;
  }

  // アクターを非スタティックとスタティックに分ける
  ActorList partitionedActors = actors;
  const auto itrEndA = std::partition(
    partitionedActors.begin(), partitionedActors.end(),
    [](const ActorList::value_type& e) { return !e->isStatic; });
  const auto itrEndB = partitionedActors.end();

  std::vector<Contact> contacts;
  contacts.reserve(actors.size());
  for (auto itrA = partitionedActors.begin(); itrA != itrEndA; ++itrA) {
    std::shared_ptr<Actor>& a = *itrA;
    if (a->collisionType == CollisionType::noCollision) {
      continue;
    }
    for (auto itrB = itrA + 1; itrB != itrEndB; ++itrB) {
      std::shared_ptr<Actor>& b = *itrB;
      if (b->collisionType == CollisionType::noCollision) {
        continue;
      }

      // 削除待ちアクターは衝突しない
      if (a->isDead) {
        break;
      } else if (b->isDead) {
        continue;
      }

      Contact contact;
      if (DetectCollision(*a, *b, contact)) {
        // ブロックしないアクターが含まれる場合、接触処理を行わない
        if (a->collisionType != CollisionType::block ||
          b->collisionType != CollisionType::block) {
          a->OnTrigger(b);
          b->OnTrigger(a);
          continue; // 以降の処理をスキップ
        }

        // 配列の中に、作成したコンタクト構造体と似ているものがあるか調べる
        auto itr = std::find_if(contacts.begin(), contacts.end(),
          [&contact](const Contact& c) { return Equal(contact, c); });

        // 似ているコンタクト構造体が見つからなければ、作成した構造体を配列に追加する
        if (itr == contacts.end()) {
          contacts.push_back(contact);
        } else {
          // 似ている構造体が見つかった場合、浸透距離が長いほうを残す
          if (contact.penLength > itr->penLength) {
            *itr = contact;
          }
        }
      }
    }
  }

  // 重なりを解決する
  for (int i = 0; i < contacts.size(); ++i) {
    Contact& c = contacts[i];

    // 重なりを解決する
    SolveContact(c);

    // 衝突処理関数を呼び出す
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

  // 音声の更新
#ifdef USE_EASY_AUDIO
  Audio::Update();
#else
  Audio::Get().Update();
#endif
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
  // シェーダの切り替えによる描画効率の低下を防ぐため、アクターをシェーダ別に分ける
  std::vector<std::vector<Actor*>> shaderGroup;
  shaderGroup.resize(shaderCount);
  for (auto& e : shaderGroup) {
    e.reserve(1024);
  }
  ActorList& defaultActors = actors[static_cast<int>(Layer::Default)];
  for (auto& e : defaultActors) {
    if (e->renderer) {
      const size_t i = static_cast<int>(e->shader);
      shaderGroup[i].push_back(e.get());
    }
  }

  // 平行光源の向き
  const glm::vec3 lightDirection = glm::normalize(glm::vec4(3,-2,-2, 0));

  // NOTE: テキスト未実装
#ifdef SUPRESS_SHADOW_JITTERING
  const float worldUnitPerTexel = 100.0f / static_cast<float>(fboShadow->GetWidth());
  const float areaMin = std::floor(-50.0f / worldUnitPerTexel) * worldUnitPerTexel;
  const float areaMax = std::floor(50.0f / worldUnitPerTexel) * worldUnitPerTexel;

  // 影用ビュープロジェクション行列を作成
  const glm::mat4& matShadowProj =
    glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 200.0f);

  // 影描画時のカメラ座標を影テクスチャのピクセル幅に制限
  const glm::vec3 viewFront = -glm::normalize(glm::vec3(0, 30, 30));
  const glm::vec3 viewRight = glm::normalize(glm::cross(viewFront, glm::vec3(0, 1, 0)));
  const glm::vec3 viewUp = glm::normalize(glm::cross(viewRight, viewFront));
  glm::vec3 viewTarget = mainCamera.target;
  viewTarget = viewFront * glm::dot(viewFront, mainCamera.target);
  viewTarget += viewRight * std::floor(glm::dot(viewRight, mainCamera.target) / worldUnitPerTexel) * worldUnitPerTexel;
  viewTarget += viewUp * std::floor(glm::dot(viewUp, mainCamera.target) / worldUnitPerTexel) * worldUnitPerTexel;
#else
  // 影用ビュープロジェクション行列を作成
  const glm::mat4& matShadowProj =
    glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 200.0f);
  const glm::vec3 viewFront = -glm::normalize(glm::vec3(0, 30, 30));
  glm::vec3 viewTarget = mainCamera.target;
#endif
  const glm::vec3 viewPosition = viewTarget - viewFront * 30.0f;
  const glm::mat4 matShadowView =
    glm::lookAt(viewPosition, viewTarget, glm::vec3(0, 1, 0));

  // 影を描画
  {
    // 描画先を影描画用FBOに変更
    fboShadow->Bind();

    glEnable(GL_DEPTH_TEST); // 深度テストを有効にする
    glEnable(GL_CULL_FACE);  // 裏面カリングを有効にする
    glDisable(GL_BLEND);     // アルファブレンドを無効にする

    glClear(GL_DEPTH_BUFFER_BIT);

    sampler->Bind(0);

    // アクターを描画
    const RenderingDataList renderingList = {
      { Shader::InstancedMesh, pipelineInstancedMesh.get() },
      { Shader::FragmentLighting, pipeline.get() },
      { Shader::GroundMap, pipelineGround.get() },
      { Shader::StaticMesh, pipelineStaticMesh.get() },
    };
    RenderShaderGroups(shaderGroup, renderingList, matShadowProj * matShadowView);

    // デフォルトのフレームバッファに戻す
    fboShadow->Unbind();
  }

  // デフォルトフレームバッファのビューポートを設定
  glViewport(0, 0,
    static_cast<GLsizei>(windowSize.x),
    static_cast<GLsizei>(windowSize.y));

  // 描画先をフレームバッファオブジェクトに変更.
  fboColor0->Bind();

  glEnable(GL_DEPTH_TEST); // 深度バッファを有効にする.

  // 裏面カリングを無効にする
  // - 戦車モデルの一部が裏返っており、裏面を表示しないと本来のモデルにならない。
  // - 木や草のモデルは裏面を表示する必要がある。
  glDisable(GL_CULL_FACE);

  glClearColor(0.5f, 0.5f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  sampler->Bind(0);
  samplerShadow->Bind(1);

  const glm::mat4& matProj = mainCamera.GetProjectionMatrix();
  const glm::mat4 matView = mainCamera.GetViewMatrix();

  // NDC座標系からテクスチャ座標系への座標変換行列
  const glm::mat4 matShadowTex = glm::mat4(
    0.5f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.5f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.5f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f
  );
  const GLint locMatShadow = 100;
  const glm::mat4 matShadow = matShadowTex * matShadowProj * matShadowView;
  pipeline->SetUniform(locMatShadow, matShadow);
  pipelineGround->SetUniform(locMatShadow, matShadow);
  pipelineInstancedMesh->SetUniform(locMatShadow, matShadow);
  pipelineStaticMesh->SetUniform(locMatShadow, matShadow);
  fboShadow->BindDepthTexture(1);

  // アクターを描画する
  // 半透明メッシュ対策として、先に地面を描く
  const RenderingDataList renderingList = {
    { Shader::GroundMap, pipelineGround.get() },
    { Shader::InstancedMesh, pipelineInstancedMesh.get() },
    { Shader::FragmentLighting, pipeline.get() },
    { Shader::StaticMesh, pipelineStaticMesh.get() },
  };
  RenderShaderGroups(shaderGroup, renderingList, matProj * matView);

  // 深度テクスチャの割り当てを解除する
  fboShadow->UnbindDepthTexture(1);

  // コライダーを表示する(デバッグ用)
  if (showCollider) {
    primitiveBuffer->BindVertexArray();
    const Primitive& primBox = GetPrimitive("Collider(Box)");
    const Primitive& primSphere = GetPrimitive("Collider(Sphere)");
    const Primitive& primCylinder = GetPrimitive("Collider(Cylinder)");
    pipelineCollider->Bind();
    texCollider->Bind(0);
    for (auto& e : defaultActors) {
      switch (e->collider->GetShapeType()) {
      case ShapeType::box: {
        const Box& box = static_cast<Box&>(*e->collider);
        const glm::vec3 scale = (box.max - box.min) * 0.5f;
        const glm::vec3 offset = (box.min + box.max) * 0.5f;
        const glm::mat4 matTRS =
          glm::translate(glm::mat4(1), e->position + offset) *
          glm::scale(glm::mat4(1), scale);
        pipelineCollider->SetUniform(0, matProj * matView * matTRS);
        primBox.Draw();
        break;
      }

      case ShapeType::sphere: {
        const Sphere& sphere = static_cast<Sphere&>(*e->collider);
        const glm::vec3 scale = glm::vec3(sphere.radius);
        const glm::vec3 offset = sphere.center;
        const glm::mat4 matTRS =
          glm::translate(glm::mat4(1), e->position + offset) *
          glm::scale(glm::mat4(1), scale);
        pipelineCollider->SetUniform(0, matProj * matView * matTRS);
        primSphere.Draw();
        break;
      }

      case ShapeType::cylinder: {
        const Cylinder& cylinder = static_cast<Cylinder&>(*e->collider);
        const glm::vec3 scale =
          glm::vec3(cylinder.radius, cylinder.height, cylinder.radius);
        const glm::vec3 offset = cylinder.bottom;
        const glm::mat4 matTRS =
          glm::translate(glm::mat4(1), e->position + offset) *
          glm::scale(glm::mat4(1), scale);
        pipelineCollider->SetUniform(0, matProj * matView * matTRS);
        primCylinder.Draw();
        break;
      }
      }
    }
    texCollider->Unbind(0);
    pipelineCollider->Unbind();
  }
}

/**
* スプライトを描画する
*/
void GameEngine::RenderSprite()
{
  fboColor0->Bind();

  const glm::mat4& matProj = mainCamera.GetProjectionMatrix();
  const glm::mat4 matView = mainCamera.GetViewMatrix();
  spriteRenderer.Update(GetActors(Layer::Sprite), matView);
  spriteRenderer.Draw(pipelineUI, matProj * matView);

}

/**
* UIアクターを描画する
*/
void GameEngine::RenderUI()
{
  const Primitive& primPlane = GetPrimitive("Res/Plane.obj");

  // 縮小画像を作成
  {
    fboColor1->Bind();
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    primitiveBuffer->BindVertexArray();
    pipelineUI->Bind();
    samplerUI->Bind(0);

    // Plane.objの半径は-0.5～+0.5なので、2倍して-1～+1にする
    glm::mat4 m = glm::mat4(1);
    m[0][0] = m[1][1] = 2;
    pipelineUI->SetUniform(Renderer::locMatTRS, m);
    pipelineUI->SetUniform(Renderer::locColor, glm::vec4(1));

    fboColor0->BindColorTexture(0);
    primPlane.Draw();
  }

  // 描画先をデフォルトのフレームバッファに戻す
  fboColor0->Unbind();
  glViewport(0, 0,
    static_cast<GLsizei>(windowSize.x),
    static_cast<GLsizei>(windowSize.y));

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // FBOの内容を描画.
  {
    glDisable(GL_BLEND);

    pipelineDoF->Bind();
    samplerDoF->Bind(0);
    samplerDoF->Bind(1);
    samplerDoF->Bind(2);
    fboColor0->BindColorTexture(0);
    fboColor1->BindColorTexture(1);
    fboColor0->BindDepthTexture(2);

    glm::mat4 m = glm::mat4(1);
    m[0][0] = m[1][1] = 2;
    pipelineDoF->SetUniform(Renderer::locMatTRS, m);
    pipelineDoF->SetUniform(Renderer::locCamera, mainCamera.GetShaderParameter());
    primPlane.Draw();

    fboColor0->UnbindColorTexture(0);
    fboColor1->UnbindColorTexture(1);
    fboColor0->UnbindDepthTexture(2);
    samplerDoF->Unbind(2);
    samplerDoF->Unbind(1);
    samplerDoF->Unbind(0);
    pipelineDoF->Unbind();
  }

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
  const glm::mat4 matVP = matProj * matView;
  for (auto& actor : a) {
    if (actor->renderer) {
      actor->renderer->Draw(*actor, *pipelineUI, matVP);
    }
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

  sampler->Unbind(0);
  pipeline->Unbind();
  primitiveBuffer->UnbindVertexArray();
}

/**
* OBJファイルからプリミティブを追加する
*/
bool GameEngine::LoadPrimitive(const char* filename)
{
  // 既に同名のプリミティブが追加されていたら何もしない
  if (primitiveBuffer->Find(filename).GetName() == filename) {
    return true; // 追加済み
  }
  // まだ追加されていないのでOBJファイルを読み込む
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
* OBJファイルからメッシュとプリミティブを追加する
*/
const MeshPtr& GameEngine::LoadMesh(const char* filename)
{
  if (primitiveBuffer->Find(filename).GetName() != filename) {
    primitiveBuffer->AddFromObjFile(filename);
  }
  return primitiveBuffer->GetMesh(filename);
}

/**
* glTFファイルからメッシュを読み込む
*/
GltfFilePtr GameEngine::LoadGltfFile(const char* filename)
{
  gltfFileBuffer->AddFromFile(filename);
  return gltfFileBuffer->GetFile(filename);
}

/**
* スタティックメッシュを取得する
*/
GltfFilePtr GameEngine::GetGltfFile(const char* filename) const
{
  return gltfFileBuffer->GetFile(filename);
}

/**
* 描画データ配列に従ってアクターを描画する
*/
void GameEngine::RenderShaderGroups(const ShaderGroupList& shaderGroups,
  const RenderingDataList& renderingList, const glm::mat4& matVP)
{
  for (const RenderingData& e : renderingList) {
    // 前処理
    if (e.shaderType == Shader::InstancedMesh ||
      e.shaderType == Shader::StaticMesh) {
      e.pipeline->SetUniform(Renderer::locMatTRS, matVP);
    } else if (e.shaderType == Shader::GroundMap) {
      texMap->Bind(2);
    }

    // アクターの描画
    primitiveBuffer->BindVertexArray();
    e.pipeline->Bind();
    for (Actor* actor : shaderGroups[static_cast<size_t>(e.shaderType)]) {
      actor->renderer->Draw(*actor, *e.pipeline, matVP);
    }

    // 後処理
    if (e.shaderType == Shader::GroundMap) {
      texMap->Unbind(2);
    }
  }
}

/**
* アクターにスタティックメッシュレンダラを設定する
*
* @param actor    レンダラを設定するアクター
* @param filename glTFファイル名
* @param index    glTFに含まれるメッシュのインデックス
*/
void SetStaticMeshRenderer(Actor& actor, const char* filename, int index)
{
  auto renderer = std::make_shared<StaticMeshRenderer>();
  renderer->SetMesh(GameEngine::Get().LoadGltfFile(filename), index);
  actor.renderer = renderer;
  actor.shader = Shader::StaticMesh;
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
* 配列テクスチャを読み込む
*/
std::shared_ptr<Texture> GameEngine::LoadTexture(const char* name, const char** fileList, size_t count)
{
  TextureBuffer::iterator itr = textureBuffer.find(name);
  if (itr == textureBuffer.end()) {
    std::shared_ptr<Texture> tex(new Texture(name, fileList, count));
    textureBuffer.insert(std::make_pair(std::string(name), tex));
    return tex;
  }
  return itr->second;
}

/**
* 地面のマップデータを更新する
*/
void GameEngine::UpdateGroundMap(int x, int y, int width, int height, const void* data)
{
  if (texMap) {
    texMap->Write(x, y, width, height, data, GL_RGBA, GL_UNSIGNED_BYTE);
  }
}

/**
* 地面のマップデータの大きさを変更する
*/
void GameEngine::ResizeGroundMap(int width, int height, const void* data)
{
  mapSize = glm::ivec2(width, height);
  texMap.reset(new Texture("GroundMap",
    mapSize.x, mapSize.y, data, GL_RGBA, GL_UNSIGNED_BYTE));

  const GLint locMapSize = 101;
  pipelineGround->SetUniform(locMapSize, glm::vec4(mapSize, 0, 0));
}


// TODO: 以下はテキスト未実装の関数

/**
* マウス座標を取得
*/
glm::vec2 GameEngine::GetMousePosition() const
{
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  return glm::vec2(x, y);
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
  return random();
}

