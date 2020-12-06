/**
* @file Audio.h
*/
#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED
#undef APIENTRY
#include <cri_adx2le.h>
#include <vector>

/**
* 音声制御クラス.
*/
class Audio
{
public:
  static Audio& Instance();

  // システム・データ制御.
  bool Initialize(
    const char* acfPath, const char* dspBusName);
  void Finalize();
  bool Load(size_t slot, const char* acbPath, const char* awbPath);
  void Unload(size_t slot);
  void Update();

  // 再生制御.
  void Play(size_t playerId, int cueId, float volume=1);
  void Stop(size_t playerId);
  void SetMasterVolume(float);
  float GetMasterVolume() const;

private:
  Audio() = default;
  ~Audio() { Finalize(); }
  Audio(const Audio&) = delete;
  Audio& operator=(const Audio&) = delete;

  // ADX2LEに設定する補助関数.
  static void ErrorCallback(
    const CriChar8* errid, CriUint32 p1, CriUint32 p2, CriUint32* parray);
  static void* Allocate(void* obj, CriUint32 size);
  static void Deallocate(void* obj, void* ptr);

  // データ管理.
  CriAtomExVoicePoolHn voicePool = nullptr;
  CriAtomDbasId dbas = CRIATOMDBAS_ILLEGAL_ID;
  std::vector<CriAtomExAcbHn> acbList;
  std::vector<CriAtomExPlayerHn> players;
  std::vector<CriAtomExAcbHn> cueIdToAcbMap;

  bool isInitialized = false; // 初期化済みならtrue.
  float masterVolume = 1; // 全体の音量(無音=0 標準音量=1).
};

#endif // AUDIO_H_INCLUDED