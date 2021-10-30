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
* オーディオ用エラーコールバック.
*
* @param errid  エラーの種類を示すID.
* @param p1     erridの補足情報その1.
* @param p2     erridの補足情報その2.
* @param parray (未使用).
*/
void AudioErrorCallback(const CriChar8* errid, CriUint32 p1, CriUint32 p2,
  CriUint32* parray)
{
  const CriChar8* err = criErr_ConvertIdToMessage(errid, p1, p2);
  std::cerr << err << std::endl;
}

/**
* オーディオ用のメモリ確保関数.
*
* @param obj  登録時に指定したユーザー引数.
* @param size 確保するバイト数.
*
* @return 確保したメモリのアドレス.
*/
void* AudioAllocate(void* obj, CriUint32 size)
{
  return operator new(size);
}

/**
* オーディオ用のメモリ解放関数.
*
* @param obj 登録時に指定したユーザー引数.
* @param ptr 開放するメモリのアドレス.
*/
void AudioDeallocate(void* obj, void* ptr)
{
  operator delete(obj);
}

} // namespace

/**
* 音声制御システムを初期化する
*
* @param acfPaht    全体設定を保持するACFファイルのパス
* @param dspBusName 音声システムで使用するDSPバス名
*
* @retval true  初期化成功
* @retval false 初期化失敗
*/
bool Audio::Initialize(const char* acfPath, const char* dspBusName)
{
  if (!audio) {
    audio = new Audio;

    // エラーコールバック関数を登録する
    criErr_SetCallback(AudioErrorCallback);

    // メモリ管理関数を登録する
    criAtomEx_SetUserAllocator(AudioAllocate, AudioDeallocate, nullptr);

    // 初期化パラメータを設定する
    CriAtomExConfig_WASAPI libConfig;
    criAtomEx_SetDefaultConfig_WASAPI(&libConfig);

    // ローダー数を設定する
    CriFsConfig fsConfig;
    criFs_SetDefaultConfig(&fsConfig);
    fsConfig.num_loaders = 40; // num_voicesより大きい値を設定すること
    libConfig.atom_ex.fs_config = &fsConfig;

    // 再生制御可能な音声の最大数. 実際の発音数はボイスプールのnum_voicesで指定する
    libConfig.atom_ex.max_virtual_voices = 64;

    // OpenGL用に右手座標系を指定
    libConfig.atom_ex.coordinate_system = CRIATOMEX_COORDINATE_SYSTEM_RIGHT_HANDED;

    // Atomライブラリを初期化
    criAtomEx_Initialize_WASAPI(&libConfig, nullptr, 0);

    // ストリーミング用バッファを作成
    audio->dbasId = criAtomDbas_Create(nullptr, nullptr, 0);

    // ACFファイルを読み込む
    if (criAtomEx_RegisterAcfFile(nullptr, acfPath, nullptr, 0) == CRI_FALSE) {
      std::cerr << "[エラー]" << __func__ << ":" << acfPath << "の読み込みに失敗\n";
      Finalize();
      return false;
    }

    // DSPバスを割り当てる
    criAtomEx_AttachDspBusSetting(dspBusName, nullptr, 0);

    // ボイスプールを設定する
    CriAtomExStandardVoicePoolConfig svpConfig;
    criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&svpConfig);
    svpConfig.num_voices = libConfig.atom_ex.max_virtual_voices / 2; // 同時発音数
    svpConfig.player_config.streaming_flag = CRI_TRUE; // ストリーミング再生を有効化
    svpConfig.player_config.max_sampling_rate =
      48000 * 2; // 最大サンプリングレート、ピッチ変更を考慮してCD音質の2倍を設定
    audio->voicePool = criAtomExVoicePool_AllocateStandardVoicePool(&svpConfig, nullptr, 0);

    // 再生制御用プレイヤーを作成する
    audio->players.resize(8);
    for (auto& e : audio->players) {
      e = criAtomExPlayer_Create(nullptr, nullptr, 0);
    }

    // acb読み込み配列を確保
    // ここで指定した数は、同時に読み込み可能なACBファイルの最大数になる
    // Loadで配列に読み込み、Unloadで破棄する
    audio->acbList.resize(16);

    // キューIDとacbファイルの対応表を確保
    // キューIDを添え字に使うことで対応するacbファイルを取得できる
    // サウンド再生にははキューIDとacbファイルのペアが必要なため
    // Loadで対応表に追加され、Unloadで削除される
    audio->cueIdToAcbMap.resize(4096);

    std::cerr << "[情報]" << __func__ << ": オーディオ機能を初期化\n";
  }
  return true;
}

/**
* 音声制御システムを破棄する
*/
void Audio::Finalize()
{
  // 既に破棄されていたら何もしない
  if (!audio) {
    return;
  }

  // すべてのプレイヤーを破棄
  for (auto& e : audio->players) {
    if (e) {
      criAtomExPlayer_Destroy(e);
      e = nullptr;
    }
  }

  // すべてのACBファイルを破棄
  for (auto& e : audio->acbList) {
    if (e) {
      criAtomExAcb_Release(e);
      e = nullptr;
    }
  }

  // キューIDとACBの対応表を初期化
  std::fill(audio->cueIdToAcbMap.begin(), audio->cueIdToAcbMap.end(), nullptr);

  // ボイスプールを破棄
  if (audio->voicePool) {
    criAtomExVoicePool_Free(audio->voicePool);
    audio->voicePool = nullptr;
  }

  // ACFファイルの登録を解除
  criAtomEx_UnregisterAcf();

  // DBASを破棄
  if (audio->dbasId != CRIATOMDBAS_ILLEGAL_ID) {
    criAtomDbas_Destroy(audio->dbasId);
    audio->dbasId = CRIATOMDBAS_ILLEGAL_ID;
  }

  // ADXLEを終了
  criAtomEx_Finalize_WASAPI();

  delete audio;
  audio = nullptr;

  std::cerr << "[情報]" << __func__ << ": オーディオ機能を破棄\n";
}

/**
* 音声制御クラスを取得する
*
* @return 音声制御クラスのインスタンスの参照
*/
Audio& Audio::Get()
{
  return *audio;
}

/**
* 音声ファイルを読み込む
*
* @param index   読み込み先のACB配列の番号
* @param acbPath ACBファイルのパス名
* @param awbPath AWBファイルのパス名.
*
* @retval true  読み込み成功
* @retval false 読み込み失敗
*/
bool Audio::Load(size_t index, const char* acbPath, const char* awbPath)
{
  if (index >= acbList.size()) {
    std::cerr << "[エラー]" << __func__ << ":" <<
      acbPath << "のインデックスが大きすぎます\n";
    return false;
  }

  // 念のため読み込み先の要素を解放
  Unload(index);

  // 音声ファイルを読み込む
  acbList[index] = criAtomExAcb_LoadAcbFile(
    nullptr, acbPath, nullptr, awbPath, nullptr, 0);
  if (!acbList[index]) {
    std::cerr << "[エラー]" << __func__ << ":" << acbPath << "の読み込みに失敗\n";
    return false;
  }

  // キューIDとACBの対応表を更新
  const CriSint32 numCues = criAtomExAcb_GetNumCues(acbList[index]);
  for (int i = 0; i < numCues; ++i) {
    // キュー情報を取得
    CriAtomExCueInfo cueInfo;
    if (!criAtomExAcb_GetCueInfoByIndex(acbList[index], i, &cueInfo)) {
      std::cerr << "[警告]" << __func__ << ":" << acbPath << "の" <<
        i << "番目のキュー情報を取得できません\n";
      continue;
    }
    // 対応表よりキューIDが大きい場合は対応表のサイズを拡張
    if (cueIdToAcbMap.size() <= static_cast<size_t>(cueInfo.id)) {
      cueIdToAcbMap.resize(cueInfo.id + 1);
    }
    // キューIDとACBを対応付ける
    cueIdToAcbMap[cueInfo.id] = acbList[index];
  }

  return true;
}

/**
* オーディオファイルを破棄する
*
* @param index 破棄するACB配列の番号
*/
void Audio::Unload(size_t index)
{
  if (index >= acbList.size()) {
    std::cerr << "[エラー]" << __func__ << ":" <<
      "インデックスが大きすぎます\n";
    return;
  }

  if (acbList[index]) {
    // 対応表から破棄予定のACBハンドルを削除
    std::replace(cueIdToAcbMap.begin(), cueIdToAcbMap.end(),
      acbList[index], static_cast<CriAtomExAcbHn>(nullptr));

    // ACBハンドルを破棄
    criAtomExAcb_Release(acbList[index]);
    acbList[index] = nullptr;
  }
}

/**
* 音声システムの状態を更新する
*/
void Audio::Update()
{
  // Atomライブラリの状態を更新
  criAtomEx_ExecuteMain();
}

/**
* 音声を再生する
*
* @param playerId 再生に使用するプレイヤー番号
* @param cueId    再生するキューID
*/
void Audio::Play(size_t playerId, int cueId, float volume)
{
  // プレイヤー番号がプレイヤー数以上の場合は何もしない
  if (playerId >= players.size()) {
    return;
  }
  // 対応表がnullptrの場合は何もしない
  if (!cueIdToAcbMap[cueId]) {
    return;
  }

  criAtomExPlayer_SetVolume(players[playerId], volume);

  // プレイヤーにキューをセット
  criAtomExPlayer_SetCueId(players[playerId], cueIdToAcbMap[cueId], cueId);

  // セットしたキューを再生
  criAtomExPlayer_Start(players[playerId]);
}

/**
* 音声を停止する
*
* @param playerId 再生を停止するプレイヤー番号
*/
void Audio::Stop(size_t playerId)
{
  // プレイヤー番号がプレイヤー数以上の場合は何もしない
  if (playerId >= players.size()) {
    return;
  }

  // 再生を停止する
  criAtomExPlayer_Stop(players[playerId]);
}









