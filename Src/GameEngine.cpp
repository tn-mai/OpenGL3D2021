/**
* @file GameEngine.cpp
*/
#include "GameEngine.h"

namespace {

GameEngine* engine = nullptr;

}

/**
* ゲームエンジンの初期化
*/
bool GameEngine::Initialize()
{
  if (!engine) {
    engine = new GameEngine;
  }

  engine->actors.reserve(1000);
  engine->newActors.reserve(1000);

  engine->primitiveBuffer.reset(new PrimitiveBuffer(1'000'000, 4'000'000));

  engine->textureBuffer.reserve(1000);

  std::random_device rd;
  engine->rg.seed(rd());

  return true;
}

/**
* ゲームエンジンの終了
*/
void GameEngine::Finalize()
{
  delete engine;
  engine = nullptr;
}

/**
* ゲームエンジンを取得する
*/
GameEngine& GameEngine::Get()
{
  return *engine;
}

/**
* ゲームエンジンを更新する
*/
void GameEngine::UpdateActors()
{
  // 新規に作成されたアクターをアクター配列に追加する
  for (int i = 0; i < newActors.size(); ++i) {
    actors.push_back(newActors[i]);
  }

  // 新規アクター配列を空にする
  newActors.clear();
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
    static std::shared_ptr<Texture> tex(new Texture(""));
    return tex;
  }
  return itr->second;
}

bool GameEngine::LoadPrimitive(const char* filename)
{
  return primitiveBuffer->AddFromObjFile(filename);
}

const Primitive& GameEngine::GetPrimitive(const char* filename) const
{
  return primitiveBuffer->GetPrimitive(filename);
}

/**
* 乱数を取得する
*/
unsigned int GameEngine::GetRandom()
{
  return rg();
}

