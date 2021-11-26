/**
* @file Audio.cpp
*
* XAudio2とWindows Media Foundationによる音声の再生.
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
* 音声関連のコードを格納する名前空間.
*/
namespace Audio {

using Microsoft::WRL::ComPtr;

/**
* 音声の再生状態.
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
* 音声の再生制御フラグ.
*/
enum Flag
{
	Flag_None = 0,
	Flag_Loop = 0x01,
};

/**
* 音声制御インターフェイス.
*
* Engine::Prepare, Engine::PrepareStream関数を使って取得する.
*/
class Sound
{
public:
	virtual ~Sound() = default;
	virtual bool Play(int flags = 0) = 0; ///< 再生及び一時停止中の再開.
	virtual bool Pause() = 0; ///< 一時停止.
	virtual bool Seek() = 0; ///< 再生位置の変更(未実装).
	virtual bool Stop() = 0; ///< 停止.
  virtual float SetVolume(float) = 0; ///< 音量の設定(既定値=1.0).
	virtual float SetPitch(float) = 0; ///< 音程の設定(既定値=1.0);
	virtual int GetState() const = 0; ///< 再生状態の取得.
  virtual float GetVolume() const = 0; ///< 音量の取得.
  virtual float GetPitch() const = 0; ///< 音程の取得.
  virtual bool IsNull() const = 0; ///< 音声設定の有無(true=設定されていない, false=設定されている)

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

// WAV形式用のFOURCC
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
* WAVデータ.
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

// Windowsハンドルを管理する型
struct ScopedHandle
{
  ScopedHandle(HANDLE h) : handle(h == INVALID_HANDLE_VALUE ? 0 : h) {}
  ~ScopedHandle() { if (handle) { CloseHandle(handle); } }
  operator HANDLE() { return handle; }
  HANDLE handle;
};

// バイナリデータを読み込む
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

// WAVファイルのフォーマット情報を取得
uint32_t GetWaveFormatTag(const WAVEFORMATEXTENSIBLE& wf)
{
  if (wf.Format.wFormatTag != WAVE_FORMAT_EXTENSIBLE) {
    return wf.Format.wFormatTag;
  }
  return wf.SubFormat.Data1;
}

// WAVファイルを読み込む
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
        // このコードでサポートしないフォーマット.
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
* Soundの空実装.
*
* オーディオファイルのパス名が無効だったり、再生できないフォーマットだったりした
* 場合に、このクラスのオブジェクトが返される.
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
* Soundの実装.
*
* ストリーミングを行わないオーディオクラス.
* SEに適している.
* 対応形式: WAV, XWM.
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
* Media Foundationを利用したストリームサウンドの実装.
*
* ストリーミングを行うオーディオクラス.
* BGMに適している.
* 対応形式: WAV, MP3, AAC, WMA.
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

      // 読み込み位置をリセット.
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
* 音声再生の制御クラス.
*
* 初期化と終了:
* -# アプリケーションの初期化処理でEngine::Initialize()を呼び出す.
* -# アプリケーションの終了処理でEngine::Destroy()を呼び出す.
*
* 音声の再生:
* -# 音声ファイル名を引数にしてPrepare(ぷりぺあ)関数を呼び出すと、戻り値として「音声制御インターフェイス」が返されるので、これを変数に保存する.
* -# 音声制御インターフェイスに対してPlay関数を呼び出すと音声が再生される. Playを呼び出すたびに同じ音声が再生される.
* -# Playを呼び出す必要がなくなったら音声制御インターフェイスを破棄する.
* -# 使いまわしをしない音声の場合「engine.Prepare("OneTimeSound.wav")->Play()」のように書くことができる.
*/
class Engine : public std::enable_shared_from_this<Engine>
{
public:
  /**
  * 音声再生エンジンを初期化する.
  *
  * @retval true  初期化成功.
  * @retval false 初期化失敗.
  */
  bool Initialize() {
    if (xaudio) {
      std::cerr << "WARNING: Audio::Engineは初期化済みです." << std::endl;
      return true;
    }
    ComPtr<IXAudio2> tmpAudio;
    UINT32 flags = 0;
#ifdef ENABLE_XAUDIO_DEBUG 
    flags |= XAUDIO2_DEBUG_ENGINE;
#endif // ENABLE_XAUDIO_DEBUG
    HRESULT hr = XAudio2Create(&tmpAudio, flags);
    if (FAILED(hr)) {
      std::cerr << "ERROR: XAudio2の作成に失敗." << std::endl;
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
      std::cerr << "ERROR: XAudio2の音量設定に失敗." << std::endl;
      return false;
    }

    // Setup for Media foundation.
    mf = std::make_unique<MediaFoundationInitialize>();
    if (FAILED(MFCreateAttributes(attributes.GetAddressOf(), 1))) {
      std::cerr << "ERROR: Media Foundationの属性オブジェクトの作成に失敗." << std::endl;
      return false;
    }
    if (FAILED(attributes->SetUINT32(MF_LOW_LATENCY, true))) {
      std::cerr << "ERROR: Media Foundationのレイテンシの設定に失敗." << std::endl;
      return false;
    }
    xaudio.Swap(std::move(tmpAudio));

    return true;
  }

  /**
  * エンジンを破棄する.
  */
  void Destroy() {
    soundList.clear();
    mfSoundList.clear();
  }

  /**
  * エンジンの状態を更新する.
  *
  * 定期的に呼び出す必要がある.
  *
  * @retval true  更新成功.
  * @retval false 更新失敗.
  *
  * @note 現在は常にtrueを返す.
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
  * 音声を準備する(SJIS文字列用).
  *
  * @param filename 音声ファイルのパス(SJIS文字列).
  *
  * @return 音声オブジェクトへのポインタ.
  *
  * ワンショット再生だけなら次のように直接Play()を呼び出すこともできる.
  * @code
  * Audio::Engine::Instance().Prepare("ファイルパス")->Play()
  * @endcode
  *
  * 効果音のように何度も再生する音声、または停止の必要がある音声の場合、戻り値を変数に格納しておいて
  * 必要なタイミングで関数を呼ぶ.
  * @code
  * // 音声を準備し、戻り値の音声制御オブジェクトを変数seに格納.
  * Audio::SoundPtr se = Audio::Engine::Instance().Prepare("ファイルパス");
  * ～～～
  * // 変数seに格納した音声制御オブジェクトを使って再生.
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
    std::cerr << "ERROR: " << filename << "を読み込めません." << std::endl;
    return std::make_shared<NullSoundImpl>();
  }

  /**
  * 音声を準備する(UTF-16文字列用).
  *
  * @param filename 音声ファイルのパス(UTF-16文字列).
  *
  * @return 音声オブジェクトへのポインタ.
  *
  * 現在のところ、この関数はMedia Foundation未対応のため、通常はSJIS文字列版のPrepareを使用すること.
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
  * ストリーミングを行う音声を準備する(UTF-16文字列用).
  *
  * @param filename 音声ファイルのパス(UTF-16文字列).
  *
  * @return 音声オブジェクトへのポインタ.
  *
  * WindowsのMedia Foundationを利用してAACやMP3などの再生を可能にしている.
  * PrepareStreamとは異なり、複数音声の同時再生を行うことができる.
  *
  * @note 通常は、この関数ではなくPrepare関数を使うこと.
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
  * マスターボリュームを設定する.
  *
  * @param vol 設定する音量.
  */
  void SetMasterVolume(float vol) {
    if (xaudio) {
      masteringVoice->SetVolume(vol);
    }
  }

  /**
  * マスターボリュームを取得する.
  *
  * @return 設定されている音量.
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
* オーディオを初期化する
*/
bool Initialize()
{
  if (!engine) {
    engine = std::make_unique<Engine>();
  }
  return engine->Initialize();
}

/**
* オーディオを破棄する
*/
void Finalize()
{
  if (engine) {
    engine->Destroy();
    engine.reset();
  }
}

/**
* オーディオ状態の更新
*/
void Update()
{
  if (!engine) {
    return;
  }
  engine->Update();
}

/**
* プレイヤーを指定して再生
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
* ループ再生を停止する
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
* 再生中かどうか調べる
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
* 音量を変更する
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
* 音量を取得する
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
* 単発再生
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
* マスターボリュームを設定する
*/
void SetMasterVolume(float volume)
{
  if (!engine) {
    return;
  }
  engine->SetMasterVolume(volume);
}

/**
* マスターボリュームを取得する
*/
float GetMasterVolume()
{
  if (!engine) {
    return 0;
  }
  return engine->GetMasterVolume();
}

} // namespace Audio

