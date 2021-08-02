/**
* @file GameEngine.cpp
*/
#include "GameEngine.h"

namespace {

GameEngine* engine = nullptr;

}

/**
* �Q�[���G���W���̏�����
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
* �Q�[���G���W���̏I��
*/
void GameEngine::Finalize()
{
  delete engine;
  engine = nullptr;
}

/**
* �Q�[���G���W�����擾����
*/
GameEngine& GameEngine::Get()
{
  return *engine;
}

/**
* �Q�[���G���W�����X�V����
*/
void GameEngine::UpdateActors()
{
  // �V�K�ɍ쐬���ꂽ�A�N�^�[���A�N�^�[�z��ɒǉ�����
  for (int i = 0; i < newActors.size(); ++i) {
    actors.push_back(newActors[i]);
  }

  // �V�K�A�N�^�[�z�����ɂ���
  newActors.clear();
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
* �������擾����
*/
unsigned int GameEngine::GetRandom()
{
  return rg();
}

