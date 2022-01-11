/**
* @file Animation.h
*/
#ifndef ANIMATION_H_INCLUDED
#define ANIMATION_H_INCLUDED
#include <vector>
#include <memory>
#include <functional>

// 先行宣言
class Actor;
class AnimationCurve;
class AnimationClip;
class Animation;
using AnimationCurvePtr = std::shared_ptr<AnimationCurve>;
using AnimationClipPtr = std::shared_ptr<AnimationClip>;
using AnimationPtr = std::shared_ptr<Animation>;

#define ANIMATION_TARGET(type, target) ([](Actor& a, float v){ static_cast<type&>(a).target = v; })

/**
* 時間と値を関連付ける構造体
*
* キーフレームアニメーションにおいて、ある時間における値を表す
*/
struct KeyFrame
{
  float time;
  float value;
};

/**
* キーフレームの配列を管理するクラス
*
* 時間経過による値の連続的な変化を表す
*/
class AnimationCurve
{
public:
  // アニメーションカーブの作成
  static AnimationCurvePtr Create();
  static AnimationCurvePtr Create(std::initializer_list<KeyFrame> init);

  // コンストラクタ・デストラクタ
  AnimationCurve() = default;
  ~AnimationCurve() = default;

  // メンバ関数
  void SetKey(const KeyFrame& newKey);
  void SetKey(std::initializer_list<KeyFrame> newKeys);
  float Eval(float t) const;
  float GetStartTime() const;
  float GetEndTime() const;

private:
  std::vector<KeyFrame> keys; // 時刻ソート済みキーフレーム配列
};

/**
* 複数のアニメーションカーブを管理するクラス
*
* アクターのアニメーションに使用する
*/
class AnimationClip
{
public:
  // 値を反映するための関数型
  using FuncType = std::function<void(Actor&, float)>;

  // アニメーションクリップの作成
  static AnimationClipPtr Create();

  // コンストラクタ・デストラクタ
  AnimationClip() = default;
  ~AnimationClip() = default;

  // メンバ関数
  void SetCurve(FuncType func, const AnimationCurvePtr& curve);
  void ClearCurves();
  void Eval(Actor& actor, float t) const;
  float GetStartTime() const;
  float GetEndTime() const;

private:
  struct Data
  {
    FuncType func; // 値を反映するための関数
    AnimationCurvePtr curve; // カーブへのポインタ
  };
  std::vector<Data> curves;
};

/**
* アニメーションを制御するクラス
*/
class Animation
{
public:
  // アニメーションの作成
  static AnimationPtr Create();

  // コンストラクタ・デストラクタ
  Animation() = default;
  ~Animation() = default;

  // メンバ関数
  void SetActor(Actor* actor);
  void SetClip(const AnimationClipPtr& p);
  void Update(float deltaTime);
  void Play();
  void Pause();
  void Stop();
  float GetTime() const;
  void SetTime(float time);
  float GetLength() const;
  bool IsPlaying() const;
  bool IsEnd() const;

  enum class WrapMode {
    once, // アニメーションの最後に達すると再生を停止する
    loop, // アニメーションの最後に達すると先頭に戻り再生を続ける
  };
  WrapMode GetWrapMode() const { return wrapMode; }
  void SetWrapMode(WrapMode mode) { wrapMode = mode; }

private:
  Actor* actor = nullptr; // アニメーション対象アクター
  AnimationClipPtr clip;  // アクターに反映するアニメーション
  float length = 0;       // アニメーションの長さ(秒)
  float time = 0;         // 再生時刻(秒)
  bool isPlaying = false; // 再生中ならtrue
  bool isPause = false;   // 一時停止中ならtrue
  WrapMode wrapMode = WrapMode::once; // ループ再生の種類
};

#endif // ANIMATION_H_INCLUDED
