/**
* @file Primitive.h
**/
#ifndef PRIMITIVE_H_INCLUDED
#define PRIMITIVE_H_INCLUDED
#include <glad/glad.h>
#include "Texture.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>

/**
* �v���~�e�B�u�f�[�^.
*/
class Primitive
{
public:
  Primitive() = default;
  Primitive(const char* name, GLenum m, GLsizei c, size_t o, GLint b) :
    name(name),
    mode(m), count(c), indices(reinterpret_cast<GLvoid*>(o)), baseVertex(b)
  {}
  ~Primitive() = default;

  void Draw() const;
  void DrawInstanced(size_t instanceCount) const;
  const std::string& GetName() const { return name; }

private:
  std::string name;
  GLenum mode = GL_TRIANGLES; ///< �v���~�e�B�u�̎��.
  GLsizei count = 0; ///< �`�悷��C���f�b�N�X��.
  const GLvoid* indices = 0; ///< �`��J�n�C���f�b�N�X�̃o�C�g�I�t�Z�b�g.
  GLint baseVertex = 0; ///< �C���f�b�N�X0�ԂƂ݂Ȃ���钸�_�z����̈ʒu.
};

/**
* �v���~�e�B�u�`��f�[�^
*/
class Mesh
{
public:
  /**
  * �}�e���A��(�ގ�)�f�[�^
  */
  struct Material
  {
    std::string name;               // �}�e���A����
    glm::vec4 color = glm::vec4(1); // �f�B�t���[�Y�F
    std::shared_ptr<Texture> tex;   // �e�N�X�`��
    std::shared_ptr<Texture> texNormal; // �@���e�N�X�`��
    float roughness = 0.5f;         // �\�ʂ̑e��

    float metalness = 0;    // �����=0, ����=1
  };

  /**
  * �}�e���A���̊��蓖�Ĕ͈�
  */
  struct UseMaterial
  {
    int materialNo = -1; // ���蓖�Ă�}�e���A���̔ԍ�
    GLsizei indexCount = 0; // �}�e���A�������蓖�Ă�C���f�b�N�X�f�[�^�̐�
  };

  /**
  * �|���S�����O���[�v������f�[�^
  */
  struct Group
  {
    std::string name; // �O���[�v��
    GLsizei indexCount = 0; // �O���[�v�Ɋ܂܂��C���f�b�N�X�f�[�^�̐�

    // 21�Ŏ���. 21b�͖�����.
    static const int noParent = -1; // �e�����Ȃ����Ƃ������萔
    int parent = noParent; // �e�O���[�v�ԍ�
    glm::mat4 matBindPose = glm::mat4(1);
    glm::mat4 matInverseBindPose = glm::mat4(1);
  };

  Mesh() = default;
  ~Mesh() = default;
  Mesh(const Mesh&) = default;
  Mesh& operator=(const Mesh&) = default;

  Primitive primitive;
  std::vector<Material> materials;
  std::vector<UseMaterial> useMaterials;
  std::vector<Group> groups;
};
using MeshPtr = std::shared_ptr<Mesh>;

/**
* �����̃v���~�e�B�u���Ǘ�����N���X.
*/
class PrimitiveBuffer
{
public:
  PrimitiveBuffer(GLsizei maxVertexCount, GLsizei maxIndexCount);
  ~PrimitiveBuffer();

  // �v���~�e�B�u�̒ǉ�(20�ɂ�pGroupAndMaterial������ǉ�)
  bool Add(size_t vertexCount, const glm::vec3* pPosition, const glm::vec4* pColor,
    const glm::vec2* pTexcoord, const glm::vec3* pNormal,
    const glm::u8vec2* pMaterialGroup,
    size_t indexCount, const GLushort* pIndex, const char* name = nullptr,
    GLenum type = GL_TRIANGLES);
  bool AddFromObjFile(const char* filename);

  // �v���~�e�B�u�̎擾.
  const Primitive& Get(size_t n) const;
  const Primitive& Find(const char* name) const;
  const MeshPtr& GetMesh(const char* name) const; // 20�Ŏ���. 20b�͖�����.

  // VAO�o�C���h�Ǘ�.
  void BindVertexArray() const;
  void UnbindVertexArray() const;

  // TODO: �e�L�X�g���ǉ�
  size_t GetCount() const { return primitives.size(); }

private:
  std::vector<Primitive> primitives;
  std::vector<MeshPtr> meshes;

  // �o�b�t�@ID.
  GLuint vboPosition = 0;
  GLuint vboColor = 0;
  GLuint vboTexcoord = 0;
  GLuint vboNormal = 0;
  GLuint vboMaterialGroup = 0;
  GLuint ibo = 0;
  GLuint vao = 0;

  GLsizei maxVertexCount = 0; // �i�[�ł���ő咸�_��.
  GLsizei curVertexCount = 0; // �i�[�ςݒ��_��.
  GLsizei maxIndexCount = 0; // �i�[�ł���ő�C���f�b�N�X��.
  GLsizei curIndexCount = 0; // �i�[�ς݃C���f�b�N�X��.
};

bool CopyData(GLuint writeBuffer, GLsizei unitSize,
  GLsizei offsetCount, size_t count, const void* data);

using TextureList = std::vector<std::shared_ptr<Texture>>;
using MaterialParameterList = std::vector<glm::vec4>;
TextureList GetTextureList(const std::vector<Mesh::Material>& materials);
TextureList GetNormalTextureList(const std::vector<Mesh::Material>& materials);
MaterialParameterList GetMaterialParameterList(const std::vector<Mesh::Material>& materials,
  const TextureList& textures, const TextureList& texNormals);

#endif // PRIMITIVE_H_INCLUDED
