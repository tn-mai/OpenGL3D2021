/**
* @file Renderer.cpp
*/
#include "Renderer.h"
#include "ProgramPipeline.h"
#include "Texture.h"
#include "Actor.h"
#include "GltfMesh.h"
#include "VertexArrayObject.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <numeric>

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
*
*/
namespace GlobalAnimatedMeshState {

namespace /* unnamed */ {
ShaderStorageBufferPtr ssbo;
std::vector<glm::mat4> dataBuffer;
} // unnamed namespace

/**
* �A�j���[�V�������b�V���p�̃o�b�t�@���쐬
*/
bool Initialize(size_t maxCount)
{
  ssbo = std::make_shared<ShaderStorageBuffer>(sizeof(glm::mat4) * maxCount);
  dataBuffer.reserve(maxCount);
  return ssbo.get();
}

/**
* �A�j���[�V�������b�V���p�̃o�b�t�@���폜
*/
void Finalize()
{
  if (ssbo) {
    ssbo.reset();
    dataBuffer.clear();
    dataBuffer.shrink_to_fit();
  }
}

/**
* �A�j���[�V�������b�V���̕`��p�f�[�^�����ׂč폜
*/
void ClearData()
{
  dataBuffer.clear();
}

/**
* �A�j���[�V�������b�V���̕`��p�f�[�^��ǉ�
*/
GLintptr AddData(const Data& data)
{
  GLintptr offset = static_cast<GLintptr>(dataBuffer.size() * sizeof(glm::mat4));
  dataBuffer.push_back(data.matRoot);
  dataBuffer.insert(dataBuffer.end(), data.matBones.begin(), data.matBones.end());

  // �I�t�Z�b�g���E��256�o�C�g�ɂȂ�悤�ɂ���
  dataBuffer.resize(((dataBuffer.size() + 3) / 4) * 4);
  return offset;
}

/**
* �A�j���[�V�������b�V���̕`��p�f�[�^��GPU�������ɃR�s�[
*/
void Upload()
{
  ssbo->BufferSubData(0, dataBuffer.size() * sizeof(Data), dataBuffer.data());
  ssbo->SwapBuffers();
}

/**
* �A�j���[�V�������b�V���̕`��Ɏg��SSBO�̈�����蓖�Ă�
*/
void Bind(GLuint bindingPoint, GLintptr offset, GLsizeiptr size)
{
  ssbo->Bind(bindingPoint, offset, size);
}

/**
* �A�j���[�V�������b�V���̕`��Ɏg��SSBO�̈�̊��蓖�Ă���������
*/
void Unbind(GLuint bindingPoint)
{
  ssbo->Unbind(bindingPoint);
}

} // GlobalAnimatedMeshState

namespace /* unnamed */ {

/**
* �A�j���[�V�����p�̒��ԃf�[�^
*/
struct Transformation
{
  glm::mat4 m = glm::mat4(1);
  bool isCalculated = false;
};
using TransformationList = std::vector<Transformation>;

/**
* �m�[�h�̃O���[�o�����f���s����v�Z����
*/
const glm::mat4& CalcGlobalTransform(const std::vector<GltfNode>& nodes,
  const GltfNode& node, TransformationList& transList)
{
  const intptr_t currentNodeId = &node - &nodes[0];
  Transformation& trans = transList[currentNodeId];
  if (trans.isCalculated) {
    return trans.m;
  }

  glm::mat4 matParent;
  if (node.parent) {
    matParent = CalcGlobalTransform(nodes, *node.parent, transList);
  } else {
    matParent = glm::mat4(1);
  }
  trans.m = matParent * trans.m;
  trans.isCalculated = true;

  return trans.m;
}

/**
* �A�j���[�V������Ԃ��ꂽ���W�ϊ��s����v�Z����
*/
TransformationList CalcAnimatedTransformations(const GltfFile& file,
  const GltfAnimation& animation, const std::vector<int>& nonAnimatedNodeList,
  float keyFrame)
{
  TransformationList transList;
  transList.resize(file.nodes.size());
  for (const auto e : nonAnimatedNodeList) {
    transList[e].m = file.nodes[e].matLocal;
  }

  for (const auto& e : animation.translationList) {
    auto& trans = transList[e.targetNodeId];
    const glm::vec3 translation = Interporation(e, keyFrame);
    trans.m *= glm::translate(glm::mat4(1), translation);
  }
  for (const auto& e : animation.rotationList) {
    auto& trans = transList[e.targetNodeId];
    const glm::quat rotation = Interporation(e, keyFrame);
    trans.m *= glm::mat4_cast(rotation);
  }
  for (const auto& e : animation.scaleList) {
    auto& trans = transList[e.targetNodeId];
    const glm::vec3 scale = Interporation(e, keyFrame);
    trans.m *= glm::scale(glm::mat4(1), scale);
  }

  for (auto& e : file.nodes) {
    CalcGlobalTransform(file.nodes, e, transList);
  }

  return transList;
}

/**
* �A�j���[�V������K�p�������W�ϊ��s�񃊃X�g���v�Z����
*
* @param file      �A�j���[�V�����ƃm�[�h�����L����t�@�C���I�u�W�F�N�g
* @param node      �X�L�j���O�Ώۂ̃m�[�h
* @param animation �v�Z�̌��ɂȂ�A�j���[�V����
* @param frame     �A�j���[�V�����̍Đ��ʒu
*
* @return �A�j���[�V������K�p�������W�ϊ��s�񃊃X�g
*/
GlobalAnimatedMeshState::Data CalculateTransform(const GltfFilePtr& file,
  const GltfNode* meshNode, const GltfAnimation* animation,
  const std::vector<int>& nonAnimatedNodeList, float frame)
{
  GlobalAnimatedMeshState::Data data;
  if (!file || !meshNode) {
    return data;
  }

  if (animation) {
    const TransformationList transList = CalcAnimatedTransformations(*file, *animation, nonAnimatedNodeList, frame);
    if (meshNode->skin >= 0) {
      // �A�j���[�V��������+�X�L������
      // @note joints�ɂ̓m�[�h�ԍ����i�[����Ă��邪�A���_�f�[�^��JOINTS_n�ɂ�
      //       �m�[�h�ԍ��ł͂Ȃ��ujoints�z��̃C���f�b�N�X�v���i�[����Ă���B
      //       �܂�A�{�[���s��z���joints�̏��Ԃ�SSBO�Ɋi�[����K�v������B
      const auto& joints = file->skins[meshNode->skin].joints;
      data.matBones.resize(joints.size());
      for (size_t i = 0; i < joints.size(); ++i) {
        const auto& joint = joints[i];
        data.matBones[i] = transList[joint.nodeId].m * joint.matInverseBindPose;
      }
      data.matRoot = glm::mat4(1);
    } else {
      // �A�j���[�V��������+�X�L���Ȃ�
      const size_t nodeId = meshNode - &file->nodes[0];
      data.matRoot = transList[nodeId].m;
    }
  } else {
    // �A�j���[�V�����Ȃ�
    data.matRoot = meshNode->matGlobal;
    if (meshNode->skin >= 0) {
      // �X�L������
      const auto& joints = file->skins[meshNode->skin].joints;
      data.matBones.resize(joints.size(), glm::mat4(1));
    }
  }
  return data;
}

} // unnamed namespace

/**
* �N���[�����쐬����
*/
RendererPtr AnimatedMeshRenderer::Clone() const
{
  return std::make_shared<AnimatedMeshRenderer>(*this);
}

/**
* �Đ�����V�[����ݒ肷��
*/
void AnimatedMeshRenderer::SetScene(const GltfFilePtr& f, int sceneNo)
{
  file = f;
  this->sceneNo = sceneNo;
  scene = &f->scenes[sceneNo];
  animation = nullptr;
  ssboRangeList.clear();

  state = State::stop;
  frame = 0;
  animationSpeed = 1;
  isLoop = true;
}

/**
* �A�j���[�V������Ԃ��X�V����
*
* @param deltaTime �O��̍X�V����̌o�ߎ���
* @param actor     �`��Ώۂ̃A�N�^�[
*/
void AnimatedMeshRenderer::Update(const Actor& actor, float deltaTime)
{
  // �Đ��t���[���X�V
  if (animation && state == State::play) {
    frame += deltaTime * animationSpeed;
    if (isLoop) {
      if (frame >= animation->totalTime) {
        frame -= animation->totalTime;
      } else if (frame < 0) {
        const float n = std::ceil(-frame / animation->totalTime);
        frame += animation->totalTime * n;
      }
    } else {
      if (frame >= animation->totalTime) {
        frame = animation->totalTime;
      } else if (frame < 0) {
        frame = 0;
      }
    }
  }

  // SSBO�ɃR�s�[����f�[�^��ǉ�
  const glm::mat4 matModel = actor.GetModelMatrix();
  ssboRangeList.clear();
  for (const auto e : scene->meshNodes) {
    GlobalAnimatedMeshState::Data data = CalculateTransform(file, e, animation, nonAnimatedNodeList, frame);
    data.matRoot = matModel * data.matRoot;
    for (auto& m : data.matBones) {
      m = matModel * m;
    }
    const GLintptr offset = GlobalAnimatedMeshState::AddData(data);
    const GLsizeiptr size = static_cast<GLsizeiptr>((data.matBones.size() + 1) * sizeof(glm::mat4));
    ssboRangeList.push_back({ offset, size });
  }

  // ��Ԃ��X�V
  if (animation) {
    switch (state) {
    case State::stop:
      break;
    case State::play:
      if (!isLoop && (frame >= animation->totalTime)) {
        state = State::stop;
      }
      break;
    case State::pause:
      break;
    }
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

  // ���f���s���GPU�������ɃR�s�[����
  //pipeline.SetUniform(locMatModel, actor.GetModelMatrix());

  for (size_t i = 0; i < scene->meshNodes.size(); ++i) {
    const glm::uint meshNo = scene->meshNodes[i]->mesh;
    const GltfMesh& meshData = file->meshes[meshNo];
    GlobalAnimatedMeshState::Bind(0, ssboRangeList[i].offset, ssboRangeList[i].size);
    //pipeline.SetUniform(11, &meshNo, 1);
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

  GlobalAnimatedMeshState::Unbind(0);
  glBindVertexArray(0);
}

/**
* �A�j���[�V�����̍Đ���Ԃ��擾����
*
* @return �Đ���Ԃ�����State�񋓌^�̒l
*/
AnimatedMeshRenderer::State AnimatedMeshRenderer::GetState() const
{
  return state;
}

/**
* ���b�V���Ɋ֘A�t����ꂽ�A�j���[�V�����̃��X�g���擾����
*
* @return �A�j���[�V�������X�g
*/
const std::vector<GltfAnimation>& AnimatedMeshRenderer::GetAnimationList() const
{
  if (!file) {
    static const std::vector<GltfAnimation> dummy;
    return dummy;
  }
  return file->animations;
}

/**
* �A�j���[�V�������Ԃ��擾����
*
* @return �A�j���[�V��������(�b)
*/
float AnimatedMeshRenderer::GetTotalAnimationTime() const
{
  if (!file || !animation) {
    return 0;
  }
  return animation->totalTime;
}

/**
*
*/
std::vector<int> MakeNonAnimatedNodeList(size_t size, const GltfAnimation* animation)
{
  std::vector<int> nonAnimatedNodeList(size);
  std::iota(nonAnimatedNodeList.begin(), nonAnimatedNodeList.end(), 0);
  for (auto e : animation->scaleList) {
    nonAnimatedNodeList[e.targetNodeId] = -1;
  }
  for (auto e : animation->rotationList) {
    nonAnimatedNodeList[e.targetNodeId] = -1;
  }
  for (auto e : animation->translationList) {
    nonAnimatedNodeList[e.targetNodeId] = -1;
  }
  nonAnimatedNodeList.erase(
    std::remove(nonAnimatedNodeList.begin(), nonAnimatedNodeList.end(), -1),
    nonAnimatedNodeList.end());
  return nonAnimatedNodeList;
}

/**
* �A�j���[�V�������Đ�����
*
* @param animationName �Đ�����A�j���[�V�����̖��O
* @param isLoop        ���[�v�Đ��̎w��(true=���[�v���� false=���[�v���Ȃ�)
*
* @retval true  �Đ��J�n
* @retval false �Đ����s
*/
bool AnimatedMeshRenderer::Play(const std::string& animationName, bool isLoop)
{
  if (file) {
    for (const auto& e : file->animations) {
      if (e.name == animationName) {
        animation = &e;
        frame = 0;
        state = State::play;
        this->isLoop = isLoop;
        nonAnimatedNodeList = MakeNonAnimatedNodeList(file->nodes.size(), animation);
        return true;
      }
    }
  }
  return false;
}

/**
*
*/
bool AnimatedMeshRenderer::Play(size_t index, bool isLoop)
{
  if (file && index < file->animations.size()) {
    animation = &file->animations[index];
    frame = 0;
    state = State::play;
    this->isLoop = isLoop;
    nonAnimatedNodeList = MakeNonAnimatedNodeList(file->nodes.size(), animation);
    return true;
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
    case State::play:
      state = State::stop;
      return true;
    case State::stop:
      return true;
    case State::pause:
      state = State::stop;
      return true;
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
    case State::play:
      state = State::pause;
      return true;
    case State::stop:
      return false;
    case State::pause:
      return true;
    }
  }
  return false;
}

/**
* �A�j���[�V�����̍Đ����ĊJ����
*
* @retval true  ����
* @retval false ���s(�A�j���[�V��������~���Ă���A�܂��̓A�j���[�V�������ݒ肳��Ă��Ȃ�)
*/
bool AnimatedMeshRenderer::Resume()
{
  if (animation) {
    switch (state) {
    case State::play:
      return true;
    case State::stop:
      return false;
    case State::pause:
      state = State::play;
      return true;
    }
  }
  return false;
}

/**
* �Đ����̃A�j���[�V���������擾����
*
* @return �Đ����̃A�j���[�V������
*         ��x��Play()�ɐ������Ă��Ȃ��ꍇ�A��̕����񂪕Ԃ����
*/
const std::string& AnimatedMeshRenderer::GetAnimation() const
{
  if (!animation) {
    static const std::string dummy("");
    return dummy;
  }
  return animation->name;
}

/**
* �A�j���[�V�����̍Đ����x��ݒ肷��
*
* @param speed �Đ����x(1.0f=����, 2.0f=2�{��, 0.5f=1/2�{��)
*/
void AnimatedMeshRenderer::SetAnimationSpeed(float speed)
{
  animationSpeed = speed;
}

/**
* �A�j���[�V�����̍Đ����x���擾����
*
* @return �Đ����x
*/
float AnimatedMeshRenderer::GetAnimationSpeed() const
{
  return animationSpeed;
}

/**
* �A�j���[�V�����̍Đ��ʒu��ݒ肷��
*
* @param position �Đ��ʒu(�b)
*/
void AnimatedMeshRenderer::SetPosition(float position)
{
  frame = position;
  if (animation) {
    if (isLoop) {
      if (frame >= animation->totalTime) {
        frame -= animation->totalTime;
      } else if (frame < 0) {
        const float n = std::ceil(-frame / animation->totalTime);
        frame += animation->totalTime * n;
      }
    } else {
      if (frame >= animation->totalTime) {
        frame = animation->totalTime;
      } else if (frame < 0) {
        frame = 0;
      }
    }
  }
}

/**
* �A�j���[�V�����̍Đ��ʒu���擾����
*
* @return �Đ��ʒu(�b)
*/
float AnimatedMeshRenderer::GetPosition() const
{
  return frame;
}

/**
* �A�j���[�V�����̍Đ����I�����Ă��邩���ׂ�
*
* @retval true  �I�����Ă���
* @retval false �I�����Ă��Ȃ��A�܂��͈�x���L���Ȗ��O��Play()�����s����Ă��Ȃ�
*
* ���[�v�Đ����̏ꍇ�A���̊֐��͏��false��Ԃ����Ƃɒ���
*/
bool AnimatedMeshRenderer::IsFinished() const
{
  if (!file || !animation) {
    return false;
  }
  return animation->totalTime <= frame;
}

/**
* ���[�v�Đ��̗L�����擾����
*
* @retval true  ���[�v�Đ������
* @retval false ���[�v�Đ�����Ȃ�
*/
bool AnimatedMeshRenderer::IsLoop() const
{
  return isLoop;
}

/**
* ���[�v�Đ��̗L����ݒ肷��
*
* @param isLoop ���[�v�Đ��̗L��
*/
void AnimatedMeshRenderer::SetLoop(bool isLoop)
{
  this->isLoop = isLoop;
}

