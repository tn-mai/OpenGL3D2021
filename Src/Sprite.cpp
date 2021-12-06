/**
* @file Sprite.cpp
*/
#include "Sprite.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>

/**
* �R���X�g���N�^
*/
Sprite::Sprite(const glm::vec3& position, std::shared_ptr<Texture> tex,
  const glm::vec2& uv0, const glm::vec2& uv1, float pixelsPerMeter) :
  Actor("Sprite", Primitive(), tex, position, glm::vec3(1), 0, glm::vec3(0)),
  uv0(uv0), uv1(uv1), pixelsPerMeter(pixelsPerMeter)
{
  layer = Layer::Sprite;
}

/**
* VAO�ɒ��_�A�g���r���[�g��ݒ肷��
*/
void SetVertexAttribute(GLuint vao, GLuint index, 
  GLint size, GLenum type, GLboolean normalized,
  GLuint relativeOffset, GLuint bindingPoint)
{
  glEnableVertexArrayAttrib(vao, index);
  glVertexArrayAttribFormat(vao, index, size, type, normalized, relativeOffset);
  glVertexArrayAttribBinding(vao, index, bindingPoint);
}

/**
* �X�v���C�g�p�̒��_�f�[�^
*/
struct SpriteVertex
{
  glm::vec3   position;
  glm::u8vec4 color;
  glm::vec2   texcoord;
};

/**
* �X�v���C�g�p�̃��������m�ۂ���
*
* @param maxSpriteCount �i�[�\�ȍő�X�v���C�g��
*
* @retval true  �m�ې���
* @retval false �m�ێ��s�A�܂��͊��Ɋm�ۍς�
*/
bool SpriteRenderer::Allocate(size_t maxSpriteCount)
{
  // �Ō��vao�����݂���ꍇ�͍쐬�ς�
  if (buffer[std::size(buffer) - 1].vao) {
    std::cerr << "[�x��]" << __func__ << ": VAO�͍쐬�ς݂ł�\n";
    return false;
  }

  // IBO���쐬
  const size_t maxVertexNo = std::min<size_t>(maxSpriteCount * 4, 65536);
  std::vector<GLushort> indices(maxVertexNo / 4 * 6);
  GLushort vertexNo = 0;
  for (size_t indexNo = 0; indexNo < indices.size(); indexNo += 6) {
    // �O�p�`1��
    indices[indexNo + 0] = vertexNo + 0;
    indices[indexNo + 1] = vertexNo + 1;
    indices[indexNo + 2] = vertexNo + 2;

    // �O�p�`2��
    indices[indexNo + 3] = vertexNo + 2;
    indices[indexNo + 4] = vertexNo + 3;
    indices[indexNo + 5] = vertexNo + 0;

    vertexNo += 4;
  }
  ibo = GLContext::CreateBuffer(
    indices.size() * sizeof(GLushort), indices.data());

  // VBO���쐬
  const size_t vboSize = sizeof(SpriteVertex) * maxSpriteCount * 4;
  for (Buffer& e : buffer) {
    e.vbo = GLContext::CreateBuffer(vboSize, nullptr);
  }

  // VAO���쐬
  const GLuint bindingPoint = 0;
  for (Buffer& e : buffer) {
    glCreateVertexArrays(1, &e.vao);
    glVertexArrayElementBuffer(e.vao, ibo);
    glVertexArrayVertexBuffer(
      e.vao, bindingPoint, e.vbo, 0, sizeof(SpriteVertex));

    // ���_�A�g���r���[�g��ݒ�
    SetVertexAttribute(e.vao, 0, 3, GL_FLOAT, GL_FALSE,
      offsetof(SpriteVertex, position), bindingPoint);

    SetVertexAttribute(e.vao, 1, 4, GL_UNSIGNED_BYTE, GL_TRUE,
      offsetof(SpriteVertex, color), bindingPoint);

    SetVertexAttribute(e.vao, 2, 2, GL_FLOAT, GL_FALSE,
      offsetof(SpriteVertex, texcoord), bindingPoint);
  }

  primitives.reserve(100);
  this->maxSpriteCount = maxSpriteCount;

  return true;
}

/**
* �`��f�[�^��j����GPU���������������
*/
void SpriteRenderer::Deallocate()
{
  primitives.clear();
  maxSpriteCount = 0;
  updatingBufferIndex = 0;

  for (Buffer& e : buffer) {
    glDeleteVertexArrays(1, &e.vao);
    glDeleteBuffers(1, &e.vbo);
    e = Buffer();
  }
  glDeleteBuffers(1, &ibo);
  ibo = 0;
}

/**
* �`��f�[�^���X�V����
*
* @param sprites  �X�V����X�v���C�g�̔z��
* @param matView  �X�V�Ɏg�p����r���[�s��
*/
void SpriteRenderer::Update(
  const std::vector<std::shared_ptr<Actor>>& sprites, const glm::mat4& matView)
{
  // �v���~�e�B�u�f�[�^���폜
  primitives.clear();

  // �X�v���C�g���ЂƂ��Ȃ���Ή������Ȃ�
  if (sprites.empty()) {
    return;
  }

  // �X�v���C�g�̐�����������ꍇ�A�`�悷��X�v���C�g���𐧌�����
  size_t spriteCount = sprites.size();
  if (sprites.size() > maxSpriteCount) {
    std::cerr << "[�x��]" << __func__ <<
      ": �X�v���C�g�����������܂�(�v��=" << sprites.size() <<
      "/�ő�=" << maxSpriteCount << ")\n";
    spriteCount = maxSpriteCount;
  }

#if 1
  // �J��������̋����ƃX�v���C�g�̃A�h���X���y�A�ɂ��Ĕz��ɑ��
  struct SortingData {
    float z;
    const Sprite* sprite;
  };
  std::vector<SortingData> sortedSprites(sprites.size());
  for (size_t i = 0; i < sprites.size(); ++i) {
    const glm::vec3 p = matView * glm::vec4(sprites[i]->position, 1);
    sortedSprites[i].z = p.z;
    sortedSprites[i].sprite = static_cast<const Sprite*>(sprites[i].get());
  }

  // �z����J��������̋���(Z��)���ŕ��בւ���
  std::sort(sortedSprites.begin(), sortedSprites.end(),
    [](const SortingData& a, const SortingData& b) { return a.z < b.z; });
  sortedSprites.resize(spriteCount);

  // �ŏ��̃v���~�e�B�u���쐬
  const PrimitiveRenderer* renderer = static_cast<PrimitiveRenderer*>(
    sortedSprites[0].sprite->renderer.get());
  primitives.push_back(Primitive{ 0, 0, renderer->GetTexture() });
#else
  // �ŏ��̃v���~�e�B�u���쐬
  primitives.push_back(Primitive{ 0, 0, sprites[0]->tex });
#endif

  // �X�v���C�g���J�����Ɍ�����u�t�r���[��]�s��v���쐬����.
  // 1. ���s�ړ��������������邽��glm::mat3�R���X�g���N�^�ō���3x3���擾.
  // 2. �g��k���������������邽��inverse-transpose�ϊ����s��.
  // 3. �J�����̉�]��ł��������߁A��]�����̋t�s����쐬.
  const glm::mat3 matViewR = glm::transpose(glm::inverse(glm::mat3(matView)));
  const glm::mat4 matInvViewR = glm::inverse(matViewR);

  // ���ׂẴX�v���C�g�𒸓_�f�[�^�ɕϊ�
  std::vector<SpriteVertex> vertices(spriteCount * 4);
  for (int i = 0; i < spriteCount; ++i) {
#if 1
    const Sprite& sprite = *sortedSprites[i].sprite;
#else
    const Sprite& sprite = static_cast<Sprite&>(*sprites[i]);
#endif
    const PrimitiveRenderer* renderer = 
      static_cast<PrimitiveRenderer*>(sprite.renderer.get());
    std::shared_ptr<Texture> tex = renderer->GetTexture();

    // �\���T�C�Y���v�Z
    const float sx =
      sprite.scale.x * tex->GetWidth() / sprite.pixelsPerMeter;
    const float sy =
      sprite.scale.y * tex->GetHeight() / sprite.pixelsPerMeter;

    // ���W�ϊ��s����쐬
    const glm::mat4 matT = glm::translate(glm::mat4(1), sprite.position);
    const glm::mat4 matR =
      glm::rotate(glm::mat4(1), sprite.rotation, glm::vec3(0, 0, 1));
    const glm::mat4 matS = glm::scale(glm::mat4(1), glm::vec3(sx, sy, 1));
    const glm::mat4 matA = glm::translate(glm::mat4(1), sprite.adjustment);
    const glm::mat4 matModel = matT * matInvViewR * matR * matS * matA;

    // �F��vec4����u8vec4�ɕϊ�
    const glm::u8vec4 color = glm::clamp(sprite.color, 0.0f, 1.0f) * 255.0f;

    // �f�[�^�̊i�[�J�n�ʒuv���v�Z
    int v = i * 4;

    // �����̒��_�f�[�^���쐬
    vertices[v].position = matModel * glm::vec4(-0.5f, -0.5f, 0, 1);
    vertices[v].color = color;
    vertices[v].texcoord = sprite.uv0;
    ++v; // ���̊i�[�ʒu��

    // �E���̒��_�f�[�^���쐬
    vertices[v].position = matModel * glm::vec4(0.5f, -0.5f, 0, 1);
    vertices[v].color = color;
    vertices[v].texcoord = glm::vec2(sprite.uv1.x, sprite.uv0.y);
    ++v; // ���̊i�[�ʒu��

    // �E��̒��_�f�[�^���쐬
    vertices[v].position = matModel * glm::vec4(0.5f, 0.5f, 0, 1);
    vertices[v].color = color;
    vertices[v].texcoord = sprite.uv1;
    ++v; // ���̊i�[�ʒu��

    // ����̒��_�f�[�^���쐬
    vertices[v].position = matModel * glm::vec4(-0.5f, 0.5f, 0, 1);
    vertices[v].color = color;
    vertices[v].texcoord = glm::vec2(sprite.uv0.x, sprite.uv1.y);

    // �C���f�b�N�X�����X�V
    // �e�N�X�`�����������A�X�V��̃C���f�b�N�X����IBO�̋��e�l�ȉ��Ȃ�A
    // �C���f�b�N�X�����X�V����B�����łȂ���ΐV�����v���~�e�B�u��ǉ�����B
    const int maxCountPerPrimitive = 65536 / 4 * 6;
    Primitive& prim = primitives.back();
    if ((prim.texture == tex) &&
      (prim.count + 6 < maxCountPerPrimitive)) {
      prim.count += 6;
    } else {
      primitives.push_back(Primitive{ 6, i * 4, tex });
    }
  } // spriteCount

  // �������ݐ�̃o�b�t�@���`��Ɏg���Ă���ꍇ�A�`��̊�����҂�
  Buffer& buf = buffer[updatingBufferIndex];
  if (buf.sync) {
    const GLenum result = glClientWaitSync(buf.sync, 0, 0);
    switch (result) {
    case GL_ALREADY_SIGNALED:
      // ���Ɋ������Ă���(����)
      break;
    case GL_TIMEOUT_EXPIRED:
      std::cerr << "[�x��]" << __func__ << ":�`��Ɏ��Ԃ��������Ă��܂�\n";
      glClientWaitSync(buf.sync, 0, 1'000'000); // �ő�1�b�ԑ҂�
      break;
    default:
      std::cerr << "[�G���[]" << __func__ << ":�����Ɏ��s(" << result << ")\n";
      break;
    }
    glDeleteSync(buf.sync);
    buf.sync = 0;
  }

  // ���_�f�[�^��GPU�������ɃR�s�[
  CopyData(buf.vbo, sizeof(SpriteVertex), 0,
    vertices.size(), vertices.data());

  // �X�V�Ώۂ�؂�ւ���
  updatingBufferIndex = (updatingBufferIndex + 1) % std::size(buffer);
}

/**
* �X�v���C�g��`�悷��
*
* @param pipeline �`��Ɏg�p����O���t�B�b�N�X�p�C�v���C��
* @param matVP    �`��Ɏg�p����r���[�v���W�F�N�V�����s��
*/
void SpriteRenderer::Draw(
  std::shared_ptr<ProgramPipeline> pipeline,
  const glm::mat4& matVP)
{
  // �v���~�e�B�u���ЂƂ��Ȃ���Ή������Ȃ�
  if (primitives.empty()) {
    return;
  }

  // �p�C�v���C�����o�C���h
  const GLint locMatTRS = 0;
  pipeline->Bind();
  pipeline->SetUniform(locMatTRS, matVP);

  // �A���t�@�u�����f�B���O��L����
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // ���ʂ��`�悷��悤�ɐݒ�
  glDisable(GL_CULL_FACE);

  // �[�x�e�X�g�͍s�����A�������݂͂��Ȃ��悤�ɐݒ�
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_FALSE);

  // VAO���o�C���h
  const size_t index = (updatingBufferIndex + std::size(buffer) - 1) % std::size(buffer);
  glBindVertexArray(buffer[index].vao);

  // �`��f�[�^�����Ԃɕ`�悷��
  for (const Primitive& e : primitives) {
    e.texture->Bind(0);
    glDrawElementsBaseVertex(GL_TRIANGLES, e.count, GL_UNSIGNED_SHORT, nullptr, e.baseVertex);
  }

  // �`��̊������Ď�����u�����I�u�W�F�N�g�v���쐬
  buffer[index].sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

  // �e�N�X�`���̃o�C���h������
  primitives.back().texture->Unbind(0);

  // VAO�̃o�C���h������
  glBindVertexArray(0);

  // �`��ݒ��߂�
  glEnable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}
