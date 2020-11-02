/**
* @file PlayerActor.h
*/
#ifndef PLAYERACTOR_H_INCLUDED
#define PLAYERACTOR_H_INCLUDED
#include "../Actor.h"

class MainGameScene;

/**
* プレイヤーアクター.
*/
class PlayerActor : public Actor
{
public:
  PlayerActor(const glm::vec3& pos, float rotY, MainGameScene* pScene);
  virtual ~PlayerActor() = default;

  void ProcessInput();
  virtual void OnUpdate(float deltaTime) override;

private:
  MainGameScene* pMainGameScene = nullptr;

  std::shared_ptr<Texture::Image2D> texBullet;
  std::shared_ptr<Texture::Image2D> texGrenade;
  std::shared_ptr<Texture::Image2D> texWoodenBarrior;
  ActorPtr builderActor;

  // 連射用変数.
  float shotTimer = 0; // 次の弾を発射するまでの残り時間(秒).
  const float shotInterval = 0.1f; // 弾の発射間隔(秒).
  int leftOfRounds = 0; // 弾の残り連射回数.
  const int maxRounds = 3; // 1回のボタン入力で発射される弾数.
};

#endif // PLAYERACTOR_H_INCLUDED
