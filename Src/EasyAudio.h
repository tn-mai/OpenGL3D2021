/**
* @file Audio.h
*/
#ifndef EASY_AUDIO_H_INCLUDED
#define EASY_AUDIO_H_INCLUDED

namespace Audio {

bool Initialize();
void Finalize();
void Update();

// プレイヤーを指定して再生・停止
void Play(int playerId, const char* filename, float volume = 1.0f, bool isLoop = false);
void Stop(int playerId);

// 音量の設定・取得
void SetVolume(int playerId, float volume);
float GetVolume(int playerId);

// プレイヤーが再生中かどうか(再生中ならtrue、それ以外はfalse)
bool IsPlaying(int playerId);

// 単発再生
void PlayOneShot(const char* filename, float volume = 1.0f);

// マスターボリュームの設定・取得
void SetMasterVolume(float volume);
float GetMasterVolume();

} // namespace Audio

#endif // EASY_AUDIO_H_INCLUDED