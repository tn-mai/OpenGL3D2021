/**
* @file Audio.cpp
*/
#include "Audio.h"
#include <algorithm>
#include <iostream>
#pragma warning(disable : 26812)

namespace {

Audio* audio = nullptr;

/**
* �I�[�f�B�I�p�G���[�R�[���o�b�N.
*
* @param errid  �G���[�̎�ނ�����ID.
* @param p1     errid�̕⑫��񂻂�1.
* @param p2     errid�̕⑫��񂻂�2.
* @param parray (���g�p).
*/
void AudioErrorCallback(const CriChar8* errid, CriUint32 p1, CriUint32 p2,
  CriUint32* parray)
{
  const CriChar8* err = criErr_ConvertIdToMessage(errid, p1, p2);
  std::cerr << err << std::endl;
}

/**
* �I�[�f�B�I�p�̃������m�ۊ֐�.
*
* @param obj  �o�^���Ɏw�肵�����[�U�[����.
* @param size �m�ۂ���o�C�g��.
*
* @return �m�ۂ����������̃A�h���X.
*/
void* AudioAllocate(void* obj, CriUint32 size)
{
  return operator new(size);
}

/**
* �I�[�f�B�I�p�̃���������֐�.
*
* @param obj �o�^���Ɏw�肵�����[�U�[����.
* @param ptr �J�����郁�����̃A�h���X.
*/
void AudioDeallocate(void* obj, void* ptr)
{
  operator delete(obj);
}

} // namespace

/**
* ��������V�X�e��������������
*
* @param acfPaht    �S�̐ݒ��ێ�����ACF�t�@�C���̃p�X
* @param dspBusName �����V�X�e���Ŏg�p����DSP�o�X��
*
* @retval true  ����������
* @retval false ���������s
*/
bool Audio::Initialize(const char* acfPath, const char* dspBusName)
{
  if (!audio) {
    audio = new Audio;

    // �G���[�R�[���o�b�N�֐���o�^����
    criErr_SetCallback(AudioErrorCallback);

    // �������Ǘ��֐���o�^����
    criAtomEx_SetUserAllocator(AudioAllocate, AudioDeallocate, nullptr);

    // �������p�����[�^��ݒ肷��
    CriAtomExConfig_WASAPI libConfig;
    criAtomEx_SetDefaultConfig_WASAPI(&libConfig);

    // ���[�_�[����ݒ肷��
    CriFsConfig fsConfig;
    criFs_SetDefaultConfig(&fsConfig);
    fsConfig.num_loaders = 40; // num_voices���傫���l��ݒ肷�邱��
    libConfig.atom_ex.fs_config = &fsConfig;

    // �Đ�����\�ȉ����̍ő吔. ���ۂ̔������̓{�C�X�v�[����num_voices�Ŏw�肷��
    libConfig.atom_ex.max_virtual_voices = 64;

    // OpenGL�p�ɉE����W�n���w��
    libConfig.atom_ex.coordinate_system = CRIATOMEX_COORDINATE_SYSTEM_RIGHT_HANDED;

    // Atom���C�u������������
    criAtomEx_Initialize_WASAPI(&libConfig, nullptr, 0);

    // �X�g���[�~���O�p�o�b�t�@���쐬
    audio->dbasId = criAtomDbas_Create(nullptr, nullptr, 0);

    // ACF�t�@�C����ǂݍ���
    if (criAtomEx_RegisterAcfFile(nullptr, acfPath, nullptr, 0) == CRI_FALSE) {
      std::cerr << "[�G���[]" << __func__ << ":" << acfPath << "�̓ǂݍ��݂Ɏ��s\n";
      Finalize();
      return false;
    }

    // DSP�o�X�����蓖�Ă�
    criAtomEx_AttachDspBusSetting(dspBusName, nullptr, 0);

    // �{�C�X�v�[����ݒ肷��
    CriAtomExStandardVoicePoolConfig svpConfig;
    criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&svpConfig);
    svpConfig.num_voices = libConfig.atom_ex.max_virtual_voices / 2; // ����������
    svpConfig.player_config.streaming_flag = CRI_TRUE; // �X�g���[�~���O�Đ���L����
    svpConfig.player_config.max_sampling_rate =
      48000 * 2; // �ő�T���v�����O���[�g�A�s�b�`�ύX���l������CD������2�{��ݒ�
    audio->voicePool = criAtomExVoicePool_AllocateStandardVoicePool(&svpConfig, nullptr, 0);

    // �Đ�����p�v���C���[���쐬����
    audio->players.resize(8);
    for (auto& e : audio->players) {
      e = criAtomExPlayer_Create(nullptr, nullptr, 0);
    }

    // acb�ǂݍ��ݔz����m��
    // �����Ŏw�肵�����́A�����ɓǂݍ��݉\��ACB�t�@�C���̍ő吔�ɂȂ�
    // Load�Ŕz��ɓǂݍ��݁AUnload�Ŕj������
    audio->acbList.resize(16);

    // �L���[ID��acb�t�@�C���̑Ή��\���m��
    // �L���[ID��Y�����Ɏg�����ƂőΉ�����acb�t�@�C�����擾�ł���
    // �T�E���h�Đ��ɂ͂̓L���[ID��acb�t�@�C���̃y�A���K�v�Ȃ���
    // Load�őΉ��\�ɒǉ�����AUnload�ō폜�����
    audio->cueIdToAcbMap.resize(4096);

    std::cerr << "[���]" << __func__ << ": �I�[�f�B�I�@�\��������\n";
  }
  return true;
}

/**
* ��������V�X�e����j������
*/
void Audio::Finalize()
{
  // ���ɔj������Ă����牽�����Ȃ�
  if (!audio) {
    return;
  }

  // ���ׂẴv���C���[��j��
  for (auto& e : audio->players) {
    if (e) {
      criAtomExPlayer_Destroy(e);
      e = nullptr;
    }
  }

  // ���ׂĂ�ACB�t�@�C����j��
  for (auto& e : audio->acbList) {
    if (e) {
      criAtomExAcb_Release(e);
      e = nullptr;
    }
  }

  // �L���[ID��ACB�̑Ή��\��������
  std::fill(audio->cueIdToAcbMap.begin(), audio->cueIdToAcbMap.end(), nullptr);

  // �{�C�X�v�[����j��
  if (audio->voicePool) {
    criAtomExVoicePool_Free(audio->voicePool);
    audio->voicePool = nullptr;
  }

  // ACF�t�@�C���̓o�^������
  criAtomEx_UnregisterAcf();

  // DBAS��j��
  if (audio->dbasId != CRIATOMDBAS_ILLEGAL_ID) {
    criAtomDbas_Destroy(audio->dbasId);
    audio->dbasId = CRIATOMDBAS_ILLEGAL_ID;
  }

  // ADXLE���I��
  criAtomEx_Finalize_WASAPI();

  delete audio;
  audio = nullptr;

  std::cerr << "[���]" << __func__ << ": �I�[�f�B�I�@�\��j��\n";
}

/**
* ��������N���X���擾����
*
* @return ��������N���X�̃C���X�^���X�̎Q��
*/
Audio& Audio::Get()
{
  return *audio;
}

/**
* �����t�@�C����ǂݍ���
*
* @param index   �ǂݍ��ݐ��ACB�z��̔ԍ�
* @param acbPath ACB�t�@�C���̃p�X��
* @param awbPath AWB�t�@�C���̃p�X��.
*
* @retval true  �ǂݍ��ݐ���
* @retval false �ǂݍ��ݎ��s
*/
bool Audio::Load(size_t index, const char* acbPath, const char* awbPath)
{
  if (index >= acbList.size()) {
    std::cerr << "[�G���[]" << __func__ << ":" <<
      acbPath << "�̃C���f�b�N�X���傫�����܂�\n";
    return false;
  }

  // �O�̂��ߓǂݍ��ݐ�̗v�f�����
  Unload(index);

  // �����t�@�C����ǂݍ���
  acbList[index] = criAtomExAcb_LoadAcbFile(
    nullptr, acbPath, nullptr, awbPath, nullptr, 0);
  if (!acbList[index]) {
    std::cerr << "[�G���[]" << __func__ << ":" << acbPath << "�̓ǂݍ��݂Ɏ��s\n";
    return false;
  }

  // �L���[ID��ACB�̑Ή��\���X�V
  const CriSint32 numCues = criAtomExAcb_GetNumCues(acbList[index]);
  for (int i = 0; i < numCues; ++i) {
    // �L���[�����擾
    CriAtomExCueInfo cueInfo;
    if (!criAtomExAcb_GetCueInfoByIndex(acbList[index], i, &cueInfo)) {
      std::cerr << "[�x��]" << __func__ << ":" << acbPath << "��" <<
        i << "�Ԗڂ̃L���[�����擾�ł��܂���\n";
      continue;
    }
    // �Ή��\���L���[ID���傫���ꍇ�͑Ή��\�̃T�C�Y���g��
    if (cueIdToAcbMap.size() <= static_cast<size_t>(cueInfo.id)) {
      cueIdToAcbMap.resize(cueInfo.id + 1);
    }
    // �L���[ID��ACB��Ή��t����
    cueIdToAcbMap[cueInfo.id] = acbList[index];
  }

  return true;
}

/**
* �I�[�f�B�I�t�@�C����j������
*
* @param index �j������ACB�z��̔ԍ�
*/
void Audio::Unload(size_t index)
{
  if (index >= acbList.size()) {
    std::cerr << "[�G���[]" << __func__ << ":" <<
      "�C���f�b�N�X���傫�����܂�\n";
    return;
  }

  if (acbList[index]) {
    // �Ή��\����j���\���ACB�n���h�����폜
    std::replace(cueIdToAcbMap.begin(), cueIdToAcbMap.end(),
      acbList[index], static_cast<CriAtomExAcbHn>(nullptr));

    // ACB�n���h����j��
    criAtomExAcb_Release(acbList[index]);
    acbList[index] = nullptr;
  }
}

/**
* �����V�X�e���̏�Ԃ��X�V����
*/
void Audio::Update()
{
  // Atom���C�u�����̏�Ԃ��X�V
  criAtomEx_ExecuteMain();
}

/**
* �������Đ�����
*
* @param playerId �Đ��Ɏg�p����v���C���[�ԍ�
* @param cueId    �Đ�����L���[ID
*/
void Audio::Play(size_t playerId, int cueId, float volume)
{
  // �v���C���[�ԍ����v���C���[���ȏ�̏ꍇ�͉������Ȃ�
  if (playerId >= players.size()) {
    return;
  }
  // �Ή��\��nullptr�̏ꍇ�͉������Ȃ�
  if (!cueIdToAcbMap[cueId]) {
    return;
  }

  criAtomExPlayer_SetVolume(players[playerId], volume);

  // �v���C���[�ɃL���[���Z�b�g
  criAtomExPlayer_SetCueId(players[playerId], cueIdToAcbMap[cueId], cueId);

  // �Z�b�g�����L���[���Đ�
  criAtomExPlayer_Start(players[playerId]);
}

/**
* �������~����
*
* @param playerId �Đ����~����v���C���[�ԍ�
*/
void Audio::Stop(size_t playerId)
{
  // �v���C���[�ԍ����v���C���[���ȏ�̏ꍇ�͉������Ȃ�
  if (playerId >= players.size()) {
    return;
  }

  // �Đ����~����
  criAtomExPlayer_Stop(players[playerId]);
}









