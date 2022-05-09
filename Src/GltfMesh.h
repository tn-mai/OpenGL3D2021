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
