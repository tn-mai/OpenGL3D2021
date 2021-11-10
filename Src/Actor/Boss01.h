/**
* @file Boss01.h
*/
#ifndef BOSS01_H_INCLUDED
#define BOSS01_H_INCLUDED
#include "../Actor.h"

/**
* ステージ1のボス敵
*/
class Boss01 : public Actor
{
public:
  Boss01(const glm::vec3& position, const glm::vec3& scale,
    float rotation, const std::shared_ptr<Actor>& target);
  virtual ~Boss01() = default;
  virtual std::shared_ptr<Actor> Clone() const override {
    return std::shared_ptr<Actor>(new Boss01(*this));
  }

  virtual void OnUpdate(float deltaTime) override;
  virtual void OnCollision(const struct Contact& contact) override;

  void SetTarget(std::shared_ptr<Actor> t) { target = t; }

private:
  // 動作モード
  void Idle(float deltaTime);
  void Danmaku(float deltaTime);
  void Machinegun(float deltaTime);
  void Missile(float deltaTime);

  using ModeFunc = void(Boss01::*)(float); // メンバ関数ポインタ型
  ModeFunc mode = &Boss01::Idle; // 現在の動作モード

  std::shared_ptr<Actor> target; // 追いかける対象のアクター
  float modeTimer = 0;     // 現在の動作モードが終了するまでの秒数
  float shotTimer = 0;     // 次の弾を発射するまでの秒数
  float shotDirection = 0; // 弾幕の発射方向
  int ammo = 0;            // マシンガンの連続発射数
};

#endif // BOSS01_H_INCLUDED
