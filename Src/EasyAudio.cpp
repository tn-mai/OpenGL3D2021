/**
* @file Audio.cpp
*
* XAudio2��Windows Media Foundation�ɂ�鉹���̍Đ�.
*/
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include "EasyAudio.h"
#include <xaudio2.h>
#include <vector>
#include <list>
#include <stdint.h>
#include <wrl/client.h>
#include <algorithm>
#include <wincodec.h>
#include <iostream>

#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "xaudio2.lib")

#ifndef NDEBUG
//#define ENABLE_XAUDIO_DEBUG
#endif // NDEBUG

/**
* �����֘A�̃R�[�h���i�[���閼�O���.
*/
namespace Audio {

using Microsoft::WRL::ComPtr;

/**
* �����̍Đ����.
*/
enum State {
	State_Create = 0x01,
	State_Preparing = 0x02,
	State_Prepared = 0x04,
	State_Playing = 0x08,
	State_Stopped = 0x10,
	State_Pausing = 0x20,
	State_Failed = 0x40,
};

/**
* �����̍Đ�����t���O.
*/
enum Flag
{
	Flag_None = 0,
	Flag_Loop = 0x01,
};

/**
* ��������C���^�[�t�F�C�X.
*
* Engine::Prepare, Engine::PrepareStream�֐����g���Ď擾����.
*/
class Sound
{
public:
	virtual ~Sound() = default;
	virtual bool Play(int flags = 0) = 0; ///< �Đ��y�шꎞ��~���̍ĊJ.
	virtual bool Pause() = 0; ///< �ꎞ��~.
	virtual bool Seek() = 0; ///< �Đ��ʒu�̕ύX(������).
	virtual bool Stop() = 0; ///< ��~.
  virtual float SetVolume(float) = 0; ///< ���ʂ̐ݒ�(����l=1.0).
	virtual float SetPitch(float) = 0; ///< �����̐ݒ�(����l=1.0);
	virtual int GetState() const = 0; ///< �Đ���Ԃ̎擾.
  virtual float GetVolume() const = 0; ///< ���ʂ̎擾.
  virtual float GetPitch() const = 0; ///< �����̎擾.
  virtual bool IsNull() const = 0; ///< �����ݒ�̗L��(true=�ݒ肳��Ă��Ȃ�, false=�ݒ肳��Ă���)

  void SetFilename(const char* filename) { this->filename = filename; }
  const std::string& GetFilename() const { return filename; }

  void SetId(int id) { this->id = id; }
  int GetId() const { return id; }

private:
  int id = -1;
  std::string filename;
};

struct MediaFoundationInitialize {
  MediaFoundationInitialize() {
    success = SUCCEEDED(MFStartup(MF_VERSION, MFSTARTUP_LITE));
  }
  ~MediaFoundationInitialize() {
    if (success) {
      MFShutdown();
    }
  }
  bool success;
};

using SoundPtr = std::shared_ptr<Sound>;

// WAV�`���p��FOURCC
const uint32_t FOURCC_RIFF_TAG = MAKEFOURCC('R', 'I', 'F', 'F');
const uint32_t FOURCC_FORMAT_TAG = MAKEFOURCC('f', 'm', 't', ' ');
const uint32_t FOURCC_DATA_TAG = MAKEFOURCC('d', 'a', 't', 'a');
const uint32_t FOURCC_WAVE_FILE_TAG = MAKEFOURCC('W', 'A', 'V', 'E');
const uint32_t FOURCC_XWMA_FILE_TAG = MAKEFOURCC('X', 'W', 'M', 'A');
const uint32_t FOURCC_XWMA_DPDS = MAKEFOURCC('d', 'p', 'd', 's');

struct RIFFChunk
{
  uint32_t tag;
  uint32_t size;
};

/**
* WAV�f�[�^.
*/
struct WF
{
  union U {
    WAVEFORMATEXTENSIBLE ext;
    struct ADPCMWAVEFORMAT {
      WAVEFORMATEX    wfx;
      WORD            wSamplesPerBlock;
      WORD            wNumCoef;
      ADPCMCOEFSET    coef[7];
    } adpcm;
  } u;
  size_t dataOffset;
  size_t dataSize;
  size_t seekOffset;
  size_t seekSize;
};

typedef std::vector<uint8_t> BufferType;

// Windows�n���h�����Ǘ�����^
struct ScopedHandle
{
  ScopedHandle(HANDLE h) : handle(h == INVALID_HANDLE_VALUE ? 0 : h) {}
  ~ScopedHandle() { if (handle) { CloseHandle(handle); } }
  operator HANDLE() { return handle; }
  HANDLE handle;
};

// �o�C�i���f�[�^��ǂݍ���
bool Read(HANDLE hFile, void* buf, size_t size)
{
  if (size > std::numeric_limits<DWORD>::max()) {
    return false;
  }
  DWORD readSize;
  if (!ReadFile(hFile, buf, static_cast<DWORD>(size), &readSize, nullptr) || readSize != size) {
    return false;
  }
  return true;
}

// WAV�t�@�C���̃t�H�[�}�b�g�����擾
uint32_t GetWaveFormatTag(const WAVEFORMATEXTENSIBLE& wf)
{
  if (wf.Format.wFormatTag != WAVE_FORMAT_EXTENSIBLE) {
    return wf.Format.wFormatTag;
  }
  return wf.SubFormat.Data1;
}

// WAV�t�@�C����ǂݍ���
bool LoadWaveFile(HANDLE hFile, WF& wf, std::vector<UINT32>& seekTable, std::vector<uint8_t>* source)
{
  RIFFChunk riffChunk;
  if (!Read(hFile, &riffChunk, sizeof(riffChunk))) {
    return false;
  }
  if (riffChunk.tag != FOURCC_RIFF_TAG) {
    return false;
  }

  uint32_t fourcc;
  if (!Read(hFile, &fourcc, sizeof(fourcc))) {
    return false;
  }
  if (fourcc != FOURCC_WAVE_FILE_TAG && fourcc != FOURCC_XWMA_FILE_TAG) {
    return false;
  }

  bool hasWaveFormat = false;
  bool hasData = false;
  bool hasDpds = false;
  size_t offset = 12;
  do {
    if (SetFilePointer(hFile, static_cast<LONG>(offset), nullptr, FILE_BEGIN) != offset) {
      return false;
    }

    RIFFChunk chunk;
    if (!Read(hFile, &chunk, sizeof(chunk))) {
      break;
    }

    if (chunk.tag == FOURCC_FORMAT_TAG) {
      if (!Read(hFile, &wf.u, std::min<size_t>(chunk.size, sizeof(WF::U)))) {
        break;
      }
      switch (GetWaveFormatTag(wf.u.ext)) {
      case WAVE_FORMAT_PCM:
        wf.u.ext.Format.cbSize = 0;
        /* fallthrough */
      case WAVE_FORMAT_IEEE_FLOAT:
      case WAVE_FORMAT_ADPCM:
        wf.seekSize = 0;
        wf.seekOffset = 0;
        hasDpds = true;
        break;
      case WAVE_FORMAT_WMAUDIO2:
      case WAVE_FORMAT_WMAUDIO3:
        break;
      default:
        // ���̃R�[�h�ŃT�|�[�g���Ȃ��t�H�[�}�b�g.
        return false;
      }
      hasWaveFormat = true;
    }
    else if (chunk.tag == FOURCC_DATA_TAG) {
      wf.dataOffset = offset + sizeof(RIFFChunk);
      wf.dataSize = chunk.size;
      hasData = true;
    }
    else if (chunk.tag == FOURCC_XWMA_DPDS) {
      wf.seekOffset = offset + sizeof(RIFFChunk);
      wf.seekSize = chunk.size / 4;
      hasDpds = true;
    }
    offset += chunk.size + sizeof(RIFFChunk);
  } while (!hasWaveFormat || !hasData || !hasDpds);
  if (!(hasWaveFormat && hasData && hasDpds)) {
    return false;
  }

  if (wf.seekSize) {
    seekTable.resize(wf.seekSize);
    SetFilePointer(hFile, static_cast<LONG>(wf.seekOffset), nullptr, FILE_BEGIN);
    if (!Read(hFile, seekTable.data(), wf.seekSize * 4)) {
      return false;
    }
  }
  if (source) {
    source->resize(wf.dataSize);
    SetFilePointer(hFile, static_cast<LONG>(wf.dataOffset), nullptr, FILE_BEGIN);
    if (!Read(hFile, source->data(), wf.dataSize)) {
      return false;
    }
  }
  return true;
}

/**
* Sound�̋����.
*
* �I�[�f�B�I�t�@�C���̃p�X����������������A�Đ��ł��Ȃ��t�H�[�}�b�g�������肵��
* �ꍇ�ɁA���̃N���X�̃I�u�W�F�N�g���Ԃ����.
*/
class NullSoundImpl : public Sound
{
public:
  virtual ~NullSoundImpl() = default;
  virtual bool Play(int) override { return false; }
  virtual bool Pause() override { return false; }
  virtual bool Seek() override { return false; }
  virtual bool Stop() override { return false; }
  virtual float SetVolume(float) override { return 0; }
  virtual float SetPitch(float) override { return 0; }
  virtual int GetState() const override { return 0; }
  virtual float GetVolume() const override { return 0; }
  virtual float GetPitch() const override { return 0; }
  virtual bool IsNull() const override { return true; }
};

/**
* Sound�̎���.
*
* �X�g���[�~���O���s��Ȃ��I�[�f�B�I�N���X.
* SE�ɓK���Ă���.
* �Ή��`��: WAV, XWM.
*/
class SoundImpl : public Sound
{
public:
  SoundImpl() : state(State_Create), sourceVoice(nullptr) {
  }

  virtual ~SoundImpl() override {
    if (sourceVoice) {
      sourceVoice->DestroyVoice();
    }
  }

  virtual bool Play(int flags) override {
    if (!sourceVoice) {
      return false;
    }
    if (!(state & State_Pausing)) {
      Stop();
      XAUDIO2_BUFFER buffer = {};
      buffer.Flags = XAUDIO2_END_OF_STREAM;
      buffer.AudioBytes = static_cast<UINT32>(source.size());
      buffer.pAudioData = source.data();
      buffer.LoopCount = flags & Flag_Loop ? XAUDIO2_LOOP_INFINITE : XAUDIO2_NO_LOOP_REGION;
      if (seekTable.empty()) {
        if (FAILED(sourceVoice->SubmitSourceBuffer(&buffer))) {
          return false;
        }
      } else {
        const XAUDIO2_BUFFER_WMA seekInfo = { seekTable.data(), static_cast<UINT32>(seekTable.size()) };
        if (FAILED(sourceVoice->SubmitSourceBuffer(&buffer, &seekInfo))) {
          return false;
        }
      }
    }
    state = State_Playing;
    return SUCCEEDED(sourceVoice->Start());
  }

  virtual bool Pause() override {
    if (sourceVoice && (state & State_Playing)) {
      state |= State_Pausing;
      return SUCCEEDED(sourceVoice->Stop());
    }
    return false;
  }

  virtual bool Seek() override {
    return sourceVoice;
  }

  virtual bool Stop() override {
    if (sourceVoice && (state & State_Playing)) {
      if (!(state & State_Pausing) && FAILED(sourceVoice->Stop())) {
        return false;
      }
      state = State_Stopped;
      return SUCCEEDED(sourceVoice->FlushSourceBuffers());
    }
    return false;
  }

  virtual float SetVolume(float volume) override {
    if (sourceVoice) {
      sourceVoice->SetVolume(volume);
    }
    return volume;
  }

  virtual float SetPitch(float pitch) override {
    if (sourceVoice) {
      sourceVoice->SetFrequencyRatio(pitch);
    }
    return pitch;
  }

  virtual int GetState() const override {
    if (!sourceVoice) {
      return State_Failed;
    }
    XAUDIO2_VOICE_STATE s;
    sourceVoice->GetState(&s);
    return s.BuffersQueued ? state : (State_Stopped | State_Prepared);
  }

  virtual float GetVolume() const override {
    float volume = 0;
    if (sourceVoice) {
      sourceVoice->GetVolume(&volume);
    }
    return volume;
  }

  virtual float GetPitch() const override {
    float ratio = 0;
    if (sourceVoice) {
      sourceVoice->GetFrequencyRatio(&ratio);
    }
    return ratio;
  }

  virtual bool IsNull() const override { return false; }

  int state;
  IXAudio2SourceVoice* sourceVoice;
  std::vector<uint8_t> source;
  std::vector<UINT32> seekTable;
};

/**
* Media Foundation�𗘗p�����X�g���[���T�E���h�̎���.
*
* �X�g���[�~���O���s���I�[�f�B�I�N���X.
* BGM�ɓK���Ă���.
* �Ή��`��: WAV, MP3, AAC, WMA.
*/
class MFStreamSoundImpl : public Sound
{
public:
  MFStreamSoundImpl() = default;
  bool Init(ComPtr<IXAudio2> xaudio, IMFAttributes* attributes, const wchar_t* filename) {
    buf.resize(MAX_BUFFER_COUNT);
    curBuf = 0;
    // open media file.
    if (FAILED(MFCreateSourceReaderFromURL(filename, attributes, sourceReader.GetAddressOf()))) {
      return false;
    }
    ComPtr<IMFMediaType> nativeMediaType;
    if (FAILED(sourceReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nativeMediaType.GetAddressOf()))) {
      return false;
    }
    GUID majorType{};
    if (FAILED(nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType))) {
      return false;
    }
    if (majorType != MFMediaType_Audio) {
      return false;
    }
    GUID subType{};
    if (FAILED(nativeMediaType->GetGUID(MF_MT_SUBTYPE, &subType))) {
      return false;
    }
    if (subType == MFAudioFormat_Float || subType == MFAudioFormat_PCM) {
      // uncompressed format.
    } else {
      // compressed format.
      ComPtr<IMFMediaType> partialMediaType;
      if (FAILED(MFCreateMediaType(partialMediaType.GetAddressOf()))) {
        return false;
      }
      if (FAILED(partialMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio))) {
        return false;
      }
      if (FAILED(partialMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM))) {
        return false;
      }
      if (FAILED(sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, partialMediaType.Get()))) {
        return false;
      }
    }
    ComPtr<IMFMediaType> uncompressedMediaType;
    if (FAILED(sourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, uncompressedMediaType.GetAddressOf()))) {
      return false;
    }
    WAVEFORMATEX* pWaveFormatEx;
    uint32_t waveFormatLength;
    if (FAILED(MFCreateWaveFormatExFromMFMediaType(uncompressedMediaType.Get(), &pWaveFormatEx, &waveFormatLength))) {
      return false;
    }

    if (FAILED(xaudio->CreateSourceVoice(&sourceVoice, pWaveFormatEx))) {
      return false;
    }
    CoTaskMemFree(pWaveFormatEx);
    return true;
  }

  virtual ~MFStreamSoundImpl() override {
    if (sourceVoice) {
      sourceVoice->DestroyVoice();
    }
  }

  virtual bool Play(int flags) override {
    if (!sourceVoice) {
      return false;
    }
    if (!(state & State_Pausing)) {
      Stop();
    }
    isEndOfStream = false;
    state = State_Playing;
    loop = flags & Flag_Loop;
    return SUCCEEDED(sourceVoice->Start());
  }

  virtual bool Pause() override {
    if (sourceVoice && (state & State_Playing) && !(state & State_Pausing)) {
      state |= State_Pausing;
      return SUCCEEDED(sourceVoice->Stop());
    }
    return false;
  }

  virtual bool Seek() override {
    return sourceVoice;
  }

  virtual bool Stop() override {
    if (sourceVoice && (state & State_Playing)) {
      if (!(state & State_Pausing)) {
        if (FAILED(sourceVoice->Stop())) {
          return false;
        }
      }
      state = State_Stopped;
      isEndOfStream = false;
      curBuf = 0;
      currentPos = 0;

      // �ǂݍ��݈ʒu�����Z�b�g.
      PROPVARIANT value;
      value.vt = VT_I8;
      value.hVal.QuadPart = 0;
      sourceReader->SetCurrentPosition(GUID_NULL, value);

      return SUCCEEDED(sourceVoice->FlushSourceBuffers());
    }
    return false;
  }

  virtual float SetVolume(float volume) override {
    if (sourceVoice) {
      sourceVoice->SetVolume(volume);
    }
    return volume;
  }

  virtual float SetPitch(float pitch) override {
    if (sourceVoice) {
      sourceVoice->SetFrequencyRatio(pitch);
    }
    return pitch;
  }

  virtual int GetState() const override {
    if (!sourceVoice) {
      return State_Failed;
    }
    XAUDIO2_VOICE_STATE s;
    sourceVoice->GetState(&s);
    return s.BuffersQueued ? (state | State_Prepared) : State_Stopped;
  }

  virtual float GetVolume() const override {
    float volume = 0;
    if (sourceVoice) {
      sourceVoice->GetVolume(&volume);
    }
    return volume;
  }

  virtual float GetPitch() const override {
    float ratio = 0;
    if (sourceVoice) {
      sourceVoice->GetFrequencyRatio(&ratio);
    }
    return ratio;
  }

  virtual bool IsNull() const override { return false; }

  enum class Result {
    success,
    endOfStream,
    readError,
  };

  Result ReadFile() {
    ComPtr<IMFSample> sample;
    DWORD flags = 0;
    if (FAILED(sourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &flags, nullptr, sample.GetAddressOf()))) {
      return Result::readError;
    }
    if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
      // reached to the end of stream.
      return Result::endOfStream;
    }
    ComPtr<IMFMediaBuffer> buffer;
    if (FAILED(sample->ConvertToContiguousBuffer(buffer.GetAddressOf()))) {
      return Result::readError;
    }
    BYTE* pAudioData = nullptr;
    if (FAILED(buffer->Lock(&pAudioData, nullptr, &buf[curBuf].length))) {
      return Result::readError;
    }
    if (buf[curBuf].length > BUFFER_SIZE) {
      return Result::readError;
    }
    std::copy(pAudioData, pAudioData + buf[curBuf].length, buf[curBuf].data);
    if (FAILED(buffer->Unlock())) {
      return Result::readError;
    }
    return Result::success;
  }

  bool Update() {
    if (isEndOfStream) {
      return false;
    }
    if (!sourceVoice) {
      return false;
    }

    XAUDIO2_VOICE_STATE voiceState;
    sourceVoice->GetState(&voiceState);
    if (voiceState.BuffersQueued >= MAX_BUFFER_COUNT - 1) {
      return true;
    }

    Result result = ReadFile();
    if (result == Result::readError) {
      return false;
    }
    if (result == Result::endOfStream) {
      if (loop) {
        PROPVARIANT value;
        value.vt = VT_I8;
        value.hVal.QuadPart = 0;
        sourceReader->SetCurrentPosition(GUID_NULL, value);
        result = ReadFile();
        if (result == Result::readError) {
          return false;
        }
      } else {
        isEndOfStream = true;
        return true;
      }
    }

    XAUDIO2_BUFFER buffer = {};
    buffer.pAudioData = buf[curBuf].data;
    buffer.AudioBytes = buf[curBuf].length;
    if (!isEndOfStream) {
      buffer.Flags = 0;
    } else {
      buffer.Flags = XAUDIO2_END_OF_STREAM;
    }
    sourceVoice->SubmitSourceBuffer(&buffer, nullptr);

    currentPos += buffer.AudioBytes;
    curBuf = (curBuf + 1) % MAX_BUFFER_COUNT;
    return true;
  }

  ComPtr<IMFSourceReader> sourceReader;
  IXAudio2SourceVoice* sourceVoice = nullptr;

  static const size_t BUFFER_SIZE = 0x20000;
  static const int MAX_BUFFER_COUNT = 3;
  struct Sample {
    uint8_t data[BUFFER_SIZE];
    DWORD length;
  };

  int state = State_Create;
  bool loop = false;
  bool isEndOfStream = false;
  std::vector<Sample> buf;
  size_t curBuf = 0;
  size_t currentPos = 0;
};

/**
* �����Đ��̐���N���X.
*
* �������ƏI��:
* -# �A�v���P�[�V�����̏�����������Engine::Initialize()���Ăяo��.
* -# �A�v���P�[�V�����̏I��������Engine::Destroy()���Ăяo��.
*
* �����̍Đ�:
* -# �����t�@�C�����������ɂ���Prepare(�Ղ�؂�)�֐����Ăяo���ƁA�߂�l�Ƃ��āu��������C���^�[�t�F�C�X�v���Ԃ����̂ŁA�����ϐ��ɕۑ�����.
* -# ��������C���^�[�t�F�C�X�ɑ΂���Play�֐����Ăяo���Ɖ������Đ������. Play���Ăяo�����тɓ����������Đ������.
* -# Play���Ăяo���K�v���Ȃ��Ȃ����特������C���^�[�t�F�C�X��j������.
* -# �g���܂킵�����Ȃ������̏ꍇ�uengine.Prepare("OneTimeSound.wav")->Play()�v�̂悤�ɏ������Ƃ��ł���.
*/
class Engine : public std::enable_shared_from_this<Engine>
{
public:
  /**
  * �����Đ��G���W��������������.
  *
  * @retval true  ����������.
  * @retval false ���������s.
  */
  bool Initialize() {
    if (xaudio) {
      std::cerr << "WARNING: Audio::Engine�͏������ς݂ł�." << std::endl;
      return true;
    }
    ComPtr<IXAudio2> tmpAudio;
    UINT32 flags = 0;
#ifdef ENABLE_XAUDIO_DEBUG 
    flags |= XAUDIO2_DEBUG_ENGINE;
#endif // ENABLE_XAUDIO_DEBUG
    HRESULT hr = XAudio2Create(&tmpAudio, flags);
    if (FAILED(hr)) {
      std::cerr << "ERROR: XAudio2�̍쐬�Ɏ��s." << std::endl;
      return false;
    }

#ifdef ENABLE_XAUDIO_DEBUG 
    XAUDIO2_DEBUG_CONFIGURATION debug = {};
    debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS | XAUDIO2_LOG_MEMORY;
    debug.BreakMask = XAUDIO2_LOG_ERRORS;
    debug.LogFunctionName = TRUE;
    tmpAudio->SetDebugConfiguration(&debug);
#endif // ENABLE_XAUDIO_DEBUG 

    hr = tmpAudio->CreateMasteringVoice(&masteringVoice);
    if (FAILED(hr)) {
      std::cerr << "ERROR: XAudio2�̉��ʐݒ�Ɏ��s." << std::endl;
      return false;
    }

    // Setup for Media foundation.
    mf = std::make_unique<MediaFoundationInitialize>();
    if (FAILED(MFCreateAttributes(attributes.GetAddressOf(), 1))) {
      std::cerr << "ERROR: Media Foundation�̑����I�u�W�F�N�g�̍쐬�Ɏ��s." << std::endl;
      return false;
    }
    if (FAILED(attributes->SetUINT32(MF_LOW_LATENCY, true))) {
      std::cerr << "ERROR: Media Foundation�̃��C�e���V�̐ݒ�Ɏ��s." << std::endl;
      return false;
    }
    xaudio.Swap(std::move(tmpAudio));

    return true;
  }

  /**
  * �G���W����j������.
  */
  void Destroy() {
    soundList.clear();
    mfSoundList.clear();
  }

  /**
  * �G���W���̏�Ԃ��X�V����.
  *
  * ����I�ɌĂяo���K�v������.
  *
  * @retval true  �X�V����.
  * @retval false �X�V���s.
  *
  * @note ���݂͏��true��Ԃ�.
  */
  bool Update() {
    soundList.remove_if(
      [](const SoundList::value_type& p) { return (p.use_count() <= 1) && (p->GetState() & State_Stopped); }
    );

    for (auto&& e : mfSoundList) {
      e->Update();
    }
    mfSoundList.remove_if(
      [](const MFSoundList::value_type& p) { return (p.use_count() <= 1) && (p->GetState() & State_Stopped); }
    );

    return true;
  }

  /**
  * ��������������(SJIS������p).
  *
  * @param filename �����t�@�C���̃p�X(SJIS������).
  *
  * @return �����I�u�W�F�N�g�ւ̃|�C���^.
  *
  * �����V���b�g�Đ������Ȃ玟�̂悤�ɒ���Play()���Ăяo�����Ƃ��ł���.
  * @code
  * Audio::Engine::Instance().Prepare("�t�@�C���p�X")->Play()
  * @endcode
  *
  * ���ʉ��̂悤�ɉ��x���Đ����鉹���A�܂��͒�~�̕K�v�����鉹���̏ꍇ�A�߂�l��ϐ��Ɋi�[���Ă�����
  * �K�v�ȃ^�C�~���O�Ŋ֐����Ă�.
  * @code
  * // �������������A�߂�l�̉�������I�u�W�F�N�g��ϐ�se�Ɋi�[.
  * Audio::SoundPtr se = Audio::Engine::Instance().Prepare("�t�@�C���p�X");
  * �`�`�`
  * // �ϐ�se�Ɋi�[������������I�u�W�F�N�g���g���čĐ�.
  * se->Play();
  * @endcode
  */
  SoundPtr Prepare(const char* filename) {
    std::vector<wchar_t> wcFilename(std::strlen(filename) + 1);
    const size_t len = mbstowcs(wcFilename.data(), filename, wcFilename.size());
    if (len != static_cast<size_t>(-1)) {
      wcFilename[len] = L'\0';
      if (SoundPtr p = Prepare(wcFilename.data())) {
        return p;
      }
      if (SoundPtr p = PrepareMFStream(wcFilename.data())) {
        return p;
      }
    }
    std::cerr << "ERROR: " << filename << "��ǂݍ��߂܂���." << std::endl;
    return std::make_shared<NullSoundImpl>();
  }

  /**
  * ��������������(UTF-16������p).
  *
  * @param filename �����t�@�C���̃p�X(UTF-16������).
  *
  * @return �����I�u�W�F�N�g�ւ̃|�C���^.
  *
  * ���݂̂Ƃ���A���̊֐���Media Foundation���Ή��̂��߁A�ʏ��SJIS������ł�Prepare���g�p���邱��.
  */
  SoundPtr Prepare(const wchar_t* filename) {
    ScopedHandle hFile = CreateFile2(filename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
    if (!hFile) {
      return nullptr;
    }
    WF wf;
    std::shared_ptr<SoundImpl> sound(new SoundImpl);
    if (!LoadWaveFile(hFile, wf, sound->seekTable, &sound->source)) {
      return nullptr;
    }
    if (FAILED(xaudio->CreateSourceVoice(&sound->sourceVoice, &wf.u.ext.Format))) {
      return nullptr;
    }
    soundList.push_back(sound);
    return sound;
  }

  /**
  * �X�g���[�~���O���s����������������(UTF-16������p).
  *
  * @param filename �����t�@�C���̃p�X(UTF-16������).
  *
  * @return �����I�u�W�F�N�g�ւ̃|�C���^.
  *
  * Windows��Media Foundation�𗘗p����AAC��MP3�Ȃǂ̍Đ����\�ɂ��Ă���.
  * PrepareStream�Ƃ͈قȂ�A���������̓����Đ����s�����Ƃ��ł���.
  *
  * @note �ʏ�́A���̊֐��ł͂Ȃ�Prepare�֐����g������.
  */
  SoundPtr PrepareMFStream(const wchar_t* filename) {
    std::shared_ptr<MFStreamSoundImpl> mfs = std::make_shared<MFStreamSoundImpl>();
    if (!mfs->Init(xaudio, attributes.Get(), filename)) {
      return nullptr;
    }
    mfSoundList.push_back(mfs);
    return mfs;
  }

  /**
  * �}�X�^�[�{�����[����ݒ肷��.
  *
  * @param vol �ݒ肷�鉹��.
  */
  void SetMasterVolume(float vol) {
    if (xaudio) {
      masteringVoice->SetVolume(vol);
    }
  }

  /**
  * �}�X�^�[�{�����[�����擾����.
  *
  * @return �ݒ肳��Ă��鉹��.
  */
  float GetMasterVolume() const {
    if (xaudio) {
      float vol;
      masteringVoice->GetVolume(&vol);
      return vol;
    }
    return 0;
  }

  SoundPtr FindSound(int id) const {
    for (auto& e : mfSoundList) {
      if (e->GetId() == id) {
        return e;
      }
    }
    for (auto& e : soundList) {
      if (e->GetId() == id) {
        return e;
      }
    }
    return nullptr;
  }

  SoundPtr FindSound(const char* filename) const {
    for (auto& e : mfSoundList) {
      if (e->GetFilename() == filename) {
        return e;
      }
    }
    for (auto& e : soundList) {
      if (e->GetFilename() == filename) {
        return e;
      }
    }
    return nullptr;
  }

private:
  ComPtr<IXAudio2> xaudio;
  IXAudio2MasteringVoice* masteringVoice = nullptr;

  using SoundList = std::list<std::shared_ptr<SoundImpl>>;
  SoundList soundList;

  std::unique_ptr<MediaFoundationInitialize> mf;
  ComPtr<IMFByteStream> pMFByteStream;
  ComPtr<IMFSourceReader> pMFSourceReader;
  ComPtr<IMFAttributes> attributes;
  typedef std::list<std::shared_ptr<MFStreamSoundImpl>> MFSoundList;
  MFSoundList mfSoundList;
};

std::unique_ptr<Engine> engine;

/**
* �I�[�f�B�I������������
*/
bool Initialize()
{
  if (!engine) {
    engine = std::make_unique<Engine>();
  }
  return engine->Initialize();
}

/**
* �I�[�f�B�I��j������
*/
void Finalize()
{
  if (engine) {
    engine->Destroy();
    engine.reset();
  }
}

/**
* �I�[�f�B�I��Ԃ̍X�V
*/
void Update()
{
  if (!engine) {
    return;
  }
  engine->Update();
}

/**
* �v���C���[���w�肵�čĐ�
*/
void Play(int playerId, const char* filename, float volume, bool isLoop)
{
  if (!engine) {
    return;
  }

  SoundPtr old = engine->FindSound(playerId);
  if (old) {
    old->Stop();
    old->SetId(-1);
  }

  SoundPtr p = engine->Prepare(filename);
  if (p) {
    p->Play(static_cast<int>(isLoop ? Flag_Loop : Flag_None));
    p->SetVolume(volume);
    p->SetId(playerId);
    p->SetFilename(filename);
  }
}

/**
* ���[�v�Đ����~����
*/
void Stop(int playerId)
{
  if (!engine) {
    return;
  }

  SoundPtr old = engine->FindSound(playerId);
  if (old) {
    old->Stop();
    old->SetId(-1);
  }
}

/**
* �Đ������ǂ������ׂ�
*/
bool IsPlaying(int playerId)
{
  if (!engine) {
    return false;
  }

  SoundPtr p = engine->FindSound(playerId);
  if (p) {
    return p->GetState() == State_Playing;
  }
  return false;
}

/**
* ���ʂ�ύX����
*/
void SetVolume(int playerId, float volume)
{
  if (!engine) {
    return;
  }

  SoundPtr p = engine->FindSound(playerId);
  if (p) {
    p->SetVolume(volume);
  }
}

/**
* ���ʂ��擾����
*/
float GetVolume(int playerId)
{
  if (!engine) {
    return 0;
  }

  SoundPtr p = engine->FindSound(playerId);
  if (p) {
    return p->GetVolume();
  }
  return 0;
}

/**
* �P���Đ�
*/
void PlayOneShot(const char* filename, float volume)
{
  if (!engine) {
    return;
  }

  SoundPtr p = engine->Prepare(filename);
  if (p) {
    p->Play(Flag_None);
    p->SetVolume(volume);
    p->SetFilename(filename);
  }
}

/**
* �}�X�^�[�{�����[����ݒ肷��
*/
void SetMasterVolume(float volume)
{
  if (!engine) {
    return;
  }
  engine->SetMasterVolume(volume);
}

/**
* �}�X�^�[�{�����[�����擾����
*/
float GetMasterVolume()
{
  if (!engine) {
    return 0;
  }
  return engine->GetMasterVolume();
}

} // namespace Audio

