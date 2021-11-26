/**
* @file Sprite.h
*/
#ifndef SPRITE_H_INCLUDED
#define SPRITE_H_INCLUDED
#include "Actor.h"

/**
* �X�v���C�g
*/
class Sprite : public Actor
{
public:
  Sprite(const glm::vec3& position, std::shared_ptr<Texture> tex,
    const glm::vec2& uv0 = glm::vec2(0), const glm::vec2& uv1 = glm::vec2(1),
    float pixelsPerMeter = 100.0f);
  virtual ~Sprite() = default;
  Sprite(const Sprite&) = default;
  Sprite& operator=(const Sprite&) = default;

  virtual std::shared_ptr<Actor> Clone() const {
    return std::make_shared<Sprite>(*this);
  }

  glm::vec2 uv0 = glm::vec2(0); // �摜�̍����e�N�X�`�����W
  glm::vec2 uv1 = glm::vec2(1); // �摜�̉E��e�N�X�`�����W
  float pixelsPerMeter = 100.0f;
};

/**
* �X�v���C�g��`�悷��N���X
*/
class SpriteRenderer
{
public:
  SpriteRenderer() = default;
  ~SpriteRenderer() { Deallocate(); }
  SpriteRenderer(const SpriteRenderer&) = delete;
  SpriteRenderer& operator=(const SpriteRenderer&) = delete;

  // �o�b�t�@�I�u�W�F�N�g�Ǘ�
  bool Allocate(size_t maxSpriteCount);
  void Deallocate();

  // �v���~�e�B�u�̍X�V
  void Update(
    const std::vector<std::shared_ptr<Actor>>& sprites,
    const glm::mat4& matView);

  // �v���~�e�B�u�̕`��
  void Draw(std::shared_ptr<ProgramPipeline> pipeline,
    const glm::mat4& matVP);

private:
  GLuint ibo = 0;
  struct Buffer {
    GLuint vbo = 0;
    GLuint vao = 0;
    GLsync sync = 0;
  } buffer[2];
  size_t maxSpriteCount = 0; // �`��ł���ő�X�v���C�g��
  size_t updatingBufferIndex = 0; // �X�V����o�b�t�@�I�u�W�F�N�g�̔ԍ�

  // �v���~�e�B�u�̕`����
  struct Primitive {
    GLsizei count; // �C���f�b�N�X��.
    GLint baseVertex; // �C���f�b�N�X0�ɑΉ����钸�_�f�[�^�̈ʒu
    std::shared_ptr<Texture> texture; // �`��Ɏg���e�N�X�`��
  };
  std::vector<Primitive> primitives; // �v���~�e�B�u�z��
};

#endif // SPRITE_H_INCLUDED
