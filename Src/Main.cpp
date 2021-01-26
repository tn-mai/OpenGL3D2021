/**
* @file Main.cpp
*/
#include <glad/glad.h>
#include "GLContext.h"
#include "GameData.h"
#include "SceneManager.h"
#include "Audio.h"
#include "Audio/OpenGL3D2020_acf.h"
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

#pragma comment(lib, "opengl32.lib")

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
*
* 詳細は(https://www.khronos.org/opengl/wiki/Debug_Output)を参照.
*/
void GLAPIENTRY DebugCallback(GLenum source, GLenum type, GLuint id,
  GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
  if (length < 0) {
    std::cerr << "[MSG] " << message << "\n";
  } else {
    const std::string s(message, message + length);
    std::cerr << "[MSG] " << s << "\n";
  }
}

/**
* GLFWからのエラー報告を処理する.
*
* @param error エラー番号.
* @param desc  エラーの内容.
*/
void ErrorCallback(int error, const char* desc)
{
  std::cerr << "ERROR: " << desc << "\n";
}

/**
* エントリーポイント.
*/
int main()
{
  glfwSetErrorCallback(ErrorCallback);

  // GLFWの初期化.
  if (glfwInit() != GLFW_TRUE) {
    return 1;
  }

  // 描画ウィンドウの作成.
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
  //glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
  GLFWwindow* window =
    glfwCreateWindow(1280, 720, "OpenGL3DActionGame", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);

  // OpenGL関数のアドレスを取得する.
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::cerr << "ERROR: GLADの初期化に失敗しました." << std::endl;
    glfwTerminate();
    return 1;
  }

  glDebugMessageCallback(DebugCallback, nullptr);

  // OpenGLの情報をコンソールウィンドウへ出力する.
  {
#define Output(name) {\
  GLint n = 0;\
  glGetIntegerv(name, &n);\
  std::cout << #name " = " << n << "\n";\
} (void)0
    const GLubyte* renderer = glGetString(GL_RENDERER);
    std::cout << "Renderer: " << renderer << "\n";
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " << version << "\n";
    Output(GL_MAX_VERTEX_ATTRIB_BINDINGS); // 少なくとも16.
    Output(GL_MAX_VERTEX_ATTRIBS); // 少なくとも16.
    Output(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET); // 少なくとも2047
    Output(GL_MAX_VERTEX_ATTRIB_STRIDE); // 少なくとも2048
    Output(GL_MAX_UNIFORM_LOCATIONS); // 少なくとも1024
    Output(GL_MAX_UNIFORM_BLOCK_SIZE); // 少なくとも16k
    Output(GL_MAX_UNIFORM_BUFFER_BINDINGS); // 少なくとも36
    Output(GL_MAX_VERTEX_UNIFORM_COMPONENTS); // 少なくとも1024
    Output(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS); // 少なくとも1024
    Output(GL_MAX_VARYING_COMPONENTS); // 少なくとも60
    Output(GL_MAX_VARYING_VECTORS); // 少なくとも15
    Output(GL_MAX_VARYING_FLOATS); // 少なくとも32
    Output(GL_MAX_VERTEX_ATTRIBS); // 少なくとも16
    Output(GL_MAX_TEXTURE_SIZE); // 少なくとも1024
    Output(GL_MAX_TEXTURE_IMAGE_UNITS); // 少なくとも16
    Output(GL_MAX_TEXTURE_BUFFER_SIZE); // 少なくとも64k
    Output(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS); // 少なくとも8
    Output(GL_MAX_SHADER_STORAGE_BLOCK_SIZE); // 少なくとも16MB
#undef Output
  }

  // オーディオ初期化.
  Audio& audio = Audio::Instance();
  audio.Initialize("Res/Audio/OpenGL3D2020.acf", CRI_OPENGL3D2020_ACF_DSPSETTING_DSPBUSSETTING_0);
  audio.Load(0, "Res/Audio/MainWorkUnit/SE.acb", nullptr);
  audio.Load(1, "Res/Audio/MainWorkUnit/BGM.acb", "Res/Audio/MainWorkUnit/BGM.awb");

  // ゲーム全体で使うデータを初期化する.
  GameData& gamedata = GameData::Get();
  if (!gamedata.Initialize(window)) {
    return 1;
  }

  // 最初に実行するシーンを指定する.
  SceneManager& sceneManager = SceneManager::Get();
  sceneManager.ChangeScene(TITLE_SCENE_NAME);

  // 経過時間計測開始.
  double elapsedTime = glfwGetTime();

  // メインループ.
  while (!glfwWindowShouldClose(window)) {
    // 経過時間を計測.
    const double newElapsedTime = glfwGetTime();
    float deltaTime = static_cast<float>(newElapsedTime - elapsedTime);
    if (deltaTime >= 0.1f) {
      deltaTime = 1.0f / 60.0f;
    }
    elapsedTime = newElapsedTime;

    // ゲームデータの状態を更新.
    gamedata.Update();

    sceneManager.Update(window, deltaTime);
    sceneManager.Render(window);

    // 音声の更新
    audio.Update();

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  sceneManager.Finalize();

  // 音声の終了.
  audio.Finalize();

  // GLFWの終了.
  glfwTerminate();

  return 0;
}