/**
* @file GltfMesh.h
*/
#ifndef GLTFMESH_H_INCLUDED
#define GLTFMESH_H_INCLUDED
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

// ��s�錾
class Texture;
using TexturePtr = std::shared_ptr<Texture>;
class VertexArrayObject;
using VertexArrayObjectPtr = std::shared_ptr<VertexArrayObject>;
class ShaderStorageBuffer;
using ShaderStorageBufferPtr = std::shared_ptr<ShaderStorageBuffer>;

/**
* �A�j���[�V�����̃L�[�t���[��
*/
template<typename T>
struct GltfKeyFrame
{
  float frame;
  T value;
};

/**
* �A�j���[�V�����̃^�C�����C��
*/
template<typename T>
struct GltfTimeline
{
  int targetNodeId;
  std::vector<GltfKeyFrame<T>> timeline;
};
glm::vec3 Interporation(const GltfTimeline<glm::vec3>& data, float frame);
glm::quat Interporation(const GltfTimeline<glm::quat>& data, float frame);

/**
* �A�j���[�V����
*/
struct GltfAnimation
{
  std::string name; // �A�j���[�V������
  std::vector<GltfTimeline<glm::vec3>> translationList;
  std::vector<GltfTimeline<glm::quat>> rotationList;
  std::vector<GltfTimeline<glm::vec3>> scaleList;
  float totalTime = 0;
};
using GltfAnimationPtr = std::shared_ptr<GltfAnimation>;

/**
* �}�e���A��
*/
struct GltfMaterial
{
  glm::vec4 baseColor = glm::vec4(1);
  TexturePtr texBaseColor;
};

/**
* �v���~�e�B�u�f�[�^
*/
struct GltfPrimitive
{
  GLenum mode = GL_TRIANGLES; // �v���~�e�B�u�̎��
  GLsizei count = 0; // �`�悷��C���f�b�N�X��
  GLenum type = GL_UNSIGNED_SHORT; // �C���f�b�N�X�f�[�^�^
  const GLvoid* indices = 0; // �`��J�n�C���f�b�N�X�̃o�C�g�I�t�Z�b�g
  GLint baseVertex = 0; // �C���f�b�N�X0�ԂƂ݂Ȃ���钸�_�z����̈ʒu

  VertexArrayObjectPtr vao;
  size_t materialNo = 0; // �}�e���A���ԍ�
};

/**
* ���b�V���f�[�^
*/
struct GltfMesh
{
  std::string name; // ���b�V����
  std::vector<GltfPrimitive> primitives;
};

/**
* �X�L��
*/
struct GltfSkin
{
  std::string name; // �X�L����

  struct Joint {
    int nodeId;
    glm::mat4 matInverseBindPose;
  };
  std::vector<Joint> joints;
};

/**
* �m�[�h
*/
struct GltfNode
{
  std::string name; // �m�[�h��
  int mesh = -1;    // ���b�V���ԍ�
  int skin = -1;    // �X�L���ԍ�
  GltfNode* parent = nullptr;         // �e�m�[�h
  std::vector<GltfNode*> children;    // �q�m�[�h�z��
  glm::mat4 matLocal = glm::mat4(1);  // ���[�J���s��
  glm::mat4 matGlobal = glm::mat4(1); // �O���[�o���s��
};

/**
* �V�[��
*/
struct GltfScene
{
  std::vector<const GltfNode*> nodes;
  std::vector<const GltfNode*> meshNodes;
};

/**
* �t�@�C��
*/
struct GltfFile
{
  std::string name; // �t�@�C����
  std::vector<GltfScene> scenes;
  std::vector<GltfNode> nodes;
  std::vector<GltfSkin> skins;
  std::vector<GltfMesh> meshes;
  std::vector<GltfMaterial> materials;
  std::vector<GltfAnimationPtr> animations;
};
using GltfFilePtr = std::shared_ptr<GltfFile>;

/**
* ���b�V�����Ǘ�����N���X
*/
class GltfFileBuffer
{
public:
  GltfFileBuffer(size_t maxBufferSize, size_t maxMatrixSize);
  ~GltfFileBuffer();

  // �R�s�[���֎~
  GltfFileBuffer(const GltfFileBuffer&) = delete;
  GltfFileBuffer& operator=(const GltfFileBuffer&) = delete;

  bool AddFromFile(const char* filename);
  GltfFilePtr GetFile(const char* filename) const;

  using AnimationMatrices = std::vector<glm::mat4>;
  void ClearAnimationBuffer();
  GLintptr AddAnimationData(const AnimationMatrices& matBones);
  void UploadAnimationBuffer();
  void BindAnimationBuffer(GLuint bindingPoint, GLintptr offset, GLsizeiptr size);
  void UnbindAnimationBuffer(GLuint bindingPoint);

private:
  // glTF�p�̃o�b�t�@�I�u�W�F�N�g
  GLuint buffer = 0;
  GLsizei maxBufferSize = 0;
  GLsizei curBufferSize = 0;
  
  // ���b�V���t�@�C���Ǘ��p�̘A�z�z��
  std::unordered_map<std::string, GltfFilePtr> files;

  // �A�j���[�V�����s��p�o�b�t�@
  ShaderStorageBufferPtr ssbo;
  std::vector<glm::mat4> dataBuffer;
};
using GltfFileBufferPtr = std::shared_ptr<GltfFileBuffer>;

#endif// GLTFMESH_H_INCLUDED
