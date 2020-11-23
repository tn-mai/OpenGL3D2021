/**
* @file Sprite.h
*/
#ifndef SPRITE_H_INCLUDED
#define SPRITE_H_INCLUDED
#include "Texture.h"
#include "Shader.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory>

/**
* スプライト.
*/
class Sprite
{
public:
  Sprite() = default;
  Sprite(const glm::vec3& pos, std::shared_ptr<Texture::Image2D> tex,
    const glm::vec2& origin = glm::vec2(0),
    const glm::vec2& size = glm::vec2(1));
  
  void Update(float deltaTime);

  std::shared_ptr<Texture::Image2D> texture; // 画像を含むテクスチャ.
  glm::vec2 imageOrigin = glm::vec2(0);      // 画像の左下テクスチャ座標.
  glm::vec2 imageSize = glm::vec2(1);        // 画像の大きさ.

  glm::vec3 position = glm::vec3(0); // 座標.
  float rotation = 0;                // Z軸回転.
  glm::vec2 scale =glm::vec2(1);     // 拡大率.
  glm::vec4 color = glm::vec4(1);    // 色と不透明度.

  glm::vec3 velocity = glm::vec3(0); // 速度.
  float angularVelocity = 0;
  glm::vec2 scaleVelocity = glm::vec2(0);
  glm::vec4 colorVelocity = glm::vec4(0);

  float gravityScale = 0; // 重力の影響率.
  float lifespan = 0;     // 寿命.
  bool isDead = false;    // 死亡フラグ.

};

void UpdateSpriteList(std::vector<std::shared_ptr<Sprite>>&, float);

/**
* スプライト描画クラス.
*/
class SpriteRenderer
{
public:
  SpriteRenderer() = default;
  ~SpriteRenderer();
  SpriteRenderer(const SpriteRenderer&) = delete;
  SpriteRenderer& operator=(const SpriteRenderer&) = delete;

  // メモリ管理.
  bool Allocate(size_t maxSpriteCount);
  void Free();

  void Update(const std::vector<std::shared_ptr<Sprite>>& sprites, const glm::mat4& matView);
  void Draw(std::shared_ptr<Shader::Pipeline> pipeline, const glm::mat4& matVP) const;

private:
  GLuint ibo = 0;
  struct Buffer {
    GLuint vboPosition = 0;
    GLuint vboColor = 0;
    GLuint vboTexcoord = 0;
    GLuint vboNormal = 0;
    GLuint vao = 0;
  };
  Buffer buffers[2];
  size_t updatingBufferIndex = 0;

  size_t maxSpriteCount = 0; // 格納できる最大スプライト数.

  struct Primitive {
    GLsizei count;
    GLint baseVertex;
    std::shared_ptr<Texture::Image2D> texture;
  };
  std::vector<Primitive> primitives;
};

#endif // SPRITE_H_INCLUDED
