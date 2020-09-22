/**
* @file GameData.h
*/
#ifndef GAMEDATA_H_INCLUDED
#define GAMEDATA_H_INCLUDED
#include "glad/glad.h"
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"
#include "Actor.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <random>

/**
* ゲーム全体で使うデータ.
*/
class GameData
{
public:
  static GameData& Get();
  bool Initialize(GLFWwindow*);
  void Update();

  /**
   キーの種類.

   押されているキーの判定はビット演算子を使って:
     const uint32_t keyPressed = GameData::Get().keyPressed;
     if (keyPressed & GameData::Key::enter) {
       // Enterキーが押されている場合の処理.
     }
   のようにする.
   押された瞬間を知りたい場合はkeyPressedInLastFrameを使う.
  */
  enum Key {
    enter = 0b0000'0000'0000'0001, // Enterキー
    left  = 0b0000'0000'0000'0010, // 矢印キー(左)
    right = 0b0000'0000'0000'0100, // 矢印キー(右)
    up    = 0b0000'0000'0000'1000, // 矢印キー(上)
    down  = 0b0000'0000'0001'0000, // 矢印キー(下)
    shot  = 0b0000'0000'0010'0000, // 弾発射キー
  };
  uint32_t keyPressed = 0; // 押しているキー.
  uint32_t keyPressedInLastFrame = 0; // 最後のフレームで押されたキー.

  // プリミティブ番号.
  // プリミティブの読み込み順と一致させること.
  enum PrimNo {
    ground,
    tree,
    house,
    cube,
    plane,
    bullet,
    zombie_male_walk_0,
    zombie_male_walk_1,
    zombie_male_walk_2,
    zombie_male_walk_3,
    zombie_male_walk_4,
    zombie_male_walk_5,
    zombie_male_down_0,
    zombie_male_down_1,
    zombie_male_down_2,
    zombie_male_down_3,
    player_idle_0,
    player_idle_1,
    player_idle_2,
    player_run_0,
    player_run_1,
    player_run_2,
    player_run_3,
    player_run_4,
    player_run_5,
  };
  void Draw(PrimNo) const;

  std::shared_ptr<Shader::Pipeline> pipeline = nullptr;
  std::shared_ptr<Shader::Pipeline> pipelineSimple;
  std::shared_ptr<Shader::Pipeline> pipelineShadow;
  Mesh::PrimitiveBuffer primitiveBuffer;
  Texture::Sampler sampler;
  GLFWwindow* window = nullptr;

  std::mt19937 random;

  // アニメーションデータ.
  std::shared_ptr<Animation> anmZombieMaleWalk;
  std::shared_ptr<Animation> anmZombieMaleDown;
  std::shared_ptr<Animation> anmPlayerIdle;
  std::shared_ptr<Animation> anmPlayerRun;

  // 倒したゾンビの数.
  size_t killCount = 0;

private:
  GameData() = default;
  ~GameData();
  GameData(const GameData&) = delete;
  GameData& operator=(const GameData&) = delete;
};


#endif // GAMEDATA_H_INCLUDED
