/**
* @file Animation.cpp
*/
#include "Animation.h"
#include <glm/glm.hpp>

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
  p->AddKey(init);
  return p;
}

/**
* キーフレームを追加する
*/
void AnimationCurve::AddKey(const KeyFrame& newKey)
{
  const auto itr = std::partition_point(
    keys.begin(), keys.end(),
    [&newKey](const KeyFrame& a) { return a.time < newKey.time; });
  if (itr == keys.end() || itr->time != newKey.time) {
    keys.insert(itr, newKey);
  }
}

/**
* キーフレームを追加する
*/
void AnimationCurve::AddKey(std::initializer_list<KeyFrame> init)
{
  for (const KeyFrame& key : init) {
    AddKey(key);
  }
}

/**
* 指定されたアドレスにアニメーションを反映する
*/
void AnimationCurve::Eval(float* target, float t) const
{
  const auto itr = std::partition_point(keys.begin(), keys.end(),
    [t](const KeyFrame& a) { return a.time <= t; });
  if (itr == keys.begin()) {
    *target = keys.front().value;
  } else if (itr == keys.end()) {
    *target = keys.back().value;
  } else {
    const auto itr0 = itr - 1;
    const float length = itr->time - itr0->time;
    t = glm::clamp((t - itr0->time) / length, 0.0f, 1.0f);
    *target = glm::mix(itr0->value, itr->value, t);
  }
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
void AnimationClip::AddCurve(FuncType func, const AnimationCurvePtr& tl)
{
  curves.emplace_back(func, tl);
}

/**
* すべてのアニメーションカーブを削除する 
*/
void AnimationClip::ClearAllCurves()
{
  curves.clear();
}

/**
* アニメーションをアクターに反映する
*/
void AnimationClip::Eval(Actor& actor, float t) const
{
  for (auto& e : curves) {
    e.second->Eval(e.first(actor), t);
  }
}

/**
* アニメーションクリップの終了時間を取得する
*/
float AnimationClip::GetEndTime() const
{
  float time = 0;
  for (auto& e : curves) {
    time = std::max(time, e.second->GetEndTime());
  }
  return time;
}

/**
* アニメーターオブジェクトを作成する
*/
AnimatorPtr Animator::Create()
{
  return std::make_shared<Animator>();
}

/**
* アニメーションを適用するアクターを設定する
*/
void Animator::SetActor(Actor* actor)
{
  this->actor = actor;
}

/**
* 再生するアニメーションクリップを設定する
*/
void Animator::SetClip(const AnimationClipPtr& p)
{
  clip = p;
  length = p->GetEndTime();
}

/**
* アニメーションを更新する
*/
void Animator::Update(float deltaTime)
{
  if (!clip || !actor || !isPlaying) {
    return;
  }
  time = glm::min(time + deltaTime, length);
  clip->Eval(*actor, time);
}

