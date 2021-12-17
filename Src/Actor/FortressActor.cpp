/**
* @file FortressActor.cpp
*/
#include "FortressActor.h"
#include "ExplosionActor.h"
#include "../GameEngine.h"
#include "../GameManager.h"
#include "../Audio.h"
#ifdef USE_EASY_AUDIO
#include "../EasyAudioSettings.h"
#else
#include "../Audio/MainWorkUnit/SE.h"
#endif // USE_EASY_AUDIO

/**
* �R���X�g���N�^
*/
FortressActor::FortressActor(
  const std::string& name,
  const Primitive& prim,
  std::shared_ptr<Texture> tex,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment) :
  Actor(name, prim, tex, position, scale, rotation, adjustment)
{
}

/**
* �R���X�g���N�^
*/
FortressActor::FortressActor(
  const std::string& name,
  const MeshPtr& mesh,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment) :
  Actor(name, mesh, position, scale, rotation, adjustment)
{
}

/**
* �R���X�g���N�^
*/
void FortressActor::OnCollision(const Contact& contact)
{
  if (contact.b->name == "Bullet") {
#ifdef USE_EASY_AUDIO
    Audio::PlayOneShot(SE_HIT);
#else
    Audio::Get().Play(1, CRI_SE_HIT);
#endif // USE_EASY_AUDIO

    // �ϋv�l�����炷
    health -= 1;
    if (health <= 0) {
      isDead = true; // �A�N�^�[����������
      GameManager::Get().AddScore(1000);

      GameEngine& engine = GameEngine::Get();
      engine.AddActor(std::make_shared<ExplosionActor>(position, 4.0f));

      // ���������Đ�
#ifdef USE_EASY_AUDIO
      Audio::PlayOneShot(SE_EXPLOSION);
#else
      Audio::Get().Play(1, CRI_SE_EXPLOSION);
#endif // USE_EASY_AUDIO
    }
    contact.b->isDead = true; // �e����������
  }
}

