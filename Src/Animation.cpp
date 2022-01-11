/**
* @file Animation.cpp
*/
#include "Animation.h"
#include <algorithm>
#include <iostream>

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
  p->SetKey(init);
  return p;
}

/**
* �L�[�t���[����ݒ肷��
*/
void AnimationCurve::SetKey(const KeyFrame& newKey)
{
  // �z�񂪁u���Ԃ̏����Ƀ\�[�g���ꂽ��ԁv���ێ��ł���悤�ȑ}���ʒu��񕪒T��
  auto pos = keys.begin();
  auto last = keys.end();
  while (pos != last) {
    const auto mid = pos + (last - pos) / 2;
    // ���̏��������u�������Ȃ��v�ŏ��̗v�f����������
    // �������̏������͂ƂĂ��d�v�B������ς���ƖړI�Ƃ���}���ʒu�𓾂��Ȃ��B
    if (mid->time < newKey.time) {
      pos = mid + 1;
    } else {
      last = mid;
    }
  }

  // �ǉ�����ʒu���E�E�E
  // �u�I�[�v�̏ꍇ: �z��̖����ɐV�����L�[��ǉ�����
  // �u���Ԃ̈قȂ�L�[�v�̏ꍇ: �u�ǉ�����ʒu�v�̎�O�ɐV�����L�[��ǉ�����
  // �u���Ԃ̓������L�[�v�̏ꍇ: �l���㏑�����邾���Œǉ��͂��Ȃ�
  if (pos == keys.end()) {
    keys.push_back(newKey);
  } else if(pos->time != newKey.time) {
    keys.insert(pos, newKey);
  } else {
    pos->value = newKey.value;
  }
}

/**
* �L�[�t���[����ݒ肷��
*/
void AnimationCurve::SetKey(std::initializer_list<KeyFrame> newKeys)
{
  for (const KeyFrame& key : newKeys) {
    SetKey(key);
  }
}

/**
* �w�肳�ꂽ�A�h���X�ɃA�j���[�V�����𔽉f����
*/
float AnimationCurve::Eval(float t) const
{
  // �L�[���ЂƂ��Ȃ����0��Ԃ�
  if (keys.empty()) {
    return 0.0f;
  }

  // t���ŏ��̃L�[�̎����ȉ��Ȃ�A�ŏ��̃L�[�̒l��Ԃ�
  if (t <= keys.front().time) {
    return keys.front().value;
  }

  // t���Ō�̃L�[�̎����ȏ�Ȃ�A�Ō�̃L�[�̒l��Ԃ�
  if (t >= keys.back().time) {
    return keys.back().value;
  }

  // �����܂ŗ����Ȃ�keys�ɂ́A't > key.time'�𖞂����L�[A�ƁA
  // 't < key.time'�𖞂����L�[B�����݂���B
  // ����L�̉���͖{���ɐ������ł��傤���H

  // t�ȏ�̎��Ԃ����L�[��T������
  auto pos = keys.begin();
  auto last = keys.end();
  while (pos != last) {
    const auto mid = pos + (last - pos) / 2;
    // ���̏��������u�������Ȃ��v�ŏ��̗v�f����������
    if (mid->time < t) {
      pos = mid + 1;
    } else {
      last = mid;
    }
  }

  // ���ȉ��̎������s���G���[���N�����Ȃ����Ƃ�ۏ؂���ɂ́A
  //   pos�̎�O��1�ȏ�̗v�f�����݂��邱�Ƃ������ł��B
  //   ���̏����͕ۏ؂���Ă���ł��傤���H
  const auto prev = pos - 1; // ����t�ȉ��̃L�[

  // prev��pos�̊Ԃ���`��Ԃ��邽�߂̔䗦���v�Z����
  // ���ϐ�ratio�̒l��0.0�`1.0�͈̔͂ɂȂ��Ă��Ȃ��Ă͂Ȃ�܂���B
  //   ���̏����͖��������ł��傤���H
  const float length = pos->time - prev->time;
  t = (t - prev->time) / length;

  // ���`��Ԃ̌��ʂ�Ԃ�
  return prev->value * (1.0f - t) + pos->value * t;
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
void AnimationClip::SetCurve(FuncType func, const AnimationCurvePtr& curve)
{
  curves.push_back(Data{ func, curve });
}

/**
* ���ׂẴA�j���[�V�����J�[�u���폜���� 
*/
void AnimationClip::ClearCurves()
{
  curves.clear();
}

/**
* �A�j���[�V�������A�N�^�[�ɔ��f����
*/
void AnimationClip::Eval(Actor& actor, float t) const
{
  for (auto& e : curves) {
    const float value = e.curve->Eval(t);
    e.func(actor, value);
  }
}

/**
* �A�j���[�V�����N���b�v�̊J�n���Ԃ��擾����
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
* �A�j���[�V�����N���b�v�̏I�����Ԃ��擾����
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
* �A�j���[�^�[�I�u�W�F�N�g���쐬����
*/
AnimationPtr Animation::Create()
{
  return std::make_shared<Animation>();
}

/**
* �A�j���[�V������K�p����A�N�^�[��ݒ肷��
*/
void Animation::SetActor(Actor* actor)
{
  this->actor = actor;
}

/**
* �Đ�����A�j���[�V�����N���b�v��ݒ肷��
*/
void Animation::SetClip(const AnimationClipPtr& p)
{
  clip = p;
  length = p->GetEndTime();
}

/**
* �A�j���[�V�������X�V����
*/
void Animation::Update(float deltaTime)
{
  if (!clip || !actor) {
    return;
  }

  // �Đ������|�[�Y���Ă��Ȃ��Ȃ玞�Ԃ�i�߂�
  if (isPlaying && !isPause) {
    time += deltaTime;
  }

  switch (wrapMode) {
  case WrapMode::once:
    clip->Eval(*actor, time);
    break;

  case WrapMode::loop: {
    float t = std::fmod(time, length);
    // t�������̏ꍇ�Afmod��length�����̕������u�]��v�Ƃ��ĕԂ�
    // ���������[�v�����ł�t�͐����ɂȂ��Ă��Ăق���
    // �����ŁAlength�����Z���Đ����ɕϊ�����
    if (t < 0) {
      t += length;
    }
    clip->Eval(*actor, t);
    break;
  }
  }
}

/**
* �Đ��J�n
*/
void Animation::Play()
{
  isPlaying = true; isPause = false;
}

/**
* �ꎞ��~
*/
void Animation::Pause()
{
  isPause = true;
}

/**
* ��~
*/
void Animation::Stop()
{
  isPlaying = false;
  isPause = false;
  time = 0;
}

/**
* �Đ��������擾����
*/
float Animation::GetTime() const
{
  return time;
}

/**
* �Đ�������ݒ肷��
*/
void Animation::SetTime(float time)
{
  this->time = time;
}

/**
* ���Đ����Ԃ��擾����
*/
float Animation::GetLength() const
{
  return length;
}

/**
* �Đ������ǂ����𒲂ׂ�
*
* @retval true  �Đ���
* @retval false �Đ����Ă��Ȃ�
*/
bool Animation::IsPlaying() const
{
  return isPlaying;
}

/**
* �Đ����������������ׂ�
*
* @retval true  �Đ�����
* @retval false �Đ����A�������͂܂��Đ����Ă��Ȃ�
*/
bool Animation::IsEnd() const
{
  return wrapMode == WrapMode::once && time >= length;
}

