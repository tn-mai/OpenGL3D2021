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
* ������(���C��)���쐬(�f�o�b�O�p)
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
* �~��(���C��)���쐬(�f�o�b�O�p)
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
* �Q�[���G���W���̏�����
*/
bool GameEngine::Initialize()
{
  if (!engine) {
    engine = new GameEngine;
    if (!engine) {
      return false;
    }

    // GLFW�̏�����.
    if (glfwInit() != GLFW_TRUE) {
      return false;
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
      return false;
    }

    glDebugMessageCallback(DebugCallback, nullptr);

    engine->window = window;
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    engine->windowSize = glm::vec2(w, h);

    // ����������������
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
    engine->pipelineAnimatedMesh.reset(new ProgramPipeline(
      "Res/AnimatedMesh.vert", "Res/StaticMesh.frag"));

    // �u���[���G�t�F�N�g�p�̃V�F�[�_������������
    const struct {
      std::shared_ptr<ProgramPipeline>& p;
      const char* fragName;
    } bloomShaders[] = {
      { engine->pipelineBrightnessFilter, "Res/BrightnessFilter.frag" },
      { engine->pipelineDownsample,  "Res/Downsample.frag" },
      { engine->pipelineUpsample, "Res/Upsample.frag" },

      { engine->pipelineSAO, "Res/ScalableAmbientObscurance.frag" },
    };
    for (const auto [p, fragName] : bloomShaders) {
      p.reset(new ProgramPipeline("Res/Simple.vert", fragName));
      // Plane.obj�̔��a��-0.5�`+0.5�Ȃ̂ŁA2�{����-1�`+1�ɂ���
      glm::mat4 m = glm::mat4(1);
      m[0][0] = m[1][1] = 2;
      p->SetUniform(Renderer::locMatTRS, m);
    }

    engine->sampler = std::shared_ptr<Sampler>(new Sampler(GL_REPEAT));
    engine->samplerUI.reset(new Sampler(GL_CLAMP_TO_EDGE));
    engine->samplerDoF.reset(new Sampler(GL_CLAMP_TO_EDGE));

    engine->samplerShadow.reset(new Sampler(GL_CLAMP_TO_EDGE));

    // �n�ʃ}�b�v�p�f�[�^���쐬
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

    // �R���C�_�[�\���p�f�[�^���쐬(�f�o�b�O�p)
    engine->pipelineCollider = engine->pipelineUI; // UI�V�F�[�_�𗬗p
    const std::vector<uint32_t> imageWhite(4 * 4, 0xff1080f0);
    engine->texCollider = std::make_shared<Texture>(
      "Debug(Collider)", 4, 4, imageWhite.data(), GL_RGBA, GL_UNSIGNED_BYTE);
    CreateBoxPrimitive(*engine->primitiveBuffer, glm::vec3(-1.0f), glm::vec3(1.0f));
    CreateSpherePrimitive(*engine->primitiveBuffer, 1, 7, 12);
    CreateCylinderPrimitive(*engine->primitiveBuffer, 1, 0, 1, 12);

    // �X�v���C�g�`��I�u�W�F�N�g��������
    engine->spriteRenderer.Allocate(1000);

    // �J�����̃A�X�y�N�g���ݒ�
    Camera& camera = engine->GetCamera();
    camera.aspectRatio = engine->windowSize.x / engine->windowSize.y;
    
    // ��ʂ̃s�N�Z������ݒ�
    camera.screenSize = engine->windowSize;

    // FBO������������
    engine->fboColor0.reset(new FramebufferObject(w, h, FboType::colorDepth));
    engine->fboColor1.reset(new FramebufferObject(w / 2, h / 2, FboType::color));
    engine->fboShadow.reset(new FramebufferObject(1024, 1024, FboType::depth));
    for (auto p : { engine->fboColor0.get(),
      engine->fboColor1.get(), engine->fboShadow.get() }) {
      if (!p || !p->GetId()) {
        return false;
      }
    }

    // �u���[���pFBO������������
    int bloomW = w;
    int bloomH = h;
    for (auto& p : engine->fboBloom) {
      p.reset(new FramebufferObject(bloomW, bloomH, FboType::color));
      if (!p || !p->GetId()) {
        return false;
      }
      bloomW /= 2;
      bloomH /= 2;
    }

    // glTF�t�@�C���p�o�b�t�@��������
    const size_t maxAnimeActorCount = 64;   // �A�j���[�V��������A�N�^�[��
    const size_t maxAnimeMatrixCount = 256; // �A�N�^�[�̃{�[����
    engine->gltfFileBuffer = std::make_shared<GltfFileBuffer>(256 * 1024 * 1024,
      sizeof(glm::mat4) * maxAnimeActorCount * maxAnimeMatrixCount);

    // ImGui�̏�����
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char glsl_version[] = "#version 450";
    ImGui_ImplOpenGL3_Init(glsl_version);

    // ImGui�̓��o�͂��Ǘ�����I�u�W�F�N�g���擾
    ImGuiIO& io = ImGui::GetIO();

    // �E�B���h�E��Ԃ̃Z�[�u�@�\�𖳌���
    io.IniFilename = nullptr;

    // �f�t�H���g�t�H���g���w��
    const float defaultFontPixels = 13.0f; // ImGui�W���̃t�H���g�T�C�Y(�s�N�Z��)
    const float fontPixels = 32.0f; // �쐬����t�H���g�̃T�C�Y(�s�N�Z��)
    ImFont* font = io.Fonts->AddFontFromFileTTF(
      "Res/font/Makinas-4-Flat.otf",
      fontPixels, nullptr, io.Fonts->GetGlyphRangesJapanese());
    if (font) {
      io.FontDefault = font;
      io.FontGlobalScale = defaultFontPixels / fontPixels;
      io.Fonts->Build();
    }

    // ����������������
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
* �Q�[���G���W���̏I��
*/
void GameEngine::Finalize()
{
  if (engine) {
    // �����̏I��.
    Audio::Finalize();

    // GUI�̏I��
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // GLFW�̏I��.
    glfwTerminate();

    delete engine;
    engine = nullptr;
  }
}

/**
* �Q�[���G���W�����擾����
*/
GameEngine& GameEngine::Get()
{
  return *engine;
}

/**
* ���O�̈�v����A�N�^�[����������
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
* ���ׂẴA�N�^�[���폜����
*/
void GameEngine::ClearAllActors()
{
  for (int layer = 0; layer < layerCount; ++layer) {
    actors[layer].clear();
  }
  newActors.clear();
}

/**
* �Q�[���G���W�����X�V����
*/
void GameEngine::UpdateActors(float deltaTime)
{
  for (int layer = 0; layer < layerCount; ++layer) {
    ActorList& actors = this->actors[layer];
    // �ȑO�̑��x���X�V
    for (int i = 0; i < actors.size(); ++i) {
      actors[i]->oldVelocity = actors[i]->velocity;
    }

    // �A�N�^�[�̏�Ԃ��X�V����
    for (int i = 0; i < actors.size(); ++i) {
      // �A�N�^�[�̎��������炷
      if (actors[i]->lifespan > 0) {
        actors[i]->lifespan -= deltaTime;

        // �����̐s�����A�N�^�[���u�폜�҂��v��Ԃɂ���
        if (actors[i]->lifespan <= 0) {
          actors[i]->isDead = true;
          continue; // �폜�҂��A�N�^�[�͍X�V���X�L�b�v
        }
      }

      // �A�j���[�V���������s
      if (actors[i]->animation) {
        actors[i]->animation->Update(deltaTime);
      }

      actors[i]->OnUpdate(deltaTime);

      // �����_���̏�Ԃ��X�V
      if (actors[i]->renderer) {
        actors[i]->renderer->Update(*actors[i], deltaTime);
      }

      // ���x�ɏd�͉����x��������
      if (!actors[i]->isStatic) {
        actors[i]->velocity.y += -9.8f * deltaTime * actors[i]->gravityScale;
      }

      // �A�N�^�[�̈ʒu���X�V����
      actors[i]->position += actors[i]->velocity * deltaTime;
    }
  }
}

/**
* �Q�[���G���W�����X�V����(�㏈��)
*/
void GameEngine::PostUpdateActors()
{
  // �V�K�ɍ쐬���ꂽ�A�N�^�[���A�N�^�[�z��ɒǉ�����
  for (int i = 0; i < newActors.size(); ++i) {
    const int layer = static_cast<int>(newActors[i]->layer);
    if (layer >= 0 && layer < layerCount) {
      actors[layer].push_back(newActors[i]);
    }
  }

  // �V�K�A�N�^�[�z�����ɂ���
  newActors.clear();
}

/**
* �A�N�^�[�̏Փ˔�����s��
*/
void GameEngine::UpdatePhysics(float deltaTime)
{
  ActorList& actors = GetActors(Layer::Default);

  // �ڒn���Ă��Ȃ���Ԃɂ���
  for (int i = 0; i < actors.size(); ++i) {
    //actors[i]->contactCount = 0;
    actors[i]->isOnActor = false;
  }

  // �A�N�^�[���X�^�e�B�b�N�ƃX�^�e�B�b�N�ɕ�����
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

      // �폜�҂��A�N�^�[�͏Փ˂��Ȃ�
      if (a->isDead) {
        break;
      } else if (b->isDead) {
        continue;
      }

      Contact contact;
      if (DetectCollision(*a, *b, contact)) {
        // �u���b�N���Ȃ��A�N�^�[���܂܂��ꍇ�A�ڐG�������s��Ȃ�
        if (a->collisionType != CollisionType::block ||
          b->collisionType != CollisionType::block) {
          a->OnTrigger(b);
          b->OnTrigger(a);
          continue; // �ȍ~�̏������X�L�b�v
        }

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
      }
    }
  }

  // �d�Ȃ����������
  for (int i = 0; i < contacts.size(); ++i) {
    Contact& c = contacts[i];

    // �d�Ȃ����������
    SolveContact(c);

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
  }
}

/**
* �J�����̏�Ԃ��X�V����
*/
void GameEngine::UpdateCamera()
{
  mainCamera.Update();
}

/**
* �V�����t���[�����J�n����
*/
void GameEngine::NewFrame()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  // �����̍X�V
#ifdef USE_EASY_AUDIO
  Audio::Update();
#else
  Audio::Get().Update();
#endif

  gltfFileBuffer->ClearAnimationBuffer();
}

/**
* �폜�҂��̃A�N�^�[���폜����
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
* �f�t�H���g�A�N�^�[��`�悷��
*/
void GameEngine::RenderDefault()
{
  // �����_���̑O���������s
  for (auto& e : actors[static_cast<int>(Layer::Default)]) {
    if (e->renderer) {
      e->renderer->PreDraw(*e);
    }
  }

  // �A�j���[�V�����o�b�t�@��GPU�������ɃR�s�[����
  gltfFileBuffer->UploadAnimationBuffer();

  // �V�F�[�_�̐؂�ւ��ɂ��`������̒ቺ��h�����߁A�A�N�^�[���V�F�[�_�ʂɕ�����
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

  // ���s�����̌���
  const glm::vec3 lightDirection = glm::normalize(glm::vec4(3,-2,-2, 0));

  // NOTE: �e�L�X�g������
#ifdef SUPRESS_SHADOW_JITTERING
  const float S = 50;
  const float worldUnitPerTexel = (S * 2) / static_cast<float>(fboShadow->GetWidth());
  const float areaMin = std::floor(-S / worldUnitPerTexel) * worldUnitPerTexel;
  const float areaMax = std::floor(S / worldUnitPerTexel) * worldUnitPerTexel;

  // �e�p�r���[�v���W�F�N�V�����s����쐬
  const glm::mat4& matShadowProj =
    glm::ortho(-S, S, -S, S, 1.0f, 400.0f);

  // �e�`�掞�̃J�������W���e�e�N�X�`���̃s�N�Z�����ɐ���
  const glm::vec3 viewFront = directionalLight.direction;
  const glm::vec3 viewRight = glm::normalize(glm::cross(viewFront, glm::vec3(0, 1, 0)));
  const glm::vec3 viewUp = glm::normalize(glm::cross(viewRight, viewFront));
  glm::vec3 viewTarget = mainCamera.target;
  viewTarget = viewFront * glm::dot(viewFront, mainCamera.target);
  viewTarget += viewRight * std::floor(glm::dot(viewRight, mainCamera.target) / worldUnitPerTexel) * worldUnitPerTexel;
  viewTarget += viewUp * std::floor(glm::dot(viewUp, mainCamera.target) / worldUnitPerTexel) * worldUnitPerTexel;
#else
  // �e�p�r���[�v���W�F�N�V�����s����쐬
  const glm::mat4& matShadowProj =
    glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 200.0f);
  const glm::vec3 viewFront = directionalLight.direction;
  glm::vec3 viewTarget = mainCamera.target;
#endif
  const glm::vec3 viewPosition = viewTarget - directionalLight.direction * 100.0f;
  const glm::mat4 matShadowView =
    glm::lookAt(viewPosition, viewTarget, glm::vec3(0, 1, 0));

  // �e��`��
  {
    // �`�����e�`��pFBO�ɕύX
    fboShadow->Bind();

    glEnable(GL_DEPTH_TEST); // �[�x�e�X�g��L���ɂ���
    glEnable(GL_CULL_FACE);  // ���ʃJ�����O��L���ɂ���
    glDisable(GL_BLEND);     // �A���t�@�u�����h�𖳌��ɂ���

    glClear(GL_DEPTH_BUFFER_BIT);

    sampler->Bind(0);

    // �A�N�^�[��`��
    const RenderingDataList renderingList = {
      { Shader::InstancedMesh, pipelineInstancedMesh.get() },
      { Shader::FragmentLighting, pipeline.get() },
      { Shader::GroundMap, pipelineGround.get() },
      { Shader::StaticMesh, pipelineStaticMesh.get() },
      { Shader::AnimatedMesh, pipelineAnimatedMesh.get() },
    };
    RenderShaderGroups(shaderGroup, renderingList, matShadowProj * matShadowView);

    // �f�t�H���g�̃t���[���o�b�t�@�ɖ߂�
    fboShadow->Unbind();
  }

  // �f�t�H���g�t���[���o�b�t�@�̃r���[�|�[�g��ݒ�
  glViewport(0, 0,
    static_cast<GLsizei>(windowSize.x),
    static_cast<GLsizei>(windowSize.y));

  // �`�����t���[���o�b�t�@�I�u�W�F�N�g�ɕύX.
  fboColor0->Bind();

  glEnable(GL_DEPTH_TEST); // �[�x�o�b�t�@��L���ɂ���.

  // ���ʃJ�����O�𖳌��ɂ���
  // - ��ԃ��f���̈ꕔ�����Ԃ��Ă���A���ʂ�\�����Ȃ��Ɩ{���̃��f���ɂȂ�Ȃ��B
  // - �؂⑐�̃��f���͗��ʂ�\������K�v������B
  glDisable(GL_CULL_FACE);

  glClearColor(0.5f, 0.5f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  sampler->Bind(0);
  samplerShadow->Bind(1);

  const glm::mat4& matProj = mainCamera.GetProjectionMatrix();
  const glm::mat4 matView = mainCamera.GetViewMatrix();

  // NDC���W�n����e�N�X�`�����W�n�ւ̍��W�ϊ��s��
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

  // �V�F�[�_�Ƀ��C�g��ݒ�
  const ProgramPipeline* pipelineListForLight[] = {
    pipeline.get(),
    pipelineGround.get(),
    pipelineInstancedMesh.get(),
    pipelineStaticMesh.get(),
    pipelineAnimatedMesh.get(),
  };
  for (auto* e : pipelineListForLight) {
    e->SetUniform(Renderer::locDirectionalLight + 0, directionalLight.direction);
    e->SetUniform(Renderer::locDirectionalLight + 1, directionalLight.color);
    e->SetUniform(Renderer::locAmbientLight, ambientLight);
  }

  // �V�F�[�_�ɃJ�������W��ݒ�
  const glm::vec4 cameraPosition(mainCamera.position, 1);
  pipeline->SetUniform(Renderer::locCameraPosition, cameraPosition);
  pipelineStaticMesh->SetUniform(Renderer::locCameraPosition, cameraPosition);
  pipelineAnimatedMesh->SetUniform(Renderer::locCameraPosition, cameraPosition);
  pipelineInstancedMesh->SetUniform(Renderer::locCameraPosition, cameraPosition);

  // �A�N�^�[��`�悷��
  // ���������b�V���΍�Ƃ��āA��ɒn�ʂ�`��
  const RenderingDataList renderingList = {
    { Shader::GroundMap, pipelineGround.get() },
    { Shader::InstancedMesh, pipelineInstancedMesh.get() },
    { Shader::FragmentLighting, pipeline.get() },
    { Shader::StaticMesh, pipelineStaticMesh.get() },
    { Shader::AnimatedMesh, pipelineAnimatedMesh.get() },
  };
  RenderShaderGroups(shaderGroup, renderingList, matProj * matView);

  // �[�x�e�N�X�`���̊��蓖�Ă���������
  fboShadow->UnbindDepthTexture(1);

  RenderAmbientOcclusion();

  // �R���C�_�[��\������(�f�o�b�O�p)
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
* �X�v���C�g��`�悷��
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
* �A���r�G���g�I�N���[�W������`�悷��
*/
void GameEngine::RenderAmbientOcclusion()
{
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  //glDisable(GL_BLEND);
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
  glBlendFunc(GL_CONSTANT_COLOR, GL_ONE);
  glBlendColor(ambientLight.r, ambientLight.g, ambientLight.b, 1);

  primitiveBuffer->BindVertexArray();
  pipelineSAO->Bind();
  samplerUI->Bind(0);

  // AO����p�p�����[�^�����j�t�H�[���ϐ��ɃR�s�[
  const float radius = 1; // ���[���h���W�n�ɂ�����AO�̃T���v�����O���a(���[�g��)
  const float scale = 1; // �u�[�x�l�̎�肤��͈́v�A�uAO�����������������v�ɂ���Ē���
  const float intensity = 2; // AO�̋���
  pipelineSAO->SetUniform(Renderer::locRadiusScaleIntensity,
    glm::vec4(radius, mainCamera.zFar * scale, intensity / pow(radius, 6.0f), 0));

  // �J�����p�����[�^�����j�t�H�[���ϐ��ɃR�s�[
  pipelineSAO->SetUniform(Renderer::locCamera,
    glm::vec4(1.0f / mainCamera.screenSize, mainCamera.zNear, mainCamera.zFar));

  // �t�v���W�F�N�V�����s������j�t�H�[���ϐ��ɃR�s�[
  const glm::mat4& matProj = mainCamera.GetProjectionMatrix();
  pipelineSAO->SetUniform(Renderer::locMatInvProj, glm::inverse(matProj));

  // AO��`��
  fboColor0->BindDepthTexture(0); // �[�x�e�N�X�`�����o�C���h
  const Primitive& primPlane = GetPrimitive("Res/Plane.obj");
  primPlane.Draw();

  // �u�����h�֐��̐ݒ���f�t�H���g�ɖ߂�
  glBlendEquation(GL_FUNC_ADD);
  glBlendColor(0, 0, 0, 1);
}

/**
* �|�X�g�G�t�F�N�g��`�悷��
*/
void GameEngine::RenderPostEffect()
{
  const Primitive& primPlane = GetPrimitive("Res/Plane.obj");

  /* ��ʊE�[�x */

  // �k���摜���쐬
  {
    fboColor1->Bind();
    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    primitiveBuffer->BindVertexArray();
    pipelineUI->Bind();
    samplerUI->Bind(0);

    // Plane.obj�̔��a��-0.5�`+0.5�Ȃ̂ŁA2�{����-1�`+1�ɂ���
    glm::mat4 m = glm::mat4(1);
    m[0][0] = m[1][1] = 2;
    pipelineUI->SetUniform(Renderer::locMatTRS, m);
    pipelineUI->SetUniform(Renderer::locColor, glm::vec4(1));

    fboColor0->BindColorTexture(0);
    primPlane.Draw();
  }

  // �`�������̃|�X�g�G�t�F�N�g�pFBO�ɐݒ�
  fboBloom[0]->Bind();
  glClear(GL_COLOR_BUFFER_BIT);

  // ��ʊE�[�x�t����FBO�̓��e��`��
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

  /* �u���[���G�t�F�N�g */
  {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    samplerUI->Bind(0);

    // ���邢�����𒊏o
    pipelineBrightnessFilter->Bind();
    fboBloom[1]->Bind();
    fboBloom[0]->BindColorTexture(0);
    primPlane.Draw();

    // �k���摜���쐬
    pipelineDownsample->Bind();
    for (size_t i = 1; i < std::size(fboBloom) - 1; ++i) {
      fboBloom[i + 1]->Bind();
      fboBloom[i]->BindColorTexture(0);
      primPlane.Draw();
    }

    // �k���摜���t���ō���
    pipelineUpsample->Bind();
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    for (size_t i = std::size(fboBloom) - 1; i > 1; --i) {
      fboBloom[i - 1]->Bind();
      fboBloom[i]->BindColorTexture(0);
      primPlane.Draw();
    }

    // �u���[���̖��邳�𒲐����Č��T�C�Y�ƍ���
    const float bloomScale = 1.0f;
    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
    glBlendColor(0, 0, 0, bloomScale);
    fboBloom[0]->Bind();
    fboBloom[1]->BindColorTexture(0);
    primPlane.Draw();

    // �`�挋�ʂ��f�t�H���g�̃t���[���o�b�t�@�ɏ����o��
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0,
      static_cast<GLsizei>(windowSize.x),
      static_cast<GLsizei>(windowSize.y));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    pipelineUI->Bind();
    fboBloom[0]->BindColorTexture(0);
    primPlane.Draw();
  } // �u���[���G�t�F�N�g
}

/**
* UI�A�N�^�[��`�悷��
*/
void GameEngine::RenderUI()
{
  const Primitive& primPlane = GetPrimitive("Res/Plane.obj");

  // �`�����f�t�H���g�̃t���[���o�b�t�@�ɖ߂�
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0,
    static_cast<GLsizei>(windowSize.x),
    static_cast<GLsizei>(windowSize.y));

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  primitiveBuffer->BindVertexArray();
  pipelineUI->Bind();
  samplerUI->Bind(0);

  // �v���W�F�N�V�����s����쐬.
  const glm::vec2 halfSize = windowSize * 0.5f;
  const glm::mat4 matProj =
    glm::ortho(-halfSize.x, halfSize.x, -halfSize.y, halfSize.y, 1.0f, 200.0f);

  // �r���[�s����쐬.
  const glm::mat4 matView =
    glm::lookAt(glm::vec3(0, 0, 100), glm::vec3(0), glm::vec3(0, 1, 0));

  // Z���W�̍~����2D�A�N�^�[��`�悷��
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
* �`��̌�n��������
*/
void GameEngine::PostRender()
{
  // �e�N�X�`���̊��蓖�Ă�����.
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);

  sampler->Unbind(0);
  pipeline->Unbind();
  primitiveBuffer->UnbindVertexArray();
}

/**
* OBJ�t�@�C������v���~�e�B�u��ǉ�����
*/
bool GameEngine::LoadPrimitive(const char* filename)
{
  // ���ɓ����̃v���~�e�B�u���ǉ�����Ă����牽�����Ȃ�
  if (primitiveBuffer->Find(filename).GetName() == filename) {
    return true; // �ǉ��ς�
  }
  // �܂��ǉ�����Ă��Ȃ��̂�OBJ�t�@�C����ǂݍ���
  return primitiveBuffer->AddFromObjFile(filename);
}

/**
* ���O�̈�v����v���~�e�B�u���擾����
*
* @param filename �v���~�e�B�u��
*
* @return filename�Ɩ��O����v����v���~�e�B�u
*/
const Primitive& GameEngine::GetPrimitive(const char* filename) const
{
  return primitiveBuffer->Find(filename);
}

/**
* OBJ�t�@�C�����烁�b�V���ƃv���~�e�B�u��ǉ�����
*/
const MeshPtr& GameEngine::LoadMesh(const char* filename)
{
  if (primitiveBuffer->Find(filename).GetName() != filename) {
    primitiveBuffer->AddFromObjFile(filename);
  }
  return primitiveBuffer->GetMesh(filename);
}

/**
* glTF�t�@�C�����烁�b�V����ǂݍ���
*/
GltfFilePtr GameEngine::LoadGltfFile(const char* filename)
{
  gltfFileBuffer->AddFromFile(filename);
  return gltfFileBuffer->GetFile(filename);
}

/**
* �X�^�e�B�b�N���b�V�����擾����
*/
GltfFilePtr GameEngine::GetGltfFile(const char* filename) const
{
  return gltfFileBuffer->GetFile(filename);
}

/**
* �`��f�[�^�z��ɏ]���ăA�N�^�[��`�悷��
*/
void GameEngine::RenderShaderGroups(const ShaderGroupList& shaderGroups,
  const RenderingDataList& renderingList, const glm::mat4& matVP)
{
  for (const RenderingData& e : renderingList) {
    // �O����
    if (e.shaderType == Shader::InstancedMesh ||
      e.shaderType == Shader::StaticMesh ||
      e.shaderType == Shader::AnimatedMesh) {
      e.pipeline->SetUniform(Renderer::locMatTRS, matVP);
    } else if (e.shaderType == Shader::GroundMap) {
      texMap->Bind(2);
    }

    // �A�N�^�[�̕`��
    primitiveBuffer->BindVertexArray();
    e.pipeline->Bind();
    for (Actor* actor : shaderGroups[static_cast<size_t>(e.shaderType)]) {
      actor->renderer->Draw(*actor, *e.pipeline, matVP);
    }

    // �㏈��
    if (e.shaderType == Shader::GroundMap) {
      texMap->Unbind(2);
    }
  }
}

/**
* �e�N�X�`����ǂݍ���
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
* �z��e�N�X�`����ǂݍ���
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
* �n�ʂ̃}�b�v�f�[�^���X�V����
*/
void GameEngine::UpdateGroundMap(int x, int y, int width, int height, const void* data)
{
  if (texMap) {
    texMap->Write(x, y, width, height, data, GL_RGBA, GL_UNSIGNED_BYTE);
  }
}

/**
* �n�ʂ̃}�b�v�f�[�^�̑傫����ύX����
*/
void GameEngine::ResizeGroundMap(int width, int height, const void* data)
{
  mapSize = glm::ivec2(width, height);
  texMap.reset(new Texture("GroundMap",
    mapSize.x, mapSize.y, data, GL_RGBA, GL_UNSIGNED_BYTE));

  const GLint locMapSize = 101;
  pipelineGround->SetUniform(locMapSize, glm::vec4(mapSize, 0, 0));
}


// TODO: �ȉ��̓e�L�X�g�������̊֐�

/**
* �}�E�X���W���擾
*/
glm::vec2 GameEngine::GetMousePosition() const
{
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  return glm::vec2(x, y);
}

/**
* �e�N�X�`�����擾����
*
* @param filename �e�N�X�`���t�@�C����
*
* @return filename����쐬���ꂽ�e�N�X�`��
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
* �������擾����
*/
unsigned int GameEngine::GetRandom()
{
  return random();
}

