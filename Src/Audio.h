/**
* @file Audio.h
*/
#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED
#define NOMINMAX
#undef APIENTRY
#include <cri_adx2le.h>
#include <vector>

/**
* ��������N���X
*/
class Audio
{
public:
  static bool Initialize(const char* acfPath, const char* dspBusName);
  static void Finalize();
  static Audio& Get();

  // ACB, AWB�t�@�C���Ǘ�
  bool Load(size_t index, const char* acbPath, const char* acwPath);
  void Unload(size_t index);

  // �I�[�f�B�I��Ԃ̍X�V
  void Update();

  // �Đ�����
  void Play(size_t playerId, int cueId, float volume = 1.0f);
  void Stop(size_t playerId);

private:
  Audio() = default;
  ~Audio() = default;
  Audio(const Audio&) = delete;
  Audio& operator=(const Audio&) = delete;

  CriAtomExVoicePoolHn voicePool = nullptr;
  CriAtomDbasId dbasId = CRIATOMDBAS_ILLEGAL_ID;
  std::vector<CriAtomExAcbHn> acbList;
  std::vector<CriAtomExPlayerHn> players;
  std::vector<CriAtomExAcbHn> cueIdToAcbMap;
};

#endif // AUDIO_H_INCLUDED
