/**
* @file Animation.cpp
*/
#include "Animation.h"
#include <glm/glm.hpp>

/**
* �A�j���[�V�����J�[�u�I�u�W�F�N�g���쐬����
*/
AnimationCurvePtr AnimationCurve::Create()
{
  return std::make_shared<AnimationCurve>();
}

/**
* �A�j���[�V�����J�[�u�I�u�W�F�N�g���쐬����
*/
AnimationCurvePtr AnimationCurve::Create(
  std::initializer_list<KeyFrame> init)
{
  auto p = Create();
  p->AddKey(init);
  return p;
}

/**
* �L�[�t���[����ǉ�����
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
* �L�[�t���[����ǉ�����
*/
void AnimationCurve::AddKey(std::initializer_list<KeyFrame> init)
{
  for (const KeyFrame& key : init) {
    AddKey(key);
  }
}

/**
* �w�肳�ꂽ�A�h���X�ɃA�j���[�V�����𔽉f����
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
* �A�j���[�V�����J�[�u�̊J�n���Ԃ��擾����
*/
float AnimationCurve::GetStartTime() const
{
  return keys.empty() ? 0.0f : keys.front().time;
}

/**
* �A�j���[�V�����J�[�u�̏I�����Ԃ��擾����
*/
float AnimationCurve::GetEndTime() const
{
  return keys.empty() ? 0.0f : keys.back().time;
}

/**
* �A�j���[�V�����N���b�v�I�u�W�F�N�g���쐬����
*/
AnimationClipPtr AnimationClip::Create()
{
  return std::make_shared<AnimationClip>();
}

/**
* �A�j���[�V�����J�[�u��ǉ�����
*/
void AnimationClip::AddCurve(FuncType func, const AnimationCurvePtr& tl)
{
  curves.emplace_back(func, tl);
}

/**
* ���ׂẴA�j���[�V�����J�[�u���폜���� 
*/
void AnimationClip::ClearAllCurves()
{
  curves.clear();
}

/**
* �A�j���[�V�������A�N�^�[�ɔ��f����
*/
void AnimationClip::Eval(Actor& actor, float t) const
{
  for (auto& e : curves) {
    e.second->Eval(e.first(actor), t);
  }
}

/**
* �A�j���[�V�����N���b�v�̏I�����Ԃ��擾����
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
* �A�j���[�^�[�I�u�W�F�N�g���쐬����
*/
AnimatorPtr Animator::Create()
{
  return std::make_shared<Animator>();
}

/**
* �A�j���[�V������K�p����A�N�^�[��ݒ肷��
*/
void Animator::SetActor(Actor* actor)
{
  this->actor = actor;
}

/**
* �Đ�����A�j���[�V�����N���b�v��ݒ肷��
*/
void Animator::SetClip(const AnimationClipPtr& p)
{
  clip = p;
  length = p->GetEndTime();
}

/**
* �A�j���[�V�������X�V����
*/
void Animator::Update(float deltaTime)
{
  if (!clip || !actor || !isPlaying) {
    return;
  }
  time = glm::min(time + deltaTime, length);
  clip->Eval(*actor, time);
}

