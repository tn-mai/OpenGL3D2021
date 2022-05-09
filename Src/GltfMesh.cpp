/**
* @file GltfMesh.cpp
*/
#include "GltfMesh.h"
#include "VertexArrayObject.h"
#include "GLContext.h"
#include "GameEngine.h"
#include "json11/json11.hpp"
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
* �A�N�Z�b�T�������o�b�t�@�̃I�t�Z�b�g���擾����
*
* @param accessor    �A�N�Z�b�T
* @param bufferViews �o�b�t�@�r���[�z��
* @param binOffset   �o�C�i���f�[�^�̐擪�I�t�Z�b�g�z��
*/
GLsizeiptr GetBufferOffset(const Json& accessor,
  const Json& bufferViews, const std::vector<GLsizeiptr>& binOffset)
{
  // �A�N�Z�b�T����K�v�ȏ����擾
  const int byteOffset = accessor["byteOffset"].int_value();
  const int bufferViewId = accessor["bufferView"].int_value();

  // �o�b�t�@�r���[����K�v�ȏ����擾
  const Json bufferView = bufferViews[bufferViewId];
  const int bufferId = bufferView["buffer"].int_value();
  const int baesByteOffset = bufferView["byteOffset"].int_value();

  // �I�t�Z�b�g���v�Z
  return binOffset[bufferId] + baesByteOffset + byteOffset;
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
* ���_�A�g���r���[�g��ݒ肷��
* 
* @param vao         ���_�A�g���r���[�g��ݒ肷��VAO
* @param index       ���_�A�g���r���[�g�ԍ��y�уo�C���f�B���O�|�C���g
* @param buffer      VBO�Ƃ��Ĉ����o�b�t�@�I�u�W�F�N�g
* @param accessor    ���_�A�g���r���[�g�f�[�^�����A�N�Z�b�T
* @param bufferViews �o�b�t�@�r���[�z��
* @param binOffset   �o�C�i���f�[�^�̃I�t�Z�b�g�z��
*/
bool SetAttribute(VertexArrayObjectPtr& vao, int index, GLuint buffer,
  const Json& accessor, const Json& bufferViews,
  const std::vector<GLsizeiptr>& binOffset)
{
  // �^���Ɨv�f���̑Ή��\(���_�A�g���r���[�g�p)
  const struct {
    const char* type; // �^�̖��O
    int elementCount; // �v�f��
  } elementCountList[] = {
    { "SCALAR", 1 }, { "VEC2", 2 }, { "VEC3", 3 }, { "VEC4", 4 },
  };

  // �v�f��
  const std::string& type = accessor["type"].string_value();
  int elementCount = -1;
  for (const auto& e : elementCountList) {
    if (type == e.type) {
      elementCount = e.elementCount;
      break;
    }
  }
  if (elementCount < 0) {
    std::cerr << "[�G���[]" << __func__ << ": " << type << "�͒��_�����ɐݒ�ł��܂���.\n";
    return false;
  }

  const GLsizei byteStride = GetBufferStride(accessor, bufferViews);
  const GLsizeiptr offset = GetBufferOffset(accessor, bufferViews, binOffset);
  const GLenum componentType = accessor["componentType"].int_value();
  vao->SetAttribute(index, index, elementCount, componentType, GL_FALSE, 0);
  vao->SetVBO(index, buffer, offset, byteStride);

  return true;
}

} // unnamed namespace

/**
* �R���X�g���N�^
*
* @param maxBufferSize ���b�V���i�[�p�o�b�t�@�̍ő�o�C�g��
*/
GltfFileBuffer::GltfFileBuffer(size_t maxBufferSize)
{
  // �_�~�[�f�[�^
  static const struct {
    glm::vec4 color = glm::vec4(1);
    glm::vec4 texcoord = glm::vec4(0);
    glm::vec4 normal = glm::vec4(0, 0, -1, 0);
  } dummyData;
  const GLsizei dummyDataSize = static_cast<GLsizei>(sizeof(dummyData));

  this->maxBufferSize = static_cast<GLsizei>(maxBufferSize + dummyDataSize);
  buffer = GLContext::CreateBuffer(this->maxBufferSize, nullptr);

  // �o�b�t�@�̐擪�Ƀ_�~�[�f�[�^��ݒ�
  CopyData(buffer, 1, 0, sizeof(dummyData), &dummyData);
  curBufferSize = dummyDataSize;
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
    std::cerr << "[�G���[]" << __func__ << " '" << filename << "'�̓ǂݍ��݂Ɏ��s���܂���.\n";
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
  std::vector<GLsizeiptr> binOffset;
  for (const Json& e : gltf["buffers"].array_items()) {
    const Json& uri = e["uri"];
    if (!uri.is_string()) {
      std::cerr << "[�G���[]" << __func__ << ": " << filename << "�ɕs����uri������܂�.\n";
      return false;
    }
    const std::string binPath = foldername + uri.string_value();
    const std::vector<char> bin = ReadFile(binPath.c_str());
    if (bin.empty()) {
      curBufferSize = prevBufferSize;
      return false;
    }

    // �o�C�i���f�[�^��GPU�������ɃR�s�[
    CopyData(buffer, 1, curBufferSize, bin.size(), bin.data());
    binOffset.push_back(curBufferSize); // �o�C�i���f�[�^�̃I�t�Z�b�g��ݒ�
    curBufferSize += static_cast<GLsizei>(bin.size());
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

      // ���_�C���f�b�N�X
      {
        const int id = currentPrim["indices"].int_value();
        const Json& accessor = accessors[id];
        if (accessor["type"].string_value() != "SCALAR") {
          std::cerr << "[�G���[]" << __func__ << "�C���f�b�N�X�f�[�^�E�^�C�v��SCALAR�łȂ��Ă͂Ȃ�܂���\n";
          std::cerr << "  type = " << accessor["type"].string_value() << "\n";
          return false;
        }

        const Json& mode = currentPrim["mode"];
        if (mode.is_number()) {
          prim.mode = mode.int_value();
        }
        prim.count = accessor["count"].int_value();
        prim.type = accessor["componentType"].int_value();

        const GLsizeiptr offset = GetBufferOffset(accessor, bufferViews, binOffset);
        prim.indices = reinterpret_cast<const GLvoid*>(offset);
      }

      // ���_����
      prim.vao = std::make_shared<VertexArrayObject>();
      prim.vao->SetIBO(buffer);
      const Json& attributes = currentPrim["attributes"];
      if (attributes["POSITION"].is_number()) {
        const int id = attributes["POSITION"].int_value();
        SetAttribute(prim.vao, 0, buffer, accessors[id], bufferViews, binOffset);
      }
      if (attributes["COLOR"].is_number()) {
        const int id = attributes["COLOR"].int_value();
        SetAttribute(prim.vao, 1, buffer, accessors[id], bufferViews, binOffset);
      } else {
        prim.vao->SetAttribute(1, 1, 4, GL_FLOAT, GL_FALSE, 0);
        prim.vao->SetVBO(1, buffer, 0, 0);
      }
      if (attributes["TEXCOORD_0"].is_number()) {
        const int id = attributes["TEXCOORD_0"].int_value();
        SetAttribute(prim.vao, 2, buffer, accessors[id], bufferViews, binOffset);
      } else {
        prim.vao->SetAttribute(2, 2, 4, GL_FLOAT, GL_FALSE, 0);
        prim.vao->SetVBO(2, buffer, 16, 0);
      }
      if (attributes["NORMAL"].is_number()) {
        const int id = attributes["NORMAL"].int_value();
        SetAttribute(prim.vao, 3, buffer, accessors[id], bufferViews, binOffset);
      } else {
        prim.vao->SetAttribute(3, 3, 4, GL_FLOAT, GL_FALSE, 0);
        prim.vao->SetVBO(3, buffer, 32, 0);
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

  // �쐬�������b�V����A�z�z��ɒǉ�
  file->name = filename;
  files.emplace(filename, file);

  // �ǂݍ��񂾃��b�V�������f�o�b�O���Ƃ��ďo��
  std::cout << "[���]" << __func__ << ": '" << filename << "'��ǂݍ��݂܂���\n";
  for (size_t i = 0; i < file->meshes.size(); ++i) {
    std::cout << "  [" << i << "] " << file->meshes[i].name << "\n";
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

