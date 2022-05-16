/**
* @file GltfMesh.cpp
*/
#include "GltfMesh.h"
#include "VertexArrayObject.h"
#include "GLContext.h"
#include "GameEngine.h"
#include "ShaderStorageBuffer.h"
#include "json11/json11.hpp"
#include <glm/gtc/quaternion.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>

using namespace json11;

namespace { // unnamed

/**
* �t�@�C����ǂݍ���
*
* @param filename �ǂݍ��ރt�@�C����
*/
std::vector<char> ReadFile(const char* filename)
{
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "[�G���[]" << __func__ << ":`" << filename << "`���J���܂���.\n";
    return {};
  }
  std::vector<char> buf(std::filesystem::file_size(filename));
  file.read(buf.data(), buf.size());
  return buf;
}

/**
* �o�C�i���f�[�^�^
*/
struct BinaryData
{
  GLsizeiptr offset;
  std::vector<char> bin;
};
using BinaryList = std::vector<BinaryData>;

/**
* �A�N�Z�b�T�������o�b�t�@�̃I�t�Z�b�g���擾����
*
* @param accessor    �A�N�Z�b�T
* @param bufferViews �o�b�t�@�r���[�z��
* @param binaryList  �o�C�i���f�[�^�z��
*/
GLsizeiptr GetBufferOffset(const Json& accessor,
  const Json& bufferViews, const BinaryList& binaryList)
{
  // �A�N�Z�b�T����K�v�ȏ����擾
  const int byteOffset = accessor["byteOffset"].int_value();
  const int bufferViewId = accessor["bufferView"].int_value();

  // �o�b�t�@�r���[����K�v�ȏ����擾
  const Json bufferView = bufferViews[bufferViewId];
  const int bufferId = bufferView["buffer"].int_value();
  const int baesByteOffset = bufferView["byteOffset"].int_value();

  // �I�t�Z�b�g���v�Z
  return binaryList[bufferId].offset + baesByteOffset + byteOffset;
}

/**
* �A�N�Z�b�T�������o�b�t�@�̃I�t�Z�b�g���擾����
*
* @param accessor    �A�N�Z�b�T
* @param bufferViews �o�b�t�@�r���[�z��
*/
GLsizei GetBufferStride(const Json& accessor, const Json& bufferViews)
{
  const int bufferViewId = accessor["bufferView"].int_value();
  const Json bufferView = bufferViews[bufferViewId];

  // byteStride��0���傫���l�Œ�`����Ă�����A���̒l��Ԃ�
  const GLsizei stride = bufferView["byteStride"].int_value();
  if (stride > 0) {
    return stride;
  }

  // byteStride������`�܂���0�������ꍇ�A�f�[�^1���̃T�C�Y���v�Z���ĕԂ�

  // �v�f�^�̃T�C�Y
  int componentSize = 1;
  const int componentType = accessor["componentType"].int_value();
  switch (componentType) {
  case GL_BYTE:           componentSize = 1; break;
  case GL_UNSIGNED_BYTE:  componentSize = 1; break;
  case GL_SHORT:          componentSize = 2; break;
  case GL_UNSIGNED_SHORT: componentSize = 2; break;
  case GL_UNSIGNED_INT:   componentSize = 4; break;
  case GL_FLOAT:          componentSize = 4; break;
  default:
    std::cerr << "[�x��]" << __func__ << ":glTF�̎d�l�ɂȂ��^" <<
      componentType << "���g���Ă��܂�\n";
    break;
  }

  // �^���Ɨv�f���̑Ή��\
  const struct {
    const char* type; // �^�̖��O
    int elementCount; // �v�f��
  } elementCountList[] = {
    { "SCALAR", 1 },
    { "VEC2", 2 }, { "VEC3", 3 }, { "VEC4", 4 },
    { "MAT2", 4 }, { "MAT3", 9 }, { "MAT4", 16 },
  };

  // �v�f��
  const std::string& type = accessor["type"].string_value();
  int elementCount = 1;
  for (const auto& e : elementCountList) {
    if (type == e.type) {
      elementCount = e.elementCount;
      break;
    }
  }

  // �v�f�̃T�C�Y���X�g���C�h�Ƃ���
  return componentSize * elementCount;
}

/**
* �A�N�Z�b�T�������o�b�t�@���̃A�h���X���擾����
*
* @param accessor    �A�N�Z�b�T
* @param bufferViews �o�b�t�@�r���[�z��
* @param binaryList  �o�C�i���f�[�^�z��
*
* @return �o�b�t�@���̃A�h���X
*/
const void* GetBuffer(const Json& accessor, const Json& bufferViews, const BinaryList& binaryList)
{
  // �A�N�Z�b�T����K�v�ȏ����擾
  const int byteOffset = accessor["byteOffset"].int_value();
  const int bufferViewId = accessor["bufferView"].int_value();
  const int count = accessor["count"].int_value();

  // �o�b�t�@�r���[����K�v�ȏ����擾
  const Json bufferView = bufferViews[bufferViewId];
  const int bufferId = bufferView["buffer"].int_value();
  const int baesByteOffset = bufferView["byteOffset"].int_value();
  const int byteLength = bufferView["byteLength"].int_value();

  // �A�N�Z�b�T���Q�Ƃ���f�[�^�̐擪���v�Z
  return binaryList[bufferId].bin.data() + baesByteOffset + byteOffset;
}

/**
* ���_�A�g���r���[�g��ݒ肷��
*
* @param vao         ���_�A�g���r���[�g��ݒ肷��VAO
* @param index       ���_�A�g���r���[�g�ԍ��y�уo�C���f�B���O�|�C���g
* @param buffer      VBO�Ƃ��Ĉ����o�b�t�@�I�u�W�F�N�g
* @param accessor    ���_�A�g���r���[�g�f�[�^�����A�N�Z�b�T
* @param bufferViews �o�b�t�@�r���[�z��
* @param binaryList  �o�C�i���f�[�^�z��
*/
bool SetAttribute(VertexArrayObjectPtr& vao, int index, GLuint buffer,
  const Json& accessor, const Json& bufferViews,
  const BinaryList& binaryList)
{
  // �^���Ɨv�f���̑Ή��\(���_�A�g���r���[�g�p)
  const struct {
    const char* type; // �^�̖��O
    int elementCount; // �v�f��
  } elementCountList[] = {
    { "SCALAR", 1 }, { "VEC2", 2 }, { "VEC3", 3 }, { "VEC4", 4 },
  };

  // �Ή��\����v�f�����擾
  const std::string& type = accessor["type"].string_value();
  int elementCount = -1;
  for (const auto& e : elementCountList) {
    if (type == e.type) {
      elementCount = e.elementCount;
      break;
    }
  }
  if (elementCount < 0) {
    std::cerr << "[�G���[]" << __func__ << ": " << type <<
      "�͒��_�����ɐݒ�ł��܂���.\n";
    return false;
  }

  // VAO�ɒ��_�A�g���r���[�g��ݒ肷��
  const GLsizei byteStride = GetBufferStride(accessor, bufferViews);
  const GLsizeiptr offset = GetBufferOffset(accessor, bufferViews, binaryList);
  const GLenum componentType = accessor["componentType"].int_value();
  vao->SetAttribute(index, index, elementCount, componentType, GL_FALSE, 0);
  vao->SetVBO(index, buffer, offset, byteStride);

  return true;
}

/**
* ���_�A�g���r���[�g�̃f�t�H���g�l��ݒ肷��
*
* @param vao          ���_�A�g���r���[�g��ݒ肷��VAO
* @param index        ���_�A�g���r���[�g�ԍ��y�уo�C���f�B���O�|�C���g
* @param buffer       VBO�Ƃ��Ĉ����o�b�t�@�I�u�W�F�N�g
* @param elementCount �v�f��
* @param offset       buffer���̃f�[�^���z�u�ʒu(�P��=�o�C�g)
*
* �V�F�[�_���K�v�Ƃ��钸�_�A�g���r���[�g�ɂ��āA�v���~�e�B�u���Ή����钸�_�f�[�^��
* �����Ȃ��ꍇ�A���̊֐��ɂ���ăf�t�H���g�l��ݒ肷��B
*/
void SetDefaultAttribute(VertexArrayObjectPtr& vao, int index, GLuint buffer,
  GLint elementCount, GLsizeiptr offset)
{
  vao->SetAttribute(index, index, elementCount, GL_FLOAT, GL_FALSE, 0);
  vao->SetVBO(index, buffer, offset, 0);
}

/**
* JSON�̔z��f�[�^��glm::vec3�ɕϊ�����
*
* @param json �ϊ����ƂȂ�z��f�[�^
*
* @return json��ϊ����Ăł���vec3�̒l
*/
glm::vec3 GetVec3(const Json& json)
{
  const std::vector<Json>& a = json.array_items();
  if (a.size() < 3) {
    return glm::vec3(0);
  }
  return glm::vec3(
    a[0].number_value(),
    a[1].number_value(),
    a[2].number_value());
}

/**
* JSON�̔z��f�[�^��glm::quat�ɕϊ�����
*
* @param json �ϊ����ƂȂ�z��f�[�^
*
* @return json��ϊ����Ăł���quat�̒l
*/
glm::quat GetQuat(const Json& json)
{
  const std::vector<Json>& a = json.array_items();
  if (a.size() < 4) {
    return glm::quat(0, 0, 0, 1);
  }
  return glm::quat(
    static_cast<float>(a[3].number_value()),
    static_cast<float>(a[0].number_value()),
    static_cast<float>(a[1].number_value()),
    static_cast<float>(a[2].number_value())
  );
}

/**
* JSON�̔z��f�[�^��glm::mat4�ɕϊ�����
*
* @param json �ϊ����ƂȂ�z��f�[�^
*
* @return json��ϊ����Ăł���mat4�̒l
*/
glm::mat4 GetMat4(const Json& json)
{
  const std::vector<Json>& a = json.array_items();
  if (a.size() < 16) {
    return glm::mat4(1);
  }
  glm::mat4 m;
  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      m[y][x] = static_cast<float>(a[y * 4 + x].number_value());
    }
  }
  return m;
}

/**
* �m�[�h�̃��[�J���p���s����v�Z����
*
* @param node gltf�m�[�h
*
* @return node�̃��[�J���p���s��
*/
glm::mat4 CalcLocalMatrix(const Json& node)
{
  if (node["matrix"].is_array()) {
    return GetMat4(node["matrix"]);
  } else {
    glm::mat4 m(1);
    if (node["translation"].is_array()) {
      m *= glm::translate(glm::mat4(1), GetVec3(node["translation"]));
    }
    if (node["rotation"].is_array()) {
      m *= glm::mat4_cast(GetQuat(node["rotation"]));
    }
    if (node["scale"].is_array()) {
      m *= glm::scale(glm::mat4(1), GetVec3(node["scale"]));
    }
    return m;
  }
}

/**
* ���b�V�������m�[�h�����X�g�A�b�v����
*/
void GetMeshNodeList(const GltfNode* node, std::vector<const GltfNode*>& list)
{
  if (node->mesh >= 0) {
    list.push_back(node);
  }
  for (const auto& child : node->children) {
    GetMeshNodeList(child, list);
  }
}

// �o�C���f�B���O�|�C���g
enum BindingPoint
{
  bpPosition,
  bpColor,
  bpTexcoord0,
  bpNormal,
  bpWeights0,
  bpJoints0,
};

/**
* �f�t�H���g���_�f�[�^
*
* ���_�f�[�^�ɑΉ�����v�f���Ȃ��ꍇ�Ɏg���ėp�f�[�^
*/
struct DefaultVertexData
{
  glm::vec4 color = glm::vec4(1);
  glm::vec2 texcoord = glm::vec2(0);
  glm::vec3 normal = glm::vec3(0, 0, -1);
  glm::vec4 joints = glm::vec4(0);
  glm::vec4 weights = glm::vec4(0, 0, 0, 0);
};

} // unnamed namespace

/**
* �R���X�g���N�^
*
* @param maxBufferSize ���b�V���i�[�p�o�b�t�@�̍ő�o�C�g��
* @param maxMatrixSize �A�j���[�V�����pSSBO�Ɋi�[�ł���ő�s��
*/
GltfFileBuffer::GltfFileBuffer(size_t maxBufferSize, size_t maxMatrixSize)
{
  const GLsizei defaultDataSize = static_cast<GLsizei>(sizeof(DefaultVertexData));

  this->maxBufferSize = static_cast<GLsizei>(maxBufferSize + defaultDataSize);
  buffer = GLContext::CreateBuffer(this->maxBufferSize, nullptr);

  // �o�b�t�@�̐擪�Ƀ_�~�[�f�[�^��ݒ�
  const DefaultVertexData defaultData;
  CopyData(buffer, 1, 0, defaultDataSize, &defaultData);
  curBufferSize = defaultDataSize;

  // �A�j���[�V�������b�V���p�̃o�b�t�@���쐬
  ssbo = std::make_shared<ShaderStorageBuffer>(maxMatrixSize * sizeof(glm::mat4));
  dataBuffer.reserve(maxMatrixSize);
}

/**
* �f�X�g���N�^
*/
GltfFileBuffer::~GltfFileBuffer()
{
  glDeleteBuffers(1, &buffer);
}

/**
* �t�@�C�����烁�b�V����ǂݍ���
*
* @param filename glTF�t�@�C����
*/
bool GltfFileBuffer::AddFromFile(const char* filename)
{
  // glTF�t�@�C����ǂݍ���
  std::vector<char> buf = ReadFile(filename);
  if (buf.empty()) {
    return false;
  }
  buf.push_back('\0');

  // json���
  std::string err;
  const Json gltf = Json::parse(buf.data(), err);
  if (!err.empty()) {
    std::cerr << "[�G���[]" << __func__ << " '" << filename <<
      "'�̓ǂݍ��݂Ɏ��s���܂���.\n";
    std::cerr << err << "\n";
    return false;
  }

  // �t�H���_�������o��
  std::string foldername(filename);
  const size_t lastSlashPos = foldername.find_last_of("/\\");
  if (lastSlashPos == std::string::npos) {
    foldername.clear();
  } else {
    foldername.resize(lastSlashPos + 1);
  }

  // �o�C�i���t�@�C����ǂݍ���
  const GLsizei prevBufferSize = curBufferSize;
  const std::vector<Json>& buffers = gltf["buffers"].array_items();
  BinaryList binaryList(buffers.size());
  for (size_t i = 0; i < buffers.size(); ++i) {
    const Json& uri = buffers[i]["uri"];
    if (!uri.is_string()) {
      std::cerr << "[�G���[]" << __func__ << ": " << filename <<
        "�ɕs����uri������܂�.\n";
      return false;
    }
    const std::string binPath = foldername + uri.string_value();
    binaryList[i].bin = ReadFile(binPath.c_str());
    if (binaryList[i].bin.empty()) {
      curBufferSize = prevBufferSize;
      return false;
    }

    // �o�C�i���f�[�^��GPU�������ɃR�s�[
    CopyData(buffer, 1, curBufferSize, binaryList[i].bin.size(), binaryList[i].bin.data());

    // �I�t�Z�b�g���X�V
    binaryList[i].offset = curBufferSize;
    curBufferSize += static_cast<GLsizei>(binaryList[i].bin.size());
  }

  // �A�N�Z�b�T����f�[�^���擾����GPU�֓]��
  const Json& accessors = gltf["accessors"];
  const Json& bufferViews = gltf["bufferViews"];

  // �C���f�b�N�X�f�[�^�ƒ��_�����f�[�^�̃A�N�Z�b�TID���擾
  GltfFilePtr file = std::make_shared<GltfFile>();
  const std::vector<Json>& mesheArray = gltf["meshes"].array_items();
  file->meshes.reserve(mesheArray.size());
  for (const Json& currentMesh : mesheArray) {
    GltfMesh mesh;

    // ���b�V�������擾
    mesh.name = currentMesh["name"].string_value();

    // �v���~�e�B�u���쐬
    const std::vector<Json>& primitiveArray = currentMesh["primitives"].array_items();
    mesh.primitives.reserve(primitiveArray.size());
    for (const Json& currentPrim : primitiveArray) {
      GltfPrimitive prim;

      // �C���f�b�N�X�f�[�^
      {
        const int id = currentPrim["indices"].int_value();
        const Json& accessor = accessors[id];
        const std::string type = accessor["type"].string_value();
        if (type != "SCALAR") {
          std::cerr << "[�G���[]" << __func__ << "�C���f�b�N�X�f�[�^�E�^�C�v��SCALAR�łȂ��Ă͂Ȃ�܂���\n";
          std::cerr << "  type = " << type << "\n";
          return false;
        }

        // �v���~�e�B�u�̎��
        const Json& mode = currentPrim["mode"];
        if (mode.is_number()) {
          prim.mode = mode.int_value();
        }

        // �C���f�b�N�X��
        prim.count = accessor["count"].int_value();

        // �C���f�b�N�X�f�[�^�̌^
        prim.type = accessor["componentType"].int_value();

        // �I�t�Z�b�g
        const GLsizeiptr offset = GetBufferOffset(accessor, bufferViews, binaryList);
        prim.indices = reinterpret_cast<const GLvoid*>(offset);
      }

      // ���_�A�g���r���[�g(���_���W)
      prim.vao = std::make_shared<VertexArrayObject>();
      prim.vao->SetIBO(buffer);
      const Json& attributes = currentPrim["attributes"];
      if (attributes["POSITION"].is_number()) {
        const int id = attributes["POSITION"].int_value();
        SetAttribute(prim.vao, bpPosition, buffer,
          accessors[id], bufferViews, binaryList);
      }

      // ���_�A�g���r���[�g(���_�̐F)
      if (attributes["COLOR"].is_number()) {
        const int id = attributes["COLOR"].int_value();
        SetAttribute(prim.vao, bpColor, buffer,
          accessors[id], bufferViews, binaryList);
      } else {
        SetDefaultAttribute(prim.vao, bpColor, buffer,
          4, offsetof(DefaultVertexData, color));
      }

      // ���_�A�g���r���[�g(�e�N�X�`�����W)
      if (attributes["TEXCOORD_0"].is_number()) {
        const int id = attributes["TEXCOORD_0"].int_value();
        SetAttribute(prim.vao, bpTexcoord0, buffer,
          accessors[id], bufferViews, binaryList);
      } else {
        SetDefaultAttribute(prim.vao, bpTexcoord0, buffer,
          2, offsetof(DefaultVertexData, texcoord));
      }

      // ���_�A�g���r���[�g(�@��)
      if (attributes["NORMAL"].is_number()) {
        const int id = attributes["NORMAL"].int_value();
        SetAttribute(prim.vao, bpNormal, buffer,
          accessors[id], bufferViews, binaryList);
      } else {
        SetDefaultAttribute(prim.vao, bpNormal, buffer,
          3, offsetof(DefaultVertexData, normal));
      }

      // ���_�A�g���r���[�g(�W���C���g)
      if (attributes["JOINTS_0"].is_number()) {
        const int id = attributes["JOINTS_0"].int_value();
        SetAttribute(prim.vao, bpJoints0, buffer,
          accessors[id], bufferViews, binaryList);
      } else {
        SetDefaultAttribute(prim.vao, bpJoints0, buffer,
          4, offsetof(DefaultVertexData, joints));
      }

      // ���_�A�g���r���[�g(�E�F�C�g)
      if (attributes["WEIGHTS_0"].is_number()) {
        const int id = attributes["WEIGHTS_0"].int_value();
        SetAttribute(prim.vao, bpWeights0, buffer,
          accessors[id], bufferViews, binaryList);
      } else {
        SetDefaultAttribute(prim.vao, bpWeights0, buffer,
          4, offsetof(DefaultVertexData, weights));
      }

      prim.materialNo = currentPrim["material"].int_value();

      // �쐬�����v���~�e�B�u��z��ɒǉ�
      mesh.primitives.push_back(prim);
    }

    // �쐬�������b�V����z��ɒǉ�
    file->meshes.push_back(mesh);
  }

  // �}�e���A��
  GameEngine& engine = GameEngine::Get();
  const std::vector<Json>& materials = gltf["materials"].array_items();
  const std::vector<Json>& textures = gltf["textures"].array_items();
  const std::vector<Json>& images = gltf["images"].array_items();
  file->materials.reserve(materials.size());
  for (const Json& material : materials) {
    const Json& pbr = material["pbrMetallicRoughness"];

    // �e�N�X�`����ǂݍ���
    std::shared_ptr<Texture> texBaseColor;
    const Json& textureNo = pbr["baseColorTexture"]["index"];
    if (textureNo.is_number()) {
      const Json& texture = textures[textureNo.int_value()];
      const int imageSourceNo = texture["source"].int_value();
      const Json& imageUri = images[imageSourceNo]["uri"];
      if (imageUri.is_string()) {
        std::string filename = foldername + imageUri.string_value();
        const size_t n = filename.find_last_of('.');
        if (n != std::string::npos) {
          filename.resize(n);
        }
        filename += ".tga";
        texBaseColor = engine.LoadTexture(filename.c_str());
      }
    }

    // �}�e���A���J���[���擾
    glm::vec4 baseColor(1);
    const std::vector<Json>& baseColorFactor = pbr["baseColorFactor"].array_items();
    if (baseColorFactor.size() >= 4) {
      for (int i = 0; i < 4; ++i) {
        baseColor[i] = static_cast<float>(baseColorFactor[i].number_value());
      }
    }

    // �擾�����f�[�^����}�e���A�����쐬
    file->materials.push_back({ baseColor, texBaseColor });
  }

  // �m�[�h�c���[
  {
    const std::vector<Json>& nodes = gltf["nodes"].array_items();
    file->nodes.resize(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
      // �e�q�֌W���\�z
      // NOTE: �|�C���^���C���f�b�N�X�̂ق������S�����������������������H
      const Json& node = nodes[i];
      GltfNode& n = file->nodes[i];
      n.name = node["name"].string_value();

      const std::vector<Json>& children = node["children"].array_items();
      n.children.reserve(children.size());
      for (const auto& e : children) {
        const int childJointId = e.int_value();
        n.children.push_back(&file->nodes[childJointId]);
        if (!file->nodes[childJointId].parent) {
          file->nodes[childJointId].parent = &n;
        }
      }

      // ���[�J�����W�ϊ��s����v�Z.
      n.matLocal = CalcLocalMatrix(nodes[i]);
    }

    // �O���[�o�����W�ϊ��s����v�Z
    for (auto& e : file->nodes) {
      e.matGlobal = e.matLocal;
      GltfNode* parent = e.parent;
      while (parent) {
        e.matGlobal = parent->matLocal * e.matGlobal;
        parent = parent->parent;
      }
    }
  }

  // �X�L���f�[�^���擾
  const std::vector<Json>& skins = gltf["skins"].array_items();
  file->skins.reserve(skins.size());
  for (const auto& skin : skins) {
    GltfSkin tmpSkin;

    const Json& accessor = accessors[skin["inverseBindMatrices"].int_value()];
    if (accessor["type"].string_value() != "MAT4") {
      std::cerr << "ERROR: �o�C���h�|�[�Y��type��MAT4�łȂ��Ă͂Ȃ�܂��� \n";
      std::cerr << "  type = " << accessor["type"].string_value() << "\n";
      return false;
    }
    if (accessor["componentType"].int_value() != GL_FLOAT) {
      std::cerr << "ERROR: �o�C���h�|�[�Y��componentType��GL_FLOAT�łȂ��Ă͂Ȃ�܂��� \n";
      std::cerr << "  type = 0x" << std::hex << accessor["componentType"].string_value() << "\n";
      return false;
    }

    // �t�o�C���h�|�[�Y�s����擾
    // @note glTF�̃o�b�t�@�f�[�^�̓��g���G���f�B�A��. �d�l�ɏ����Ă���
    //       ���s���ɂ���Ă͕ϊ��̕K�v����
    const glm::mat4* inverseBindPoseList =
      static_cast<const glm::mat4*>(GetBuffer(accessor, bufferViews, binaryList));

    // �֐߃f�[�^���擾
    const std::vector<Json>& joints = skin["joints"].array_items();
    tmpSkin.joints.resize(joints.size());
    for (size_t i = 0; i < joints.size(); ++i) {
      tmpSkin.joints[i].nodeId = joints[i].int_value();
      tmpSkin.joints[i].matInverseBindPose = inverseBindPoseList[i];
    }
    tmpSkin.name = skin["name"].string_value();
    file->skins.push_back(tmpSkin);
  }

  {
    const std::vector<Json>& nodes = gltf["nodes"].array_items();
    for (size_t i = 0; i < nodes.size(); ++i) {
      const Json& meshId = nodes[i]["mesh"];
      if (meshId.is_number()) {
        file->nodes[i].mesh = meshId.int_value();
      }
      const Json& skinId = nodes[i]["skin"];
      if (skinId.is_number()) {
        file->nodes[i].skin = skinId.int_value();
      }
    }

    // �V�[���Ɋ܂܂�郁�b�V���m�[�h���擾
    // @note ���b�V���m�[�h������`�悷��΂悢�̂ŁAscenes��nodes�z���
    //       �Ή�����m�[�h��ێ�����K�v�͂Ȃ��B
    //       �m�[�h�s��̍X�V�͐e�m�[�h��H�邱�ƂŎ����ł���B
    const std::vector<Json>& scenes = gltf["scenes"].array_items();
    file->scenes.resize(scenes.size());
    for (size_t i = 0; i < scenes.size(); ++i) {
      const std::vector<Json>& nodes = scenes[i]["nodes"].array_items();
      auto& scene = file->scenes[i];
      for (auto& e : nodes) {
        auto node = &file->nodes[e.int_value()];
        scene.nodes.push_back(node);
        GetMeshNodeList(node, scene.meshNodes);
      }
    }
  }

  // �A�j���[�V�����f�[�^���擾
  for (const auto& animation : gltf["animations"].array_items()) {
    GltfAnimationPtr anime = std::make_shared<GltfAnimation>();
    anime->translationList.reserve(32);
    anime->rotationList.reserve(32);
    anime->scaleList.reserve(32);
    anime->name = animation["name"].string_value();

    const std::vector<Json>& channels = animation["channels"].array_items();
    const std::vector<Json>& samplers = animation["samplers"].array_items();
    for (const Json& e : channels) {
      const int samplerId = e["sampler"].int_value();
      const Json& sampler = samplers[samplerId];
      const Json& target = e["target"];
      const int targetNodeId = target["node"].int_value();
      if (targetNodeId < 0) {
        continue;
      }

      const int inputAccessorId = sampler["input"].int_value();
      const int inputCount = accessors[inputAccessorId]["count"].int_value();
      const void* pInput = GetBuffer(accessors[inputAccessorId], bufferViews, binaryList);

      const int outputAccessorId = sampler["output"].int_value();
      const int outputCount = accessors[outputAccessorId]["count"].int_value();
      const void* pOutput = GetBuffer(accessors[outputAccessorId], bufferViews, binaryList);

      const std::string& path = target["path"].string_value();
      anime->totalTime = 0;
      if (path == "translation") {
        const GLfloat* pKeyFrame = static_cast<const GLfloat*>(pInput);
        const glm::vec3* pData = static_cast<const glm::vec3*>(pOutput);
        GltfTimeline<glm::vec3> timeline;
        timeline.timeline.reserve(inputCount);
        for (int i = 0; i < inputCount; ++i) {
          anime->totalTime = std::max(anime->totalTime, pKeyFrame[i]);
          timeline.timeline.push_back({ pKeyFrame[i], pData[i] });
        }
        timeline.targetNodeId = targetNodeId;
        anime->translationList.push_back(timeline);
      } else if (path == "rotation") {
        const GLfloat* pKeyFrame = static_cast<const GLfloat*>(pInput);
        const glm::quat* pData = static_cast<const glm::quat*>(pOutput);
        GltfTimeline<glm::quat> timeline;
        timeline.timeline.reserve(inputCount);
        for (int i = 0; i < inputCount; ++i) {
          anime->totalTime = std::max(anime->totalTime, pKeyFrame[i]);
          timeline.timeline.push_back({ pKeyFrame[i], pData[i] });
        }
        timeline.targetNodeId = targetNodeId;
        anime->rotationList.push_back(timeline);
      } else if (path == "scale") {
        const GLfloat* pKeyFrame = static_cast<const GLfloat*>(pInput);
        const glm::vec3* pData = static_cast<const glm::vec3*>(pOutput);
        GltfTimeline<glm::vec3> timeline;
        timeline.timeline.reserve(inputCount);
        for (int i = 0; i < inputCount; ++i) {
          anime->totalTime = std::max(anime->totalTime, pKeyFrame[i]);
          timeline.timeline.push_back({ pKeyFrame[i], pData[i] });
        }
        timeline.targetNodeId = targetNodeId;
        anime->scaleList.push_back(timeline);
      }
    }
    file->animations.push_back(anime);
  }

  // �쐬�������b�V����A�z�z��ɒǉ�
  file->name = filename;
  files.emplace(filename, file);

  // �ǂݍ��񂾃��b�V�������f�o�b�O���Ƃ��ďo��
  std::cout << "[���]" << __func__ << ": '" << filename << "'��ǂݍ��݂܂���\n";
  for (size_t i = 0; i < file->meshes.size(); ++i) {
    std::cout << "  meshes[" << i << "]=\"" << file->meshes[i].name << "\"\n";
  }
  for (size_t i = 0; i < file->animations.size(); ++i) {
    std::string name = file->animations[i]->name;
    if (name.size() <= 0) {
      name = "<NO NAME>";
    } else {
      name = std::string("\"") + name + "\"";
    }
    std::cout << "  animations[" << i << "]=" << name << "\n";
  }

  return true;
}

/**
* �t�@�C�����擾����
*/
GltfFilePtr GltfFileBuffer::GetFile(const char* filename) const
{
  const auto itr = files.find(filename);
  if (itr == files.end()) {
    return nullptr;
  }
  return itr->second;
}

/**
* �A�j���[�V�������b�V���̕`��p�f�[�^�����ׂč폜
*/
void GltfFileBuffer::ClearAnimationBuffer()
{
  dataBuffer.clear();
}

/**
* �A�j���[�V�������b�V���̕`��p�f�[�^��ǉ�
*/
GLintptr GltfFileBuffer::AddAnimationData(const AnimationMatrices& matBones)
{
  GLintptr offset = static_cast<GLintptr>(dataBuffer.size() * sizeof(glm::mat4));
  dataBuffer.insert(dataBuffer.end(), matBones.begin(), matBones.end());

  // SSBO�̃I�t�Z�b�g�A���C�����g�����𖞂������߂ɁA256�o�C�g���E�ɔz�u����B
  // 256��OpenGL�d�l�ŋ������ő�l(GeForce�n�����̒l���g���Ă���)�B
  dataBuffer.resize(((dataBuffer.size() + 3) / 4) * 4);
  return offset;
}

/**
* �A�j���[�V�������b�V���̕`��p�f�[�^��GPU�������ɃR�s�[
*/
void GltfFileBuffer::UploadAnimationBuffer()
{
  ssbo->BufferSubData(0, dataBuffer.size() * sizeof(glm::mat4), dataBuffer.data());
  ssbo->SwapBuffers();
}

/**
* �A�j���[�V�������b�V���̕`��Ɏg��SSBO�̈�����蓖�Ă�
*/
void GltfFileBuffer::BindAnimationBuffer(GLuint bindingPoint, GLintptr offset, GLsizeiptr size)
{
  if (size > 0) {
    ssbo->Bind(bindingPoint, offset, size);
  }
}

/**
* �A�j���[�V�������b�V���̕`��Ɏg��SSBO�̈�̊��蓖�Ă���������
*/
void GltfFileBuffer::UnbindAnimationBuffer(GLuint bindingPoint)
{
  ssbo->Unbind(bindingPoint);
}

/**
* �A�j���[�V�����^�C�����C����̎w�肵�����Ԃ̒l�����߂�
*/
glm::vec3 Interporation(const GltfTimeline<glm::vec3>& data, float frame)
{
  const auto maxFrame = std::lower_bound(data.timeline.begin(), data.timeline.end(), frame,
    [](const GltfKeyFrame<glm::vec3>& keyFrame, float frame) { return keyFrame.frame < frame; });
  if (maxFrame == data.timeline.begin()) {
    return data.timeline.front().value;
  }
  if (maxFrame == data.timeline.end()) {
    return data.timeline.back().value;
  }
  const auto minFrame = maxFrame - 1;
  const float ratio = glm::clamp((frame - minFrame->frame) / (maxFrame->frame - minFrame->frame), 0.0f, 1.0f);
  return glm::mix(minFrame->value, maxFrame->value, ratio);
}

/**
* �A�j���[�V�����^�C�����C����̎w�肵�����Ԃ̒l�����߂�
*/
glm::quat Interporation(const GltfTimeline<glm::quat>& data, float frame)
{
  const auto maxFrame = std::lower_bound(data.timeline.begin(), data.timeline.end(), frame,
    [](const GltfKeyFrame<glm::quat>& keyFrame, float frame) { return keyFrame.frame < frame; });
  if (maxFrame == data.timeline.begin()) {
    return data.timeline.front().value;
  }
  if (maxFrame == data.timeline.end()) {
    return data.timeline.back().value;
  }
  const auto minFrame = maxFrame - 1;
  const float ratio = glm::clamp((frame - minFrame->frame) / (maxFrame->frame - minFrame->frame), 0.0f, 1.0f);
  return glm::slerp(minFrame->value, maxFrame->value, ratio);
}

