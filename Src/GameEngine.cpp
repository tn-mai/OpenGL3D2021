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

    // �J�����̃A�X�y�N�g���ݒ�
    Camera& camera = engine->GetCamera();
    camera.aspectRatio = engine->windowSize.x / engine->windowSize.y;

    // ImGui�̏�����
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char glsl_version[] = "#version 450";
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImGuiIO& io = ImGui::GetIO();

    // �E�B���h�E��Ԃ̃Z�[�u�@�\�𖳌���
    io.IniFilename = nullptr;

    // GUI�̑傫����ݒ�
    const float guiScale = 1.5f;
    ImGui::GetStyle().ScaleAllSizes(guiScale);

    // �f�t�H���g�t�H���g���w��
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
* �Q�[���G���W���̏I��
*/
void GameEngine::Finalize()
{
  if (engine) {
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

      actors[i]->OnUpdate(deltaTime);

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
void GameEngine::UpdatePysics(float deltaTime)
{
  ActorList& actors = GetActors(Layer::Default);

  // �ڒn���Ă��Ȃ���Ԃɂ���
  for (int i = 0; i < actors.size(); ++i) {
    actors[i]->contactCount = 0;
    actors[i]->isOnActor = false;
  }

#if 1
  // ��X�^�e�B�b�N�ȃA�N�^�[�����X�g�A�b�v
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
      // �����A�N�^�[�͏Փ˂��Ȃ�
      if (a == b) {
        continue;
      }

      // �폜�҂��A�N�^�[�͏Փ˂��Ȃ�
      if (a->isDead) {
        break;
      } else if (b->isDead) {
        continue;
      }

      Contact contact[2];
      if (DetectCollision(*a, *b, contact[0])) {
        contact[1] = Reverse(contact[0]);
        // �z��̒��ɁA�쐬�����R���^�N�g�\���̂Ǝ��Ă�����̂����邩���ׂ�
        for (const auto& e : contact) {
          auto itr = std::find_if(contacts.begin(), contacts.end(),
            [&e](const Contact& c) { return Equal2(e, c); });

          // ���Ă���R���^�N�g�\���̂�������Ȃ���΁A�쐬�����\���̂�z��ɒǉ�����
          if (itr == contacts.end()) {
            contacts.push_back(e);
            ++e.a->contactCount;
          } else {
            // ���Ă���\���̂����������ꍇ�A�Z�������������ق����c��
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

  // �d�Ȃ����������
  for (int i = 0; i < contacts.size(); ++i) {
    Contact& c = contacts[i];

    // �d�Ȃ����������
    if (!c.a->isStatic) {
      SolveContact(c);
    }

    // �Փˏ����֐����Ăяo��
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
  glEnable(GL_DEPTH_TEST); // �[�x�o�b�t�@��L���ɂ���.
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

  // �A�N�^�[��`�悷��
  const int layer = static_cast<int>(Layer::Default);
  ActorList& defaultActors = actors[layer];
  for (int i = 0; i < defaultActors.size(); ++i) {
    Draw(*defaultActors[i], *pipeline, matProj, matView);
  }
}

/**
* UI�A�N�^�[��`�悷��
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
* �`��̌�n��������
*/
void GameEngine::PostRender()
{
  // �e�N�X�`���̊��蓖�Ă�����.
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
  return rg();
}

