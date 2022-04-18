/**
* @file Primitive.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "Primitive.h"
#include "GLContext.h"
#include "GameEngine.h"
#include <fstream>
#include <string>
#include <stdio.h>
#include <iostream>

/**
* �f�[�^��GPU�������ɃR�s�[����.
*
* @param writeBuffer �R�s�[��̃o�b�t�@�I�u�W�F�N�g.
* @param unitSize    �v�f�̃o�C�g��.
* @param offsetCount �R�s�[��I�t�Z�b�g(�v�f�P��).
* @param count       �R�s�[����v�f��.
* @param data        �R�s�[����f�[�^�̃A�h���X.
*
* @retval true  �R�s�[����.
* @retval false �R�s�[���s.
*/
bool CopyData(GLuint writeBuffer, GLsizei unitSize,
  GLsizei offsetCount, size_t count, const void* data)
{
  const GLsizei size = static_cast<GLsizei>(unitSize * count);
  const GLuint readBuffer = GLContext::CreateBuffer(size, data);
  if (!readBuffer) {
    std::cerr << "[�G���[]" << __func__ << ": �R�s�[���o�b�t�@�̍쐬�Ɏ��s(size=" <<
      size << ").\n";
    return false;
  }
  const GLsizei offset = static_cast<GLsizei>(unitSize * offsetCount);
  glCopyNamedBufferSubData(readBuffer, writeBuffer, 0, offset, size);
  glDeleteBuffers(1, &readBuffer);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[�G���[]" << __func__ << ": �f�[�^�̃R�s�[�Ɏ��s(size=" <<
      size << ", offset=" << offset << ").\n";
    return false;
  }
  return true;
}

/**
* �}�e���A���Ŏg���Ă���e�N�X�`���̈ꗗ���擾����
*/
TextureList GetTextureList(
  const std::vector<Mesh::Material>& materials)
{
  TextureList textures;
  for (const auto& e : materials) {
    if (!e.tex) {
      continue;
    }
    const auto itr = std::find(textures.begin(), textures.end(), e.tex);
    if (itr == textures.end()) {
      textures.push_back(e.tex);
    }
  }
  return textures;
}

/**
* �}�e���A�����g���e�N�X�`���̔ԍ��ꗗ���擾����
*/
TextureIndexList GetTextureIndexList(
  const std::vector<Mesh::Material>& materials, const TextureList& textures)
{
  TextureIndexList indices(materials.size(), 0);
  for (int m = 0; m < materials.size(); ++m) {
    for (int i = 0; i < textures.size(); ++i) {
      if (textures[i] == materials[m].tex) {
        indices[m] = i;
        break;
      }
    }
  }
  return indices;
}

/**
* MTL�t�@�C������}�e���A����ǂݍ���
*
* @param  foldername �t�@�C���̂���t�H���_��
* @return filename   MTL�t�@�C����
*/
std::vector<Mesh::Material> LoadMaterial(
  const std::string& foldername, const std::string& filename)
{
  std::vector<Mesh::Material> materials;

  const std::string mtlname = foldername + filename;
  std::ifstream ifs(mtlname);
  if (!ifs) {
    return materials;
  }

  GameEngine& engine = GameEngine::Get();

  Mesh::Material m; // �f�[�^�ǂݎ��p�ϐ�
  size_t lineNo = 0; // �ǂݍ��񂾍s��
  while (!ifs.eof()) {
    std::string line;
    std::getline(ifs, line); // �t�@�C������1�s�ǂݍ���
    ++lineNo;

    // �s�̐擪�ɂ���󔒂�ǂݔ�΂�
    int n;
    char ctype[64];
    if (sscanf(line.c_str(), " %63s%n", ctype, &n) < 1) {
      continue;
    }
    // �R�����g�s�Ȃ疳�����Ď��̍s�֐i��
    if (ctype[0] == '#') {
      continue;
    }

    const char* p = line.c_str() + n; // ���l�������w���|�C���^

    // �^�C�v�ʂ̃f�[�^�ǂݍ��ݏ���
    if (strcmp(ctype, "newmtl") == 0) { // �}�e���A����
      // �ǂݎ�����}�e���A����z��ɒǉ�
      if (!m.name.empty()) {
        materials.push_back(m);
        m = Mesh::Material(); // �}�e���A���f�[�^��������
      }
      for (; *p == ' ' || *p == '\t'; ++p) {} // �擪�̋󔒂�����
      m.name = std::string(p);
    }
    else if (strcmp(ctype, "Kd") == 0) { // �f�B�t���[�Y�J���[
      if (sscanf(p, " %f %f %f", &m.color.x, &m.color.y, &m.color.z) != 3) {
        std::cerr << "[�x��]" << __func__ << ":�f�B�t���[�Y�J���[�̓ǂݎ��Ɏ��s.\n" <<
          "  " << mtlname << "(" << lineNo << "�s��): " << line << "\n";
      }
    }
    else if (strcmp(ctype, "d") == 0) { // �A���t�@�l
      if (sscanf(p, " %f", &m.color.w) != 1) {
        std::cerr << "[�x��]" << __func__ << ":�A���t�@�l�̓ǂݎ��Ɏ��s.\n" <<
          "  " << mtlname << "(" << lineNo << "�s��): " << line << "\n";
      }
    }
    else if (strcmp(ctype, "map_Kd") == 0) { // �f�B�t���[�Y�e�N�X�`��
      for (; *p == ' ' || *p == '\t'; ++p) {} // �擪�̋󔒂�����
      const std::string textureName = foldername + p;
      m.tex = engine.LoadTexture(textureName.c_str());
    }
  }

  // �Ō�ɓǂݎ�����}�e���A����z��ɒǉ�
  if (!m.name.empty()) {
    materials.push_back(m);
  }
  return materials;
}

/**
* �v���~�e�B�u��`�悷��.
*/
void Primitive::Draw() const
{
  glDrawElementsBaseVertex(mode, count, GL_UNSIGNED_SHORT, indices, baseVertex);
}

/**
* �C���X�^���V���O����Ńv���~�e�B�u��`�悷��
*
* @param instanceCount �`�悷��C���X�^���X��
*/
void Primitive::DrawInstanced(size_t instanceCount) const
{
  glDrawElementsInstancedBaseVertex(mode, count, GL_UNSIGNED_SHORT, indices,
    static_cast<GLsizei>(instanceCount), baseVertex);
}

/**
* �v���~�e�B�u�p�̃��������m�ۂ���.
*
* @param maxVertexCount  �i�[�\�ȍő咸�_��.
* @param maxIndexCount   �i�[�\�ȍő�C���f�b�N�X��.
*/
PrimitiveBuffer::PrimitiveBuffer(GLsizei maxVertexCount, GLsizei maxIndexCount)
{
  vboPosition = GLContext::CreateBuffer(sizeof(glm::vec3) * maxVertexCount, nullptr);
  vboColor = GLContext::CreateBuffer(sizeof(glm::vec4) * maxVertexCount, nullptr);
  vboTexcoord = GLContext::CreateBuffer(sizeof(glm::vec2) * maxVertexCount, nullptr);
  vboNormal = GLContext::CreateBuffer(sizeof(glm::vec3) * maxVertexCount, nullptr);
  vboMaterialGroup =
    GLContext::CreateBuffer(sizeof(glm::u8vec2) * maxVertexCount, nullptr);
  ibo = GLContext::CreateBuffer(sizeof(GLushort) * maxIndexCount, nullptr);
  vao = GLContext::CreateVertexArray(
    vboPosition, vboColor, vboTexcoord, vboNormal, vboMaterialGroup, ibo);
  if (!vboPosition || !vboColor || !vboTexcoord || !vboNormal || !ibo || !vao) {
    std::cerr << "[�G���[]" << __func__ << ": VAO�̍쐬�Ɏ��s.\n";
  }

  primitives.reserve(1000);
  meshes.reserve(1000);

  this->maxVertexCount = maxVertexCount;
  this->maxIndexCount = maxIndexCount;
}

/**
* �f�X�g���N�^.
*/
PrimitiveBuffer::~PrimitiveBuffer()
{
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &ibo);
  glDeleteBuffers(1, &vboMaterialGroup);
  glDeleteBuffers(1, &vboNormal);
  glDeleteBuffers(1, &vboTexcoord);
  glDeleteBuffers(1, &vboColor);
  glDeleteBuffers(1, &vboPosition);
}

/**
* �`��f�[�^��ǉ�����.
*
* @param vertexCount �ǉ����钸�_�f�[�^�̐�.
* @param pPosition   ���W�f�[�^�ւ̃|�C���^.
* @param pColor      �F�f�[�^�ւ̃|�C���^.
* @param pTexcoord   �e�N�X�`�����W�f�[�^�ւ̃|�C���^.
* @param pNormal     �@���f�[�^�ւ̃|�C���^.
* @param indexCount  �ǉ�����C���f�b�N�X�f�[�^�̐�.
* @param pIndex      �C���f�b�N�X�f�[�^�ւ̃|�C���^.
*
* @retval true  �ǉ��ɐ���.
* @retval false �ǉ��Ɏ��s.
*/
bool PrimitiveBuffer::Add(size_t vertexCount, const glm::vec3* pPosition,
  const glm::vec4* pColor, const glm::vec2* pTexcoord, const glm::vec3* pNormal,
  const glm::u8vec2* pMaterialGroup,
  size_t indexCount, const GLushort* pIndex, const char* name, GLenum type)
{
  // �G���[�`�F�b�N.
  if (!vao) {
    std::cerr << "[�G���[]" << __func__ <<
      ": VAO�̍쐬�Ɏ��s���Ă��܂�.\n";
    return false;
  } else if (vertexCount > static_cast<size_t>(maxVertexCount) - curVertexCount) {
    std::cerr << "[�x��]" << __func__ << ": VBO�����t�ł�(max=" << maxVertexCount <<
      ", cur=" << curVertexCount << ", add=" << vertexCount << ")\n";
    return false;
  } else if (indexCount > static_cast<size_t>(maxIndexCount) - curIndexCount) {
    std::cerr << "[�x��]" << __func__ << ": IBO�����t�ł�(max=" << maxIndexCount <<
      ", cur=" << curIndexCount << ", add=" << indexCount << ")\n";
    return false;
  }

  // GPU�������ɒ��_���W�f�[�^���R�s�[.
  if (!CopyData(vboPosition, sizeof(glm::vec3), curVertexCount, vertexCount,
    pPosition)) {
    return false;
  }

  // GPU�������ɐF�f�[�^���R�s�[.
  if (!CopyData(vboColor, sizeof(glm::vec4), curVertexCount, vertexCount, pColor)) {
    return false;
  }

  // GPU�������Ƀe�N�X�`�����W�f�[�^���R�s�[.
  if (!CopyData(vboTexcoord, sizeof(glm::vec2), curVertexCount, vertexCount,
    pTexcoord)) {
    return false;
  }

  // GPU�������ɖ@���f�[�^���R�s�[.
  if (!CopyData(vboNormal, sizeof(glm::vec3), curVertexCount, vertexCount, pNormal)) {
    return false;
  }

  // GPU�������Ƀ}�e���A���ԍ��E�O���[�v�ԍ����R�s�[
  // �f�[�^���Ȃ��ꍇ�̓_�~�[�f�[�^���R�s�[����
  std::vector<glm::u8vec2> dummy;
  if (!pMaterialGroup) {
    dummy.resize(vertexCount, glm::u8vec2(0));
    pMaterialGroup = dummy.data();
  }
  if (!CopyData(vboMaterialGroup, sizeof(glm::u8vec2), curVertexCount,
    vertexCount, pMaterialGroup)) {
    return false;
  }

  // GPU�������ɃC���f�b�N�X�f�[�^���R�s�[.
  if (!CopyData(ibo, sizeof(GLushort), curIndexCount, indexCount, pIndex)) {
    return false;
  }

  // �`��f�[�^���쐬.
  const Primitive prim(name, type, static_cast<GLsizei>(indexCount),
    sizeof(GLushort) * curIndexCount, curVertexCount);

  // �`��f�[�^��z��ɒǉ�.
  primitives.push_back(prim);

  // ���݂̃f�[�^�����A�ǉ������f�[�^���������₷.
  curVertexCount += static_cast<GLsizei>(vertexCount);
  curIndexCount += static_cast<GLsizei>(indexCount);

  return true;
}

// �C���f�b�N�X�p
struct Index {
  int v = 0;
  int vt = 0;
  int vn = 0;
};

/**
* ear-clipping algorithm.
*/
std::vector<Index> EarClipping(const std::vector<glm::vec3>& positions, std::vector<Index> f)
{
  std::vector<Index> result;
  result.reserve((f.size() - 2) * 3);

  // �ŏ��ɖ@���쐬�ɓK����3���_��T���Ė@�����쐬���A���̌�ear-clipping����.
  // �̂����A���݂̎����ł͖@���쐬�ɓK����3���_�̌��������܂����āA�Ӑ}�����O�p�`�ɂȂ�Ȃ��\��������.
  glm::vec3 normal;
  bool hasNormal = false;
  for (int i = 0; f.size() > 3; i = (i + 1) % static_cast<int>(f.size())) {
    const int size = static_cast<int>(f.size());
    const glm::vec3 a = positions[f[i + 0].v - 1];
    const glm::vec3 b = positions[f[(i + 1) % size].v - 1];
    const glm::vec3 c = positions[f[(i + 2) % size].v - 1];
    const glm::vec3 ab = b - a;
    const glm::vec3 bc = c - b;
    const glm::vec3 ca = a - c;
    if (hasNormal) {
      // ���p��180�x���傫�����Ear�ł͂Ȃ�.
      const glm::vec3 nba = a - b;
      const glm::vec3 s = glm::cross(bc, nba);
      if (glm::dot(normal, s) < 0) {
        continue;
      }
    }

    const glm::vec3 v = glm::cross(ab, bc); // �ʖ@��(��)
    bool isContain = false;
    for (int j = (i + 3) % size; j != i; j = (j + 1) % size) {
      const glm::vec3 p = positions[f[j].v - 1];
      const glm::vec3 ap = p - a;
      const glm::vec3 bp = p - b;
      const glm::vec3 cp = p - c;
      const float v0 = glm::dot(v, glm::cross(ab, bp));
      const float v1 = glm::dot(v, glm::cross(bc, cp));
      const float v2 = glm::dot(v, glm::cross(ca, ap));
      isContain |= (v0 > 0 && v1 > 0 && v2 > 0) || (v0 < 0 && v1 < 0 && v2 < 0);
      if (isContain) {
        break;
      }
    }
    // �����ɑ��̒��_���܂܂Ȃ��Ȃ�Ear�ł���.
    if (!isContain) {
      if (hasNormal) {
        result.push_back(f[i + 0]);
        result.push_back(f[(i + 1) % size]);
        result.push_back(f[(i + 2) % size]);
        f.erase(f.begin() + (i + 1) % size);
        --i;
      } else {
        hasNormal = true;
        normal = v;
        i = -1;
      }
    }
  }
  result.push_back(f[0]);
  result.push_back(f[1]);
  result.push_back(f[2]);

  return result;
}

/**
* OBJ�t�@�C������v���~�e�B�u��ǉ�����.
*
* @param filename ���[�h����OBJ�t�@�C����.
*
* @retval true  �ǉ�����.
* @retval false �ǉ����s.
*/
bool PrimitiveBuffer::AddFromObjFile(const char* filename)
{
  // �t�@�C�����J��.
  std::ifstream ifs(filename);
  if (!ifs) {
    std::cerr << "[�G���[]" << __func__ << ":`" << filename << "`���J���܂���.\n";
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

  // �f�[�^�ǂݎ��p�̕ϐ�������
  std::vector<glm::vec3> objPositions; // OBJ�t�@�C���̒��_���W�p
  std::vector<glm::vec2> objTexcoords; // OBJ�t�@�C���̃e�N�X�`�����W�p
  std::vector<glm::vec3> objNormals;   // OBJ�t�@�C���̖@��
  std::vector<Index> objIndices; // OBJ�t�@�C���̃C���f�b�N�X

  Mesh mesh;

  // �e�ʂ�\��.
  objPositions.reserve(10'000);
  objTexcoords.reserve(10'000);
  objNormals.reserve(10'000);
  objIndices.reserve(10'000);

  // �t�@�C�����烂�f���̃f�[�^��ǂݍ���.
  size_t lineNo = 0; // �ǂݍ��񂾍s��
  while (!ifs.eof()) {
    std::string line;
    std::getline(ifs, line); // �t�@�C������1�s�ǂݍ���
    ++lineNo;

    // �s�̐擪�ɂ���󔒂�ǂݔ�΂�.
    const size_t posData = line.find_first_not_of(' ');
    if (posData != std::string::npos) {
      line = line.substr(posData);
    }

    // ��s�܂��̓R�����g�s�Ȃ疳�����Ď��̍s�֐i��.
    if (line.empty() || line[0] == '#') {
      continue;
    }

    // �f�[�^�̎�ނ��擾.
    const size_t endOfType = line.find(' ');
    const std::string type = line.substr(0, endOfType);
    const char* p = line.c_str() + endOfType; // ���l�������w���|�C���^

    // �^�C�v�ʂ̃f�[�^�ǂݍ��ݏ���.
    if (type == "v") { // ���_���W
      glm::vec3 v(0);
      if (sscanf(p, "%f %f %f", &v.x, &v.y, &v.z) != 3) {
        std::cerr << "[�x��]" << __func__ << ":���_���W�̓ǂݎ��Ɏ��s.\n" <<
          "  " << filename << "(" << lineNo << "�s��): " << line << "\n";
      }
      objPositions.push_back(v);

    } else if (type == "vt") { // �e�N�X�`�����W
      glm::vec2 vt(0);
      if (sscanf(p, "%f %f", &vt.x, &vt.y) != 2) {
        std::cerr << "[�x��]" << __func__ << ":�e�N�X�`�����W�̓ǂݎ��Ɏ��s.\n" <<
          "  " << filename << "(" << lineNo << "�s��): " << line << "\n";
      }
      objTexcoords.push_back(vt);

    } else if (type == "vn") { // �@��
      glm::vec3 vn(0);
      if (sscanf(p, "%f %f %f", &vn.x, &vn.y, &vn.z) != 3) {
        std::cerr << "[�x��]" << __func__ << ":�@���̓ǂݎ��Ɏ��s.\n" <<
          "  " << filename << "(" << lineNo << "�s��): " << line << "\n";
      }
      objNormals.push_back(vn);

    } else if (type == "f") { // ��
      std::vector<Index> f;
      for (size_t i = 0; ; ++i) {
        int readBytes = 0;
        Index tmp;
        if (sscanf(p, " %d/%d/%d %n", &tmp.v, &tmp.vt, &tmp.vn, &readBytes) == 3) {
          f.push_back(tmp);
          p += readBytes;
        } else if (sscanf(p, " %d//%d %n", &tmp.v, &tmp.vn, &readBytes) == 2) {
          f.push_back(tmp);
          p += readBytes;
        } else if (sscanf(p, " %d/%d %n", &tmp.v, &tmp.vt, &readBytes) == 2) {
          f.push_back(tmp);
          p += readBytes;
        } else if (sscanf(p, " %d %n", &tmp.v, &readBytes) == 1) {
          f.push_back(tmp);
          p += readBytes;
        } else {
          break;
        }
      }

      if (f.size() >= 3) {
        if (false && f.size() > 3) {
          std::vector<Index> tmp = EarClipping(objPositions, f);
          objIndices.insert(objIndices.end(), tmp.begin(), tmp.end());
          // ���[�Y�}�e���A���ƃO���[�v�̃C���f�b�N�X�����X�V����
          if (!mesh.useMaterials.empty()) {
            mesh.useMaterials.back().indexCount += static_cast<GLsizei>(tmp.size());
          }
          if (!mesh.groups.empty()) {
            mesh.groups.back().indexCount += static_cast<GLsizei>(tmp.size());
          }
        } else {
          for (size_t i = 2; i < f.size(); ++i) {
            objIndices.push_back(f[0]);
            objIndices.push_back(f[i - 1]);
            objIndices.push_back(f[i]);
          }

          // �C���f�b�N�X�����X�V����
          const GLsizei triangleCount = static_cast<GLsizei>(f.size() - 2);
          if (!mesh.useMaterials.empty()) {
            mesh.useMaterials.back().indexCount += triangleCount * 3;
          }
          if (!mesh.groups.empty()) {
            mesh.groups.back().indexCount += triangleCount * 3;
          }
        }
      } else {
        std::cerr << "[�x��]" << __func__ << ":�ʃf�[�^�̓ǂݎ��Ɏ��s.\n"
          "  " << filename << "(" << lineNo << "�s��): " << line << "\n";
      }
    }
    else if (type == "mtllib") {
      for (; *p == ' '; ++p) {} // �擪�̋󔒂�����
      const std::vector<Mesh::Material> m = LoadMaterial(foldername, p);
      mesh.materials.insert(mesh.materials.end(), m.begin(), m.end());
    }
    else if (type == "g" || type == "o") {
      // �V�����O���[�v���쐬
      for (; *p == ' '; ++p) {} // �擪�̋󔒂�����
      mesh.groups.push_back(Mesh::Group());
      mesh.groups.back().name = p;
    }
    else if (type == "usemtl") {
      // �V�������[�Y�}�e���A�����쐬
      for (; *p == ' '; ++p) {} // �擪�̋󔒂�����
      mesh.useMaterials.push_back(Mesh::UseMaterial{});

      // ���O����v����}�e���A���̔ԍ���ݒ�
      for (int i = 0; i < mesh.materials.size(); ++i) {
        if (mesh.materials[i].name == p) {
          mesh.useMaterials.back().materialNo = i;
          break;
        }
      }
    }
    else {
      std::cerr << "[�x��]" << __func__ << ":���Ή��̌`���ł�.\n" <<
        "  " << filename << "(" << lineNo << "�s��): " << line << "\n";
    }
  }

  // �f�[�^�ϊ��p�̕ϐ�������.
  std::vector<glm::vec3> positions; // OpenGL�p�̒��_���W
  std::vector<glm::vec4> colors;    // OpenGL�p�̐F
  std::vector<glm::vec2> texcoords; // OpenGL�p�̃e�N�X�`�����W
  std::vector<glm::vec3> normals;   // OpenGL�p�̖@��
  std::vector<GLushort> indices;    // OpenGL�p�̃C���f�b�N�X

  // �f�[�^�ϊ��p�̃��������m��.
  const size_t indexCount = objIndices.size();
  positions.reserve(indexCount);
  texcoords.reserve(indexCount);
  normals.reserve(indexCount);
  indices.reserve(indexCount);

  // OBJ�t�@�C���̃f�[�^��OpenGL�̃f�[�^�ɕϊ�.
  for (size_t i = 0; i < indexCount; ++i) {
    // �C���f�b�N�X�f�[�^��ǉ�.
    indices.push_back(static_cast<GLushort>(i));

    // ���_���W��ϊ�.
    const int v = objIndices[i].v - 1;
    if (v < static_cast<int>(objPositions.size())) {
      positions.push_back(objPositions[v]);
    } else {
      std::cerr << "[�x��]" << __func__ << ":���_���W�C���f�b�N�X" << v <<
        "�͔͈�[0, " << objPositions.size() << ")�̊O���w���Ă��܂�.\n" <<
        "  " << filename << "\n";
      positions.push_back(glm::vec3(0));
    }

    // �e�N�X�`�����W��ϊ�.
    if (objIndices[i].vt == 0) {
      texcoords.push_back(glm::vec2(0));
    } else {
      const int vt = objIndices[i].vt - 1;
      if (vt < static_cast<int>(objTexcoords.size())) {
        texcoords.push_back(objTexcoords[vt]);
      } else {
        std::cerr << "[�x��]" << __func__ << ":�e�N�X�`�����W�C���f�b�N�X" << vt <<
          "�͔͈�[0, " << objTexcoords.size() << ")�̊O���w���Ă��܂�.\n" <<
          "  " << filename << "\n";
        texcoords.push_back(glm::vec2(0));
      }
    }

    // �@���f�[�^���Ȃ��ꍇ�A�ʂ̒��_���W����@�����v�Z����.
    if (objIndices[i].vn == 0) {
      // �ʂ̒��_���W��z��p�Ɏ擾.
      const size_t n = i / 3;
      glm::vec3 p[3];
      for (size_t j = 0; j < 3; ++j) {
        const int v = objIndices[n * 3 + j].v - 1;
        p[j] = objPositions[v];
      }

      // ��a�ƕ�b���v�Z.
      const glm::vec3 a = p[1] - p[0];
      const glm::vec3 b = p[2] - p[0];

      // a��b�ɐ����ȃx�N�g�����v�Z.
      glm::vec3 normal = glm::cross(a, b);

      // �����x�N�g���̒������v�Z.
      const float length = sqrt(
        normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);

      // �����x�N�g����P�ʃx�N�g���ɕϊ�.
      normal = normal / length;

      normals.push_back(normal);
    }
    else {
      // �@����ϊ�.
      const int vn = objIndices[i].vn - 1;
      if (vn < static_cast<int>(objNormals.size())) {
        normals.push_back(objNormals[vn]);
      } else {
        std::cerr << "[�x��]" << __func__ << ":�@���C���f�b�N�X" << vn <<
          "�͔͈�[0, " << objNormals.size() << ")�̊O���w���Ă��܂�.\n" <<
          "  " << filename << "\n";
        normals.push_back(glm::vec3(0, 1, 0));
      }
    }
  }

  // �F�f�[�^��ݒ�.
  colors.resize(positions.size(), glm::vec4(1));

  // �}�e���A���ԍ���ݒ�
  std::vector<glm::u8vec2> materialGroups(positions.size(), glm::u8vec2(0));
  GLsizei rangeFirst = 0;
  for (int useNo = 0; useNo < mesh.useMaterials.size(); ++useNo) {
    const Mesh::UseMaterial& m = mesh.useMaterials[useNo];
    const int maxMaterialNo = 9; // �V�F�[�_�Ŏg����}�e���A���͍ő�10��
    const int n = glm::clamp(m.materialNo, 0, maxMaterialNo);
    for (int i = 0; i < m.indexCount; ++i) {
      const int vertexNo = indices[rangeFirst + i];
      materialGroups[vertexNo].x = n;
    }
    rangeFirst += m.indexCount;
  }

  // �O���[�v�ԍ���ݒ�
  rangeFirst = 0;
  for (int groupNo = 0; groupNo < mesh.groups.size(); ++groupNo) {
    const Mesh::Group& g = mesh.groups[groupNo];
    for (int i = 0; i < g.indexCount; ++i) {
      const int vertexNo = indices[rangeFirst + i];
      materialGroups[vertexNo].y = groupNo;
    }
    rangeFirst += g.indexCount;
  }

  // ���_�f�[�^�ƃC���f�b�N�X�f�[�^��GPU�������ɃR�s�[����.
  const bool result = Add(positions.size(), positions.data(), colors.data(),
    texcoords.data(), normals.data(), materialGroups.data(),
    indices.size(), indices.data(), filename);
  if (result) {
    std::cout << "[���]" << __func__ << ":" << filename << "(���_��=" <<
      positions.size() << " �C���f�b�N�X��=" << indices.size() << ")\n";
  } else {
    std::cerr << "[�G���[]" << __func__ << ":" << filename << "�̓ǂݍ��݂Ɏ��s.\n";
  }

  // ���b�V����ǉ�����
  mesh.primitive = primitives.back();
  meshes.push_back(std::make_shared<Mesh>(mesh));

  return result;
}

/**
* �v���~�e�B�u���擾����.
*
* @param n �v���~�e�B�u�̃C���f�b�N�X.
*
* @return n�ɑΉ�����v���~�e�B�u.
*/
const Primitive& PrimitiveBuffer::Get(size_t n) const
{
  if (n > static_cast<int>(primitives.size())) {
    std::cerr << "[�x��]" << __func__ << ":" << n <<
      "�͖����ȃC���f�b�N�X�ł�(size=" << primitives.size() << ").\n";
    // ���̕`��f�[�^��Ԃ�.
    static const Primitive dummy;
    return dummy;
  }
  return primitives[n];
}

/**
* �v���~�e�B�u���擾����.
*
* @param name �v���~�e�B�u�̖��O
*
* @return ���O��name�ƈ�v����v���~�e�B�u.
*/
const Primitive& PrimitiveBuffer::Find(const char* name) const
{
  for (int i = 0; i < primitives.size(); ++i) {
    if (primitives[i].GetName() == name) {
      // ���O����v����`��f�[�^��������(�������`��f�[�^��Ԃ�)
      return primitives[i];
    }
  }

  // ���O����v����`��f�[�^�͌�����Ȃ�����(���̕`��f�[�^��Ԃ�)
  static const Primitive dummy;
  return dummy;
}

/**
* ���b�V�����擾����
*/
const MeshPtr& PrimitiveBuffer::GetMesh(const char* name) const
{
  for (size_t i = 0; i < meshes.size(); ++i) {
    if (meshes[i]->primitive.GetName() == name) {
      return meshes[i];
    }
  }
  static const MeshPtr dummy;
  return dummy;
}

/**
* VAO���O���t�B�b�N�X�p�C�v���C���Ƀo�C���h����.
*/
void PrimitiveBuffer::BindVertexArray() const
{
  glBindVertexArray(vao);
}

/**
* VAO�̃o�C���h����������.
*/
void PrimitiveBuffer::UnbindVertexArray() const
{
  glBindVertexArray(0);
}

