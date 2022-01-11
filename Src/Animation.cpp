/**
* @file Animation.cpp
*/
#include "Animation.h"
#include <algorithm>
#include <iostream>

/**
* アニメーションカーブオブジェクトを作成する
*/
AnimationCurvePtr AnimationCurve::Create()
{
  return std::make_shared<AnimationCurve>();
}

/**
* アニメーションカーブオブジェクトを作成する
*/
AnimationCurvePtr AnimationCurve::Create(
  std::initializer_list<KeyFrame> init)
{
  auto p = Create();
  p->SetKey(init);
  return p;
}

/**
* キーフレームを設定する
*/
void AnimationCurve::SetKey(const KeyFrame& newKey)
{
  // 配列が「時間の昇順にソートされた状態」を維持できるような挿入位置を二分探索
  auto pos = keys.begin();
  auto last = keys.end();
  while (pos != last) {
    const auto mid = pos + (last - pos) / 2;
    // 次の条件式を「満たさない」最初の要素を検索する
    // ※ここの条件式はとても重要。条件を変えると目的とする挿入位置を得られない。
    if (mid->time < newKey.time) {
      pos = mid + 1;
    } else {
      last = mid;
    }
  }

  // 追加する位置が・・・
  // 「終端」の場合: 配列の末尾に新しいキーを追加する
  // 「時間の異なるキー」の場合: 「追加する位置」の手前に新しいキーを追加する
  // 「時間の等しいキー」の場合: 値を上書きするだけで追加はしない
  if (pos == keys.end()) {
    keys.push_back(newKey);
  } else if(pos->time != newKey.time) {
    keys.insert(pos, newKey);
  } else {
    pos->value = newKey.value;
  }
}

/**
* キーフレームを設定する
*/
void AnimationCurve::SetKey(std::initializer_list<KeyFrame> newKeys)
{
  for (const KeyFrame& key : newKeys) {
    SetKey(key);
  }
}

/**
* 指定されたアドレスにアニメーションを反映する
*/
float AnimationCurve::Eval(float t) const
{
  // キーがひとつもなければ0を返す
  if (keys.empty()) {
    return 0.0f;
  }

  // tが最初のキーの時刻以下なら、最初のキーの値を返す
  if (t <= keys.front().time) {
    return keys.front().value;
  }

  // tが最後のキーの時刻以上なら、最後のキーの値を返す
  if (t >= keys.back().time) {
    return keys.back().value;
  }

  // ここまで来たならkeysには、't > key.time'を満たすキーAと、
  // 't < key.time'を満たすキーBが存在する。
  // ※上記の仮定は本当に正しいでしょうか？

  // t以上の時間を持つキーを探索する
  auto pos = keys.begin();
  auto last = keys.end();
  while (pos != last) {
    const auto mid = pos + (last - pos) / 2;
    // 次の条件式を「満たさない」最初の要素を検索する
    if (mid->time < t) {
      pos = mid + 1;
    } else {
      last = mid;
    }
  }

  // ※以下の式が実行時エラーを起こさないことを保証するには、
  //   posの手前に1つ以上の要素が存在することが条件です。
  //   この条件は保証されているでしょうか？
  const auto prev = pos - 1; // 時刻t以下のキー

  // prevとposの間を線形補間するための比率を計算する
  // ※変数ratioの値は0.0～1.0の範囲になっていなくてはなりません。
  //   この条件は満たされるでしょうか？
  const float length = pos->time - prev->time;
  t = (t - prev->time) / length;

  // 線形補間の結果を返す
  return prev->value * (1.0f - t) + pos->value * t;
}

/**
* アニメーションカーブの開始時間を取得する
*/
float AnimationCurve::GetStartTime() const
{
  return keys.empty() ? 0.0f : keys.front().time;
}

/**
* アニメーションカーブの終了時間を取得する
*/
float AnimationCurve::GetEndTime() const
{
  return keys.empty() ? 0.0f : keys.back().time;
}

/**
* アニメーションクリップオブジェクトを作成する
*/
AnimationClipPtr AnimationClip::Create()
{
  return std::make_shared<AnimationClip>();
}

/**
* アニメーションカーブを追加する
*/
void AnimationClip::SetCurve(FuncType func, const AnimationCurvePtr& curve)
{
  curves.push_back(Data{ func, curve });
}

/**
* すべてのアニメーションカーブを削除する 
*/
void AnimationClip::ClearCurves()
{
  curves.clear();
}

/**
* アニメーションをアクターに反映する
*/
void AnimationClip::Eval(Actor& actor, float t) const
{
  for (auto& e : curves) {
    const float value = e.curve->Eval(t);
    e.func(actor, value);
  }
}

/**
* アニメーションクリップの開始時間を取得する
*/
float AnimationClip::GetStartTime() const
{
  float time = 0;
  for (auto& e : curves) {
    time = std::min(time, e.curve->GetStartTime());
  }
  return time;
}

/**
* アニメーションクリップの終了時間を取得する
*/
float AnimationClip::GetEndTime() const
{
  float time = 0;
  for (auto& e : curves) {
    time = std::max(time, e.curve->GetEndTime());
  }
  return time;
}

/**
* アニメーターオブジェクトを作成する
*/
AnimationPtr Animation::Create()
{
  return std::make_shared<Animation>();
}

/**
* アニメーションを適用するアクターを設定する
*/
void Animation::SetActor(Actor* actor)
{
  this->actor = actor;
}

/**
* 再生するアニメーションクリップを設定する
*/
void Animation::SetClip(const AnimationClipPtr& p)
{
  clip = p;
  length = p->GetEndTime();
}

/**
* アニメーションを更新する
*/
void Animation::Update(float deltaTime)
{
  if (!clip || !actor) {
    return;
  }

  // 再生中かつポーズしていないなら時間を進める
  if (isPlaying && !isPause) {
    time += deltaTime;
  }

  switch (wrapMode) {
  case WrapMode::once:
    clip->Eval(*actor, time);
    break;

  case WrapMode::loop: {
    float t = std::fmod(time, length);
    // tが負数の場合、fmodはlength未満の負数を「余り」として返す
    // しかしループ処理ではtは正数になっていてほしい
    // そこで、lengthを加算して正数に変換する
    if (t < 0) {
      t += length;
    }
    clip->Eval(*actor, t);
    break;
  }
  }
}

/**
* 再生開始
*/
void Animation::Play()
{
  isPlaying = true; isPause = false;
}

/**
* 一時停止
*/
void Animation::Pause()
{
  isPause = true;
}

/**
* 停止
*/
void Animation::Stop()
{
  isPlaying = false;
  isPause = false;
  time = 0;
}

/**
* 再生時刻を取得する
*/
float Animation::GetTime() const
{
  return time;
}

/**
* 再生時刻を設定する
*/
void Animation::SetTime(float time)
{
  this->time = time;
}

/**
* 総再生時間を取得する
*/
float Animation::GetLength() const
{
  return length;
}

/**
* 再生中かどうかを調べる
*
* @retval true  再生中
* @retval false 再生していない
*/
bool Animation::IsPlaying() const
{
  return isPlaying;
}

/**
* 再生が完了したか調べる
*
* @retval true  再生完了
* @retval false 再生中、もしくはまだ再生していない
*/
bool Animation::IsEnd() const
{
  return wrapMode == WrapMode::once && time >= length;
}

