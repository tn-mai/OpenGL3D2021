/**
* @file Audio.h
*/
#ifndef EASY_AUDIO_H_INCLUDED
#define EASY_AUDIO_H_INCLUDED

namespace Audio {

bool Initialize();
void Finalize();
void Update();

// �v���C���[���w�肵�čĐ��E��~
void Play(int playerId, const char* filename, float volume = 1.0f, bool isLoop = false);
void Stop(int playerId);

// ���ʂ̐ݒ�E�擾
void SetVolume(int playerId, float volume);
float GetVolume(int playerId);

// �v���C���[���Đ������ǂ���(�Đ����Ȃ�true�A����ȊO��false)
bool IsPlaying(int playerId);

// �P���Đ�
void PlayOneShot(const char* filename, float volume = 1.0f);

// �}�X�^�[�{�����[���̐ݒ�E�擾
void SetMasterVolume(float volume);
float GetMasterVolume();

} // namespace Audio

#endif // EASY_AUDIO_H_INCLUDED