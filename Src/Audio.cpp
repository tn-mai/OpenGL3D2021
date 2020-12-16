/**
* @file Audio.cpp
*/
#include "Audio.h"
#include "Audio/OpenGL3D2020_acf.h"
#include <algorithm>
#include <iostream>

const int audioMaxVirtualVoice = 64;
const int audioMaxVoice = audioMaxVirtualVoice / 2;
const int audioMaxLoader = audioMaxVoice + 8;

/**
* オーディオシステム用エラーコールバック.
*/
void Audio::ErrorCallback(const CriChar8* errid, CriUint32 p1, CriUint32 p2,
  CriUint32* parray)
{
  const CriChar8* err = criErr_ConvertIdToMessage(errid, p1, p2);
  std::cerr << err << std::endl;
}

/**
* オーディオシステム用アロケータ.
*/
void* Audio::Allocate(void* obj, CriUint32 size)
{
  return operator new(size);
}

/**
* オーディオシステム用デアロケータ.
*/
void Audio::Deallocate(void* obj, void* ptr)
{
  operator delete(obj);
}

/**
* 音声制御クラスを取得する.
*
* @return 音声制御クラスのインスタンスの参照.
*/
Audio& Audio::Instance()
{
  static Audio instance;
  return instance;
}

/**
* 音声制御システムを初期化する.
*
* @param acfPaht    全体設定を保持するACFファイルのパス.
* @param dspBusName 音声システムで使用するDSPバス名.
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool Audio::Initialize(const char* acfPath, const char* dspBusName)
{
  // 初期化済みなら何もしない.
  if (isInitialized) {
    return true;
  }

  // エラーコールバックとメモリ管理関数を登録する.
  criErr_SetCallback(ErrorCallback);
  criAtomEx_SetUserAllocator(Allocate, Deallocate, nullptr);

  // 初期化パラメータを設定してADX2 LEを初期化する.
  CriFsConfig fsConfig;
  criFs_SetDefaultConfig(&fsConfig);
  // ストリーム再生を有効にする場合、num_voicesより大きくすること.
  // デフォルトではnum_voicesが8、num_loadersが16なので、num_voices+8個あればよさそう.
  // 実験した限りでは+1個でもいけたが、何かの設定で増減するかもしれないので+8個が無難か.
  // max_virtual_voicesのデフォルトが16なのでこれと合わせる案もあるが、どちらが正解なのか...
  // ストリーム再生を有効にするとすべてのボイスにローダーが割り当てられるため.
  fsConfig.num_loaders = audioMaxLoader;
  CriAtomExConfig_WASAPI libConfig;
  criAtomEx_SetDefaultConfig_WASAPI(&libConfig);
  libConfig.atom_ex.fs_config = &fsConfig;
  // 発音制御可能な音声の最大数. 実際の発音数はボイスプールのnum_voicesで指定する.
  libConfig.atom_ex.max_virtual_voices = audioMaxVirtualVoice;
  // 右手座標系を指定.
  libConfig.atom_ex.coordinate_system = CRIATOMEX_COORDINATE_SYSTEM_RIGHT_HANDED;
  criAtomEx_Initialize_WASAPI(&libConfig, nullptr, 0);
  dbas = criAtomDbas_Create(nullptr, nullptr, 0);

  // 設定ファイルを読み込む.
  if (criAtomEx_RegisterAcfFile(nullptr, acfPath, nullptr, 0) == CRI_FALSE) {
    std::cerr << "[エラー]" << __func__ << ":" << acfPath << "を読み込めません.\n";
    Finalize();
    return false;
  }
  criAtomEx_AttachDspBusSetting(dspBusName, nullptr, 0);

  // 再生環境を設定する.
  CriAtomExStandardVoicePoolConfig svpConfig;
  criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&svpConfig);
  svpConfig.num_voices = audioMaxVoice; // 同時発音数.
  svpConfig.player_config.streaming_flag = CRI_TRUE; // ストリーミング再生を有効化.
  svpConfig.player_config.max_sampling_rate =
    CRIATOM_DEFAULT_INPUT_MAX_SAMPLING_RATE * 2; // 最大サンプリングレート. ピッチ変更を考慮してCD音質の2倍を設定.
  voicePool = criAtomExVoicePool_AllocateStandardVoicePool(&svpConfig, nullptr, 0);
  if (!voicePool) {
    std::cerr << "[エラー]" << __func__ << ":ボイスプールの作成に失敗.\n";
    Finalize();
    return false;
  }

  // 再生制御用プレイヤーを作成する.
  players.resize(8);
  for (auto& e : players) {
    e = criAtomExPlayer_Create(nullptr, nullptr, 0);
  }

  // acb読み込みバッファを確保.
  // ここで指定した数は、同時に読み込み可能なACBファイルの最大数になる.
  // Loadでバッファに読み込み、Unloadで破棄する.
  acbList.resize(16);

  // キューIDとacbファイルの対応表.
  // キューIdを添え字に使うことで対応するacbファイルを取得できる.
  // サウンド再生にははキューIDとacbファイルのペアが必要なため.
  // Loadで対応づけが行われ、Unloadで解除される.
  cueIdToAcbMap.resize(4096);

  isInitialized = true;
  return true;
}

/**
* 音声制御システムを破棄する.
*/
void Audio::Finalize()
{
  // すべてのプレイヤーを破棄.
  for (auto& e : players) {
    if (e) {
      criAtomExPlayer_Destroy(e);
    }
  }
  players.clear();

  // すべてのACBファイルを破棄.
  for (auto e : acbList) {
    if (e) {
      criAtomExAcb_Release(e);
    }
  }
  acbList.clear();

  // キューIDとACBの対応表を初期化.
  cueIdToAcbMap.clear();

  // ボイスプールを破棄.
  if (voicePool) {
    criAtomExVoicePool_Free(voicePool);
    voicePool = nullptr;
  }

  // ACFファイルの登録を解除.
  criAtomEx_UnregisterAcf();

  // DBASを破棄.
  if (dbas != CRIATOMDBAS_ILLEGAL_ID) {
    criAtomDbas_Destroy(dbas);
    dbas = CRIATOMDBAS_ILLEGAL_ID;
  }

  // ADX2LEを終了.
  criAtomEx_Finalize_WASAPI();

  isInitialized = false;
}

/**
* オーディオファイルを読み込む.
*
* @param index 読み込み先オーディオスロット番号.
*
* @retval true  読み込み成功.
* @retval false 読み込み失敗.
*/
bool Audio::Load(size_t index, const char* acbPath, const char* awbPath)
{
  if (index >= acbList.size()) {
    std::cerr << "[エラー]" << __func__ << "インデックスが大きすぎます.\n";
    return false;
  }

  Unload(index);

  acbList[index] = criAtomExAcb_LoadAcbFile(nullptr, acbPath, nullptr, awbPath, nullptr, 0);
  if (!acbList[index]) {
    std::cerr << "[エラー]" << __func__ << ":" << acbPath << "の読み込みに失敗.\n";
    return false;
  }

  // キューIDとACBファイルの対応表を作成する.
  const CriSint32 numCues = criAtomExAcb_GetNumCues(acbList[index]);
  for (int i = 0; i < numCues; ++i) {
    CriAtomExCueInfo cueInfo;
    if (!criAtomExAcb_GetCueInfoByIndex(acbList[index], i, &cueInfo)) {
      std::cerr << "[エラー]" << __func__ << ":" << acbPath << "の" << i << "番目のキュー情報を取得できません.\n";
      continue;
    }
    if (cueIdToAcbMap.size() <= static_cast<size_t>(cueInfo.id)) {
      cueIdToAcbMap.resize(cueInfo.id + 1);
    }
    cueIdToAcbMap[cueInfo.id] = acbList[index];
  }
  return true;
}

/**
* オーディオファイルを破棄する.
*
* @param index 破棄するオーディオスロット番号.
*/
void Audio::Unload(size_t index)
{
  if (index >= acbList.size()) {
    std::cerr << "[エラー]" << __func__ << "インデックスが大きすぎます.\n";
    return;
  }

  if (acbList[index]) {
    std::replace(cueIdToAcbMap.begin(), cueIdToAcbMap.end(),
      acbList[index], static_cast<CriAtomExAcbHn>(nullptr));
    criAtomExAcb_Release(acbList[index]);
    acbList[index] = nullptr;
  }
}

/**
* 音声システムの状態を更新する.
*/
void Audio::Update()
{
  if (!isInitialized) {
    return;
  }
  // 音声システムの状態を更新.
  criAtomEx_ExecuteMain();
}

/**
* 音声を再生する.
*
* @param playerId 再生に使用するプレイヤー番号.
* @param cueId    再生するキューID.
*/
void Audio::Play(size_t playerId, int cueId, float volume)
{
  // プレイヤー番号がプレイヤー数以上の場合は何もしない.
  if (playerId >= players.size()) {
    return;
  }
  // 対応表がnullptrの場合は何もしない.
  if (!cueIdToAcbMap[cueId]) {
    return;
  }

  // プレイヤーにキューをセット.
  criAtomExPlayer_SetCueId(players[playerId], cueIdToAcbMap[cueId], cueId);

  criAtomExPlayer_SetVolume(players[playerId], volume);

  // セットしたキューを再生.
  criAtomExPlayer_Start(players[playerId]);
}

/**
* 音声を停止する.
*
* @param playerId 再生を停止するプレイヤー番号.
*/
void Audio::Stop(size_t playerId)
{
  // プレイヤー番号がプレイヤー数以上の場合は何もしない.
  if (playerId >= players.size()) {
    return;
  }
  // 再生を停止する.
  criAtomExPlayer_Stop(players[playerId]);
}

/**
* 全体音量を設定する.
*
* @param volume 設定する全体音量(無音=0 標準音量=1).
*/
void Audio::SetMasterVolume(float volume)
{
  if (!isInitialized) {
    return;
  }
  masterVolume = volume;
  criAtomExCategory_SetVolumeById(CRI_OPENGL3D2020_ACF_CATEGORY_CATEGORY_0, masterVolume);
}

/**
* 全体音量を取得する.
*
* @return 設定されている全体音量(無音=1 標準音量=1).
*/
float Audio::GetMasterVolume() const
{
  return masterVolume;
}

