/**
* @file Animation.h
*/
#ifndef ANIMATION_H_INCLUDED
#define ANIMATION_H_INCLUDED
#include <vector>
#include <memory>
#include <algorithm>
#include <functional>

// ��s�錾
class Actor;
class AnimationCurve;
class AnimationClip;
class Animator;
using AnimationCurvePtr = std::shared_ptr<AnimationCurve>;
using AnimationClipPtr = std::shared_ptr<AnimationClip>;
using AnimatorPtr = std::shared_ptr<Animator>;

#define ANIMATION_TARGET(type, target) ([](Actor& a){ return &static_cast<type&>(a).target; })

/**
* ���Ԃƒl���֘A�t����\����
*
* �L�[�t���[���A�j���[�V�����ɂ����āA���鎞�Ԃɂ�����l��\��
*/
struct KeyFrame
{
  float time;
  float value;
};

/**
* �L�[�t���[���̔z����Ǘ�����N���X
*
* ���Ԍo�߂ɂ��l�̘A���I�ȕω���\��
*/
class AnimationCurve
{
public:
  static AnimationCurvePtr Create();
  static AnimationCurvePtr Create(std::initializer_list<KeyFrame> init);
  AnimationCurve() = default;
  ~AnimationCurve() = default;

  void AddKey(const KeyFrame& newKey);
  void AddKey(std::initializer_list<KeyFrame> init);
  void Eval(float* target, float t) const;
  float GetStartTime() const;
  float GetEndTime() const;

private:
  std::vector<KeyFrame> keys;
};

/**
* �����̃L�[�t���[���A�j���[�V�������Ǘ�����N���X
*
* �A�N�^�[�̃A�j���[�V�����Ɏg�p����
*/
class AnimationClip
{
public:
  using FuncType = std::function<float*(Actor&)>;

  static AnimationClipPtr Create();
  AnimationClip() = default;
  ~AnimationClip() = default;

  void AddCurve(FuncType func, const AnimationCurvePtr& tl);
  void ClearAllCurves();
  void Eval(Actor& actor, float t) const;
  float GetEndTime() const;

private:
  std::vector<std::pair<FuncType, AnimationCurvePtr>> curves;
};

/**
* �A�j���[�V�����𐧌䂷��N���X
*/
class Animator
{
public:
  static AnimatorPtr Create();
  Animator() = default;
  ~Animator() = default;

  void SetActor(Actor* actor);
  void SetClip(const AnimationClipPtr& p);
  void Update(float deltaTime);
  void Play() { isPlaying = true; }
  void Pause() { isPlaying = false; }
  void Stop() { isPlaying = false; time = 0; }
  void ResetTime() { time = 0; }
  float GetLength() const { return length; }
  float GetTime() const { return time; }
  bool IsEnd() const { return time >= length; }

private:
  Actor* actor = nullptr;
  AnimationClipPtr clip;
  float length = 0;
  float time = 0;
  bool isPlaying = false;
};

#endif // ANIMATION_H_INCLUDED
