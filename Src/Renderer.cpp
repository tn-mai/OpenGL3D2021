/**
* @file Renderer.cpp
*/
#include "Renderer.h"
#include "ProgramPipeline.h"
#include "Texture.h"
#include "Actor.h"
#include "GameEngine.h"
#include "GltfMesh.h"
#include "VertexArrayObject.h"
#include <glm/gtc/matrix_transform.hpp>
#include <numeric>
#include <algorithm>

/**
* �N���[�����쐬����
*/
RendererPtr PrimitiveRenderer::Clone() const
{
  return std::make_shared<PrimitiveRenderer>(*this);
}

/**
* �v���~�e�B�u��`�悷��
*/
void PrimitiveRenderer::Draw(const Actor& actor,
  const ProgramPipeline& pipeline, const glm::mat4& matVP)
{
  // ���f���s����v�Z����
  glm::mat4 matT = glm::translate(glm::mat4(1), actor.position);
  glm::mat4 matR = glm::rotate(glm::mat4(1), actor.rotation, glm::vec3(0, 1, 0));
  glm::mat4 matS = glm::scale(glm::mat4(1), actor.scale);
  glm::mat4 matA = glm::translate(glm::mat4(1), actor.adjustment);
  glm::mat4 matModel = matT * matR * matS * matA;

  // MVP�s����v�Z����
  glm::mat4 matMVP = matVP * matModel;

  // ���f���s���MVP�s���GPU�������ɃR�s�[����
  pipeline.SetUniform(locMatTRS, matMVP);
  if (actor.layer == Layer::Default) {
    pipeline.SetUniform(locMatModel, matModel);

    // �}�e���A���f�[�^��ݒ�
    const glm::uint texture = 0;
    pipeline.SetUniform(locMaterialColor, glm::vec4(1));
    pipeline.SetUniform(locMaterialTexture, &texture, 1);

    // �O���[�v�s���ݒ�
    constexpr glm::mat4 m[32] = {
      glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1),
      glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1),
      glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1),
      glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1), glm::mat4(1),
    };
    pipeline.SetUniform(locMatGroupModels, m, 32);
  }

  // TODO: �e�L�X�g���ǉ�
  const GLint locColor = 200;
  pipeline.SetUniform(locColor, actor.color);

  if (tex) {
    tex->Bind(0); // �e�N�X�`�������蓖�Ă�
  }
  prim.Draw();  // �v���~�e�B�u��`�悷��
}

/**
* �N���[�����쐬����
*/
RendererPtr MeshRenderer::Clone() const
{
  return std::make_shared<MeshRenderer>(*this);
}

/**
* ���b�V����`�悷��
*/
void MeshRenderer::Draw(const Actor& actor,
  const ProgramPipeline& pipeline, const glm::mat4& matVP)
{
  if (!mesh) {
    return;
  }

  // ���f���s����v�Z����
  glm::mat4 matT = glm::translate(glm::mat4(1), actor.position);
  glm::mat4 matR = glm::rotate(glm::mat4(1), actor.rotation, glm::vec3(0, 1, 0));
  glm::mat4 matS = glm::scale(glm::mat4(1), actor.scale);
  glm::mat4 matA = glm::translate(glm::mat4(1), actor.adjustment);
  glm::mat4 matModel = matT * matR * matS * matA;

  // MVP�s����v�Z����
  glm::mat4 matMVP = matVP * matModel;

  // GPU�������ɑ��邽�߂̃}�e���A���f�[�^���X�V
  if (materialChanged) {
    materialChanged = false;
    colors.resize(materials.size());
    for (int i = 0; i < materials.size(); ++i) {
      colors[i] = materials[i].color;
    }
    textures = GetTextureList(materials);
    textureIndices = GetTextureIndexList(materials, textures);
  }

  // ���f���s���MVP�s���GPU�������ɃR�s�[����
  pipeline.SetUniform(locMatTRS, matMVP);
  if (actor.layer == Layer::Default) {
    pipeline.SetUniform(locMatModel, matModel);

    // �}�e���A���f�[�^��ݒ�
    pipeline.SetUniform(locMaterialColor, colors.data(), colors.size());
    pipeline.SetUniform(locMaterialTexture,
      textureIndices.data(), textureIndices.size());

    // TODO: �e�L�X�g���ǉ�
    // �O���[�v�s���ݒ�
    const std::vector<glm::mat4> m = CalcGroupMatirices();
    pipeline.SetUniform(locMatGroupModels, m.data(), m.size());
  }

  // TODO: �e�L�X�g���ǉ�
  const GLint locColor = 200;
  pipeline.SetUniform(locColor, actor.color);

  const GLuint bindingPoints[] = { 0, 2, 3, 4, 5, 6, 7, 8 };
  const size_t size = std::min(textures.size(), std::size(bindingPoints));
  for (int i = 0; i < size; ++i) {
    textures[i]->Bind(bindingPoints[i]);
  }
  mesh->primitive.Draw();
}

/**
* ���b�V����ݒ肷��
*/
void MeshRenderer::SetMesh(const MeshPtr& p)
{
  mesh = p;
  if (mesh) {
    materials = mesh->materials;
    // �}�e���A�������݂��Ȃ����b�V���̏ꍇ�A�}�e���A�����Œ�1�����Ԃɂ���
    if (materials.empty()) {
      materials.push_back(Mesh::Material{});
    }
    matGroupModels.resize(mesh->groups.size(), glm::mat4(1));
    matGroupModels.shrink_to_fit();
    materialChanged = true;
  }
}

/**
* �e�q�֌W���l������N�Ԗڂ̃O���[�v�s����v�Z����
*/
void MeshRenderer::CalcNthGroupMatrix(int n,
  std::vector<glm::mat4>& m, std::vector<bool>& calculated) const
{
  // �v�Z�ς݂Ȃ牽�����Ȃ�
  if (calculated[n]) {
    return;
  }

  // N�Ԗڂ̍s���ݒ�
  m[n] = mesh->groups[n].matBindPose *  // �{���̈ʒu�ɖ߂�
    matGroupModels[n] *                 // ���W�ϊ����s��
    mesh->groups[n].matInverseBindPose; // ���_�Ɉړ�������

  // �e������ꍇ�A�e�̃O���[�v�s�����Z
  const int parent = mesh->groups[n].parent;
  if (parent != Mesh::Group::noParent) {
    CalcNthGroupMatrix(parent, m, calculated);
    m[n] = m[parent] * m[n];
  }

  // �v�Z�����������̂Ōv�Z�ς݃t���O�𗧂Ă�
  calculated[n] = true;
}

/**
* �e�q�֌W���l�����Ă��ׂẴO���[�v�s����v�Z����
*/
std::vector<glm::mat4> MeshRenderer::CalcGroupMatirices() const 
{
  // ���b�V�����ݒ肳��Ă��Ȃ���΋�̔z���Ԃ�
  if (!mesh) {
    return std::vector<glm::mat4>();
  }

  // �O���[�v�s����v�Z����
  std::vector<glm::mat4> m(mesh->groups.size());
  std::vector<bool> calculated(m.size(), false);
  for (int n = 0; n < m.size(); ++n) {
    CalcNthGroupMatrix(n, m, calculated);
  }
  return m;
}

/**
* �R���X�g���N�^
*/
InstancedMeshRenderer::InstancedMeshRenderer(size_t instanceCount)
{
  instances.reserve(instanceCount);
  ssbo = std::make_shared<ShaderStorageBuffer>(instanceCount * sizeof(InstanceData));
}

/**
* �N���[�����쐬����
*/
RendererPtr InstancedMeshRenderer::Clone() const
{
  auto clone = std::make_shared<InstancedMeshRenderer>(*this);
  clone->ssbo = std::make_shared<ShaderStorageBuffer>(ssbo->GetSize());
  return clone;
}

/**
* ���b�V����`�悷��
*/
void InstancedMeshRenderer::Draw(const Actor& actor,
  const ProgramPipeline& pipeline,
  const glm::mat4& matVP)
{
  if (!mesh) {
    return;
  }
  if (latestInstanceSize <= 0) {
    return;
  }

  // GPU�������ɑ��邽�߂̃}�e���A���f�[�^���X�V
  if (materialChanged) {
    materialChanged = false;
    colors.resize(materials.size());
    for (int i = 0; i < materials.size(); ++i) {
      colors[i] = materials[i].color;
    }
    textures = GetTextureList(materials);
    textureIndices = GetTextureIndexList(materials, textures);
  }

  // ���f���s���GPU�������ɃR�s�[����
  pipeline.SetUniform(locMatModel, actor.GetModelMatrix());
  if (actor.layer == Layer::Default) {
    // �}�e���A���f�[�^��ݒ�
    pipeline.SetUniform(locMaterialColor, colors.data(), colors.size());
    pipeline.SetUniform(locMaterialTexture,
      textureIndices.data(), textureIndices.size());
  }

  ssbo->Bind(0);

  // TODO: �e�L�X�g���ǉ�
  const GLint locColor = 200;
  pipeline.SetUniform(locColor, actor.color);

  const GLuint bindingPoints[] = { 0, 2, 3, 4, 5, 6, 7, 8 };
  const size_t size = std::min(textures.size(), std::size(bindingPoints));
  for (int i = 0; i < size; ++i) {
    textures[i]->Bind(bindingPoints[i]);
  }
  mesh->primitive.DrawInstanced(latestInstanceSize);

  ssbo->FenceSync();
  ssbo->Unbind(0);
}

/**
* �C���X�^���X�̃��f���s����X�V����
*/
void InstancedMeshRenderer::UpdateInstanceTransforms()
{
  // ����ł���C���X�^���X���폜����
  const auto i = std::remove_if(instances.begin(), instances.end(),
    [](const ActorPtr& e) { return e->isDead; });
  instances.erase(i, instances.end());

  // �C���X�^���X�����v�Z
  latestInstanceSize = std::min(
    ssbo->GetSize() / sizeof(glm::mat4), instances.size());
  if (latestInstanceSize <= 0) {
    return;
  }

  // ���f���s����v�Z
  std::vector<InstanceData> transforms(latestInstanceSize);
  for (size_t i = 0; i < latestInstanceSize; ++i) {
    transforms[i].matModel = instances[i]->GetModelMatrix();
    transforms[i].color = instances[i]->color;
  }

  // ���f���s���GPU�������ɃR�s�[
  ssbo->BufferSubData(0, latestInstanceSize * sizeof(InstanceData), transforms.data());
  ssbo->SwapBuffers();
}

/**
* �C���X�^���X�̃��f���s����X�V����
*/
void InstancedMeshRenderer::UpdateInstanceData(size_t size, const InstanceData* data)
{
  // �C���X�^���X�����v�Z
  latestInstanceSize = std::min(
    ssbo->GetSize() / sizeof(InstanceData), size);
  if (latestInstanceSize <= 0) {
    return;
  }

  // ���f���s���GPU�������ɃR�s�[
  ssbo->BufferSubData(0, latestInstanceSize * sizeof(InstanceData), data);
  ssbo->SwapBuffers();
}

/**
* �N���[�����쐬����
*/
RendererPtr StaticMeshRenderer::Clone() const
{
  return std::make_shared<StaticMeshRenderer>(*this);
}

/**
* ���b�V����`�悷��
*/
void StaticMeshRenderer::Draw(const Actor& actor,
  const ProgramPipeline& pipeline, const glm::mat4& matVP)
{
  if (!file || meshIndex < 0 || meshIndex >= file->meshes.size()) {
    return;
  }

  // ���f���s���GPU�������ɃR�s�[����
  pipeline.SetUniform(locMatModel, actor.GetModelMatrix());

  for (const auto& prim : file->meshes[meshIndex].primitives) {
    // �}�e���A���f�[�^��ݒ�
    const GltfMaterial& m = file->materials[prim.materialNo];
    pipeline.SetUniform(locMaterialColor, m.baseColor);
    m.texBaseColor->Bind(0);

    prim.vao->Bind();
    glDrawElementsBaseVertex(prim.mode, prim.count, prim.type,
      prim.indices, prim.baseVertex);
  }
  glBindVertexArray(0);
}

/**
* �N���[�����쐬����
*/
RendererPtr AnimatedMeshRenderer::Clone() const
{
  return std::make_shared<AnimatedMeshRenderer>(*this);
}

/**
* �A�j���[�V������Ԃ��X�V����
*
* @param deltaTime �O��̍X�V����̌o�ߎ���
* @param actor     �`��Ώۂ̃A�N�^�[
*/
void AnimatedMeshRenderer::Update(Actor& actor, float deltaTime)
{
  // �Đ��t���[���X�V
  if (animation && state == State::play) {
    time += deltaTime * animationSpeed;
    if (isLoop) {
      time -= animation->totalTime * std::floor(time / animation->totalTime);
    } else {
      time = std::clamp(time, 0.0f, animation->totalTime);
    }
  }

  // ��Ԃ��X�V
  if (animation) {
    switch (state) {
    case State::stop:
      break;
    case State::play:
      if (!isLoop && (time >= animation->totalTime)) {
        state = State::stop;
      }
      break;
    case State::pause:
      break;
    }
  }
}

/**
* �`��̑O���������s
*/
void AnimatedMeshRenderer::PreDraw(const Actor& actor)
{
  // �S���b�V���̃A�j���[�V�����s����X�V
  const glm::mat4 matModel = actor.GetModelMatrix();
  ssboRangeList.clear();
  for (const GltfNode* e : scene->meshNodes) {
    // �A�j���[�V�����s����v�Z
    auto matBones = CalcAnimationMatrices(
      file, e, animation.get(), nonAnimatedNodes, time);

    // �A�j���[�V�����s��Ƀ��f���s�������
    for (auto& m : matBones) {
      m = matModel * m;
    }

    // �A�j���[�V�����s����o�b�t�@�ɒǉ����A�ǉ���̃I�t�Z�b�g�ƃT�C�Y���L�^
    const GLintptr offset = fileBuffer->AddAnimationMatrices(matBones);
    const GLsizeiptr size =
      static_cast<GLsizeiptr>(matBones.size() * sizeof(glm::mat4));
    ssboRangeList.push_back({ offset, size });
  }
}

/**
* �X�P���^�����b�V����`�悷��
*/
void AnimatedMeshRenderer::Draw(const Actor& actor,
  const ProgramPipeline& pipeline, const glm::mat4& matVP)
{
  if (!file || !scene || ssboRangeList.empty()) {
    return;
  }

  // �m�[�h�Ɋ܂܂��S�Ẵ��b�V����`��
  for (size_t i = 0; i < scene->meshNodes.size(); ++i) {
    const glm::uint meshNo = scene->meshNodes[i]->mesh;
    const GltfMesh& meshData = file->meshes[meshNo];

    // SSBO���o�C���h
    fileBuffer->BindAnimationBuffer(0, ssboRangeList[i].offset, ssboRangeList[i].size);

    // ���b�V���Ɋ܂܂��S�Ẵv���~�e�B�u��`��
    for (const auto& prim : meshData.primitives) {
      // �}�e���A���f�[�^��ݒ�
      const GltfMaterial& m = file->materials[prim.materialNo];
      pipeline.SetUniform(locMaterialColor, m.baseColor);

      // @todo �e�N�X�`�����Ȃ��ꍇ�ɔ����e�N�X�`����\��t����
      if (m.texBaseColor) {
        m.texBaseColor->Bind(0);
      }

      prim.vao->Bind();
      glDrawElementsBaseVertex(prim.mode, prim.count, prim.type,
        prim.indices, prim.baseVertex);
    }
  }

  // SSBO��VAO�̃o�C���h������
  fileBuffer->UnbindAnimationBuffer(0);
  glBindVertexArray(0);
}

/**
* �\������t�@�C����ݒ肷��
*/
void AnimatedMeshRenderer::SetFile(const GltfFilePtr& f, int sceneNo)
{
  file = f;
  scene = &f->scenes[sceneNo];
  animation = nullptr;
  ssboRangeList.clear();

  state = State::stop;
  time = 0;
  animationSpeed = 1;
  isLoop = true;
}

/**
* �A�j���[�V������ݒ肷��
*
* @param animation �Đ�����A�j���[�V����
* @param isLoop    ���[�v�Đ��̎w��(true=���[�v���� false=���[�v���Ȃ�)
*
* @retval true  �ݒ萬��
* @retval false �ݒ莸�s
*/
bool AnimatedMeshRenderer::SetAnimation(const GltfAnimationPtr& animation, bool isLoop)
{
  // �t�@�C�����ݒ肳��Ă��Ȃ���Ή������Ȃ�
  if (!file) {
    return false;
  }

  // �����A�j���[�V�������w�肳�ꂽ�ꍇ�͉������Ȃ�
  if (this->animation == animation) {
    return true;
  }

  // �A�j���[�V������ݒ�
  this->animation = animation;

  // �A�j���[�V������nullptr�̏ꍇ�͍Đ���Ԃ����u��~�v�ɂ���
  if (!animation) {
    state = State::stop;
    return false;
  }

  // �A�j���[�V�������s��Ȃ��m�[�h�̃��X�g�����
  {
    const int withAnimation = -1; // �u�A�j���[�V��������v��\���l

    // �S�m�[�h�ԍ��̃��X�g���쐬
    const size_t size = file->nodes.size();
    nonAnimatedNodes.resize(size);
    std::iota(nonAnimatedNodes.begin(), nonAnimatedNodes.end(), 0);

    // �A�j���[�V�����Ώۂ̃m�[�h�ԍ����u�A�j���[�V��������v�Œu��������
    for (const auto& e : animation->scales) {
      if (e.targetNodeId < size) {
        nonAnimatedNodes[e.targetNodeId] = withAnimation;
      }
    }
    for (const auto& e : animation->rotations) {
      if (e.targetNodeId < size) {
        nonAnimatedNodes[e.targetNodeId] = withAnimation;
      }
    }
    for (const auto& e : animation->translations) {
      if (e.targetNodeId < size) {
        nonAnimatedNodes[e.targetNodeId] = withAnimation;
      }
    }

    // �u�A�j���[�V��������v�����X�g����폜����
    const auto itr = std::remove(
      nonAnimatedNodes.begin(), nonAnimatedNodes.end(), withAnimation);
    nonAnimatedNodes.erase(itr, nonAnimatedNodes.end());
  }

  // ��Ԃ��u��~���v�ɐݒ�
  time = 0;
  state = State::stop;
  this->isLoop = isLoop;

  return true;
}

/**
* �A�j���[�V������ݒ肷��
*
* @param name   �Đ�����A�j���[�V�����̖��O
* @param isLoop ���[�v�Đ��̎w��(true=���[�v���� false=���[�v���Ȃ�)
*
* @retval true  �ݒ萬��
* @retval false �ݒ莸�s
*/
bool AnimatedMeshRenderer::SetAnimation(const std::string& name, bool isLoop)
{
  if (!file) {
    return false;
  }

  for (const auto& e : file->animations) {
    if (e->name == name) {
      return SetAnimation(e, isLoop);
    }
  }
  return false;
}

/**
* �A�j���[�V������ݒ肷��
*
* @param index  �Đ�����A�j���[�V�����ԍ�
* @param isLoop ���[�v�Đ��̎w��(true=���[�v���� false=���[�v���Ȃ�)
*
* @retval true  �ݒ萬��
* @retval false �ݒ莸�s
*/
bool AnimatedMeshRenderer::SetAnimation(size_t index, bool isLoop)
{
  if (!file || index >= file->animations.size()) {
    return false;
  }
  return SetAnimation(file->animations[index], isLoop);
}

/**
* �A�j���[�V�����̍Đ����J�n�E�ĊJ����
*
* @retval true  ����
* @retval false ���s(�A�j���[�V�������ݒ肳��Ă��Ȃ�)
*/
bool AnimatedMeshRenderer::Play()
{
  if (animation) {
    switch (state) {
    case State::play:  return true;
    case State::stop:  state = State::play; return true;
    case State::pause: state = State::play; return true;
    }
  }
  return false;
}

/**
* �A�j���[�V�����̍Đ����~����
*
* @retval true  ����
* @retval false ���s(�A�j���[�V�������ݒ肳��Ă��Ȃ�)
*/
bool AnimatedMeshRenderer::Stop()
{
  if (animation) {
    switch (state) {
    case State::play:  state = State::stop; return true;
    case State::stop:  return true;
    case State::pause: state = State::stop; return true;
    }
  }
  return false;
}

/**
* �A�j���[�V�����̍Đ����ꎞ��~����
*
* @retval true  ����
* @retval false ���s(�A�j���[�V��������~���Ă���A�܂��̓A�j���[�V�������ݒ肳��Ă��Ȃ�)
*/
bool AnimatedMeshRenderer::Pause()
{
  if (animation) {
    switch (state) {
    case State::play:  state = State::pause; return true;
    case State::stop:  return false;
    case State::pause: return true;
    }
  }
  return false;
}

/**
* �A�j���[�V�����̍Đ��ʒu��ݒ肷��
*
* @param position �Đ��ʒu(�b)
*/
void AnimatedMeshRenderer::SetPosition(float position)
{
  time = position;
  if (animation) {
    if (isLoop) {
      time -= animation->totalTime * std::floor(time / animation->totalTime);
    } else {
      time = std::clamp(time, 0.0f, animation->totalTime);
    }
  } // animation
}

/**
* �A�j���[�V�����̍Đ��ʒu���擾����
*
* @return �Đ��ʒu(�b)
*/
float AnimatedMeshRenderer::GetPosition() const
{
  return time;
}

/**
* �A�j���[�V�����̍Đ����I�����Ă��邩���ׂ�
*
* @retval true  �I�����Ă���
* @retval false �I�����Ă��Ȃ��A�܂��͈�x��Play()�����s����Ă��Ȃ�
*
* ���[�v�Đ����̏ꍇ�A���̊֐��͏��false��Ԃ����Ƃɒ���
*/
bool AnimatedMeshRenderer::IsFinished() const
{
  if (!file || !animation || isLoop) {
    return false;
  }

  // �Đ����x(����)�ɂ���ďI�������ς���
  if (animationSpeed < 0) {
    return time <= 0;
  }
  return time >= animation->totalTime;
}

/**
* �A�N�^�[�ɃX�^�e�B�b�N���b�V�������_����ݒ肷��
*
* @param actor    �����_����ݒ肷��A�N�^�[
* @param filename glTF�t�@�C����
* @param meshNo   �`�悷�郁�b�V���̃C���f�b�N�X
*/
StaticMeshRendererPtr SetStaticMeshRenderer(
  Actor& actor, const char* filename, int meshNo)
{
  GameEngine& engine = GameEngine::Get();
  auto renderer = std::make_shared<StaticMeshRenderer>();
  renderer->SetMesh(engine.LoadGltfFile(filename), meshNo);
  actor.renderer = renderer;
  actor.shader = Shader::StaticMesh;
  return renderer;
}

/**
* �A�N�^�[�ɃA�j���[�V�������b�V�������_����ݒ肷��
*
* @param actor    �����_����ݒ肷��A�N�^�[
* @param filename glTF�t�@�C����
* @param sceneNo  �`�悷��V�[���̔ԍ�
*/
AnimatedMeshRendererPtr SetAnimatedMeshRenderer(
  Actor& actor, const char* filename, int sceneNo)
{
  GameEngine& engine = GameEngine::Get();
  auto renderer = std::make_shared<AnimatedMeshRenderer>();
  GltfFilePtr p = engine.LoadGltfFile(filename);
  renderer->SetFileBuffer(engine.GetGltfFileBuffer());
  renderer->SetFile(p, sceneNo);
  actor.renderer = renderer;
  actor.shader = Shader::AnimatedMesh;
  return renderer;
}

