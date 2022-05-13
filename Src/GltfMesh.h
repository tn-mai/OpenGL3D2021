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

// �X�L���f�[�^
struct GltfSkin
{
  std::string name;

  struct Joint {
    int nodeId;
    glm::mat4 matInverseBindPose;
  };
  std::vector<Joint> joints;
};

// �m�[�h
struct GltfNode
{
  GltfNode* parent = nullptr;
  int mesh = -1;
  int skin = -1;
  std::vector<GltfNode*> children;
  glm::mat4 matLocal = glm::mat4(1);
  glm::mat4 matGlobal = glm::mat4(1);
};

// �A�j���[�V�����̃L�[�t���[��
template<typename T>
struct GltfKeyFrame
{
  float frame;
  T value;
};

// �A�j���[�V�����̃^�C�����C��
template<typename T>
struct GltfTimeline
{
  int targetNodeId;
  std::vector<GltfKeyFrame<T>> timeline;
};
glm::vec3 Interporation(const GltfTimeline<glm::vec3>& data, float frame);
glm::quat Interporation(const GltfTimeline<glm::quat>& data, float frame);

// �A�j���[�V����
struct GltfAnimation
{
  std::vector<GltfTimeline<glm::vec3>> translationList;
  std::vector<GltfTimeline<glm::quat>> rotationList;
  std::vector<GltfTimeline<glm::vec3>> scaleList;
  float totalTime = 0;
  std::string name;
};

/**
* �V�[��
*/
struct GltfScene
{
  //std::vector<const GltfNode*> nodes;
  std::vector<const GltfNode*> meshNodes;
};

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
* �t�@�C��
*/
struct GltfFile
{
  std::string name; // �t�@�C����
  std::vector<GltfMesh> meshes;
  std::vector<GltfMaterial> materials;

  // �A�j���[�V�������b�V���p�f�[�^
  std::vector<GltfScene> scenes;
  std::vector<GltfNode> nodes;
  std::vector<GltfSkin> skins;
  std::vector<GltfAnimation> animations;
};
using GltfFilePtr = std::shared_ptr<GltfFile>;

/**
* ���b�V�����Ǘ�����N���X
*/
class GltfFileBuffer
{
public:
  explicit GltfFileBuffer(size_t maxBufferSize);
  ~GltfFileBuffer();
  GltfFileBuffer(const GltfFileBuffer&) = delete;
  GltfFileBuffer& operator=(const GltfFileBuffer&) = delete;

  bool AddFromFile(const char* filename);
  GltfFilePtr GetFile(const char* filename) const;

private:
  // glTF�p�̃o�b�t�@�I�u�W�F�N�g
  GLuint buffer = 0;
  GLsizei maxBufferSize = 0;
  GLsizei curBufferSize = 0;
  
  // ���b�V���t�@�C���Ǘ��p�̘A�z�z��
  std::unordered_map<std::string, GltfFilePtr> files;
};
using GltfFileBufferPtr = std::shared_ptr<GltfFileBuffer>;

#endif// GLTFMESH_H_INCLUDED
