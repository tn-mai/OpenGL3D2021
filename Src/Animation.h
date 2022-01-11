/**
* @file Animation.h
*/
#ifndef ANIMATION_H_INCLUDED
#define ANIMATION_H_INCLUDED
#include <vector>
#include <memory>
#include <functional>

// ��s�錾
class Actor;
class AnimationCurve;
class AnimationClip;
class Animation;
using AnimationCurvePtr = std::shared_ptr<AnimationCurve>;
using AnimationClipPtr = std::shared_ptr<AnimationClip>;
using AnimationPtr = std::shared_ptr<Animation>;

#define ANIMATION_TARGET(type, target) ([](Actor& a, float v){ static_cast<type&>(a).target = v; })

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
  // �A�j���[�V�����J�[�u�̍쐬
  static AnimationCurvePtr Create();
  static AnimationCurvePtr Create(std::initializer_list<KeyFrame> init);

  // �R���X�g���N�^�E�f�X�g���N�^
  AnimationCurve() = default;
  ~AnimationCurve() = default;

  // �����o�֐�
  void SetKey(const KeyFrame& newKey);
  void SetKey(std::initializer_list<KeyFrame> newKeys);
  float Eval(float t) const;
  float GetStartTime() const;
  float GetEndTime() const;

private:
  std::vector<KeyFrame> keys; // �����\�[�g�ς݃L�[�t���[���z��
};

/**
* �����̃A�j���[�V�����J�[�u���Ǘ�����N���X
*
* �A�N�^�[�̃A�j���[�V�����Ɏg�p����
*/
class AnimationClip
{
public:
  // �l�𔽉f���邽�߂̊֐��^
  using FuncType = std::function<void(Actor&, float)>;

  // �A�j���[�V�����N���b�v�̍쐬
  static AnimationClipPtr Create();

  // �R���X�g���N�^�E�f�X�g���N�^
  AnimationClip() = default;
  ~AnimationClip() = default;

  // �����o�֐�
  void SetCurve(FuncType func, const AnimationCurvePtr& curve);
  void ClearCurves();
  void Eval(Actor& actor, float t) const;
  float GetStartTime() const;
  float GetEndTime() const;

private:
  struct Data
  {
    FuncType func; // �l�𔽉f���邽�߂̊֐�
    AnimationCurvePtr curve; // �J�[�u�ւ̃|�C���^
  };
  std::vector<Data> curves;
};

/**
* �A�j���[�V�����𐧌䂷��N���X
*/
class Animation
{
public:
  // �A�j���[�V�����̍쐬
  static AnimationPtr Create();

  // �R���X�g���N�^�E�f�X�g���N�^
  Animation() = default;
  ~Animation() = default;

  // �����o�֐�
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
    once, // �A�j���[�V�����̍Ō�ɒB����ƍĐ����~����
    loop, // �A�j���[�V�����̍Ō�ɒB����Ɛ擪�ɖ߂�Đ��𑱂���
  };
  WrapMode GetWrapMode() const { return wrapMode; }
  void SetWrapMode(WrapMode mode) { wrapMode = mode; }

private:
  Actor* actor = nullptr; // �A�j���[�V�����ΏۃA�N�^�[
  AnimationClipPtr clip;  // �A�N�^�[�ɔ��f����A�j���[�V����
  float length = 0;       // �A�j���[�V�����̒���(�b)
  float time = 0;         // �Đ�����(�b)
  bool isPlaying = false; // �Đ����Ȃ�true
  bool isPause = false;   // �ꎞ��~���Ȃ�true
  WrapMode wrapMode = WrapMode::once; // ���[�v�Đ��̎��
};

#endif // ANIMATION_H_INCLUDED
