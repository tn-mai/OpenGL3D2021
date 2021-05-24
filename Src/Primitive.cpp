/**
* @file Primitive.cpp
*/
#define _CRT_SECURE_NO_WARNINGS
#include "Primitive.h"
#include "GLContext.h"
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
* �v���~�e�B�u��`�悷��.
*/
void Primitive::Draw() const
{
  glDrawElementsBaseVertex(mode, count, GL_UNSIGNED_SHORT, indices, baseVertex);
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
  vboNormal = GLContext::CreateBuffer(sizeof(glm::vec2) * maxVertexCount, nullptr);
  ibo = GLContext::CreateBuffer(sizeof(GLushort) * maxIndexCount, nullptr);
  vao = GLContext::CreateVertexArray(vboPosition, vboColor, vboTexcoord, vboNormal, ibo);
  if (!vboPosition || !vboColor || !vboTexcoord || !vboNormal || !ibo || !vao) {
    std::cerr << "[�G���[]" << __func__ << ": VAO�̍쐬�Ɏ��s.\n";
  }

  primitives.reserve(1000);

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
  size_t indexCount, const GLushort* pIndex)
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

  // GPU�������ɃC���f�b�N�X�f�[�^���R�s�[.
  if (!CopyData(ibo, sizeof(GLushort), curIndexCount, indexCount, pIndex)) {
    return false;
  }

  // �`��f�[�^���쐬.
  const Primitive prim(GL_TRIANGLES, static_cast<GLsizei>(indexCount),
    sizeof(GLushort) * curIndexCount, curVertexCount);

  // �`��f�[�^��z��ɒǉ�.
  primitives.push_back(prim);

  // ���݂̃f�[�^�����A�ǉ������f�[�^���������₷.
  curVertexCount += static_cast<GLsizei>(vertexCount);
  curIndexCount += static_cast<GLsizei>(indexCount);

  return true;
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

  // �f�[�^�ǂݎ��p�̕ϐ�������
  std::vector<glm::vec3> objPositions; // OBJ�t�@�C���̒��_���W�p
  std::vector<glm::vec2> objTexcoords; // OBJ�t�@�C���̃e�N�X�`�����W�p
  std::vector<glm::vec3> objNormals;   // OBJ�t�@�C���̖@��
  // �C���f�b�N�X�p
  struct Index {
    int v = 0;
    int vt = 0;
    int vn = 0;
  };
  std::vector<Index> objIndices; // OBJ�t�@�C���̃C���f�b�N�X

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
      Index f[3];
      bool isSuccess = false;
      if (sscanf(p, "%d/%d %d/%d %d/%d",
        &f[0].v, &f[0].vt,
        &f[1].v, &f[1].vt,
        &f[2].v, &f[2].vt) == 6) {
        isSuccess = true;
      } else if(sscanf(p, "%d/%d/%d %d/%d/%d %d/%d/%d",
        &f[0].v, &f[0].vt, &f[0].vn,
        &f[1].v, &f[1].vt, &f[0].vn,
        &f[2].v, &f[2].vt, &f[0].vn) == 9) {
        isSuccess = true;
      }

      if (isSuccess) {
        for (int i = 0; i < 3; ++i) {
          objIndices.push_back(f[i]);
        }
      } else {
        std::cerr << "[�x��]" << __func__ << ":�ʃf�[�^�̓ǂݎ��Ɏ��s.\n"
          "  " << filename << "(" << lineNo << "�s��): " << line << "\n";
      }

    } else {
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
    const int vt = objIndices[i].vt - 1;
    if (vt < static_cast<int>(objTexcoords.size())) {
      texcoords.push_back(objTexcoords[vt]);
    } else {
      std::cerr << "[�x��]" << __func__ << ":�e�N�X�`�����W�C���f�b�N�X" << vt <<
        "�͔͈�[0, " << objTexcoords.size() << ")�̊O���w���Ă��܂�.\n" <<
        "  " << filename << "\n";
      texcoords.push_back(glm::vec2(0));
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

  // �v���~�e�B�u��ǉ�����.
  const bool result = Add(positions.size(), positions.data(), colors.data(),
    texcoords.data(), normals.data(), indices.size(), indices.data());
  if (result) {
    std::cout << "[���]" << __func__ << ":" << filename << "(���_��=" <<
      positions.size() << " �C���f�b�N�X��=" << indices.size() << ")\n";
  } else {
    std::cerr << "[�G���[]" << __func__ << ":" << filename << "�̓ǂݍ��݂Ɏ��s.\n";
  }
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

