/**
* @file GLContext.cpp
*/
#include "GLContext.h"
#include <glm/glm.hpp>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

/**
* OpenGLコンテキストに関する機能を格納する名前空間.
*/
namespace GLContext {

/**
* バッファオブジェクトを作成する.
*
* @param size データのサイズ.
* @param data データへのポインタ.
*
* @return 作成したバッファオブジェクト.
*/
GLuint CreateBuffer(GLsizeiptr size, const GLvoid* data)
{
  GLuint id = 0;
  glCreateBuffers(1, &id);
  glNamedBufferStorage(id, size, data, 0);
  return id;
}

/**
* Vertex Array Objectを作成する.
*
* @param vboPosition VAOに関連付けられる座標データ.
* @param vboColor    VAOに関連付けられるカラーデータ.
* @param vboTexcoord VAOに関連付けられるテクスチャ座標データ.
* @param vboNormal   VAOに関連付けられる法線データ.
* @param ibo         VAOに関連付けられるインデックスデータ.
*
* @return 作成したVAO.
*/
GLuint CreateVertexArray(GLuint vboPosition, GLuint vboColor,
  GLuint vboTexcoord, GLuint vboNormal, GLuint vboMaterialGroup,
  GLuint ibo)
{
  if (!vboPosition || !vboColor || !vboTexcoord || !vboNormal ||
    !vboMaterialGroup || !ibo) {
    std::cerr << "[エラー]" << __func__ << ":バッファオブジェクトが0です。\n";
    return 0;
  }

  GLuint id = 0;
  glCreateVertexArrays(1, &id);

  const GLuint positionIndex = 0;
  const GLuint positionBindingIndex = 0;
  glEnableVertexArrayAttrib(id, positionIndex);
  glVertexArrayAttribFormat(id, positionIndex, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(id, positionIndex, positionBindingIndex);
  glVertexArrayVertexBuffer(
    id, positionBindingIndex, vboPosition, 0, sizeof(Position));

  const GLuint colorIndex = 1;
  const GLuint colorBindingIndex = 1;
  glEnableVertexArrayAttrib(id, colorIndex);
  glVertexArrayAttribFormat(id, colorIndex, 4, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(id, colorIndex, colorBindingIndex);
  glVertexArrayVertexBuffer(id, colorBindingIndex, vboColor, 0, sizeof(Color));

  const GLuint texcoordIndex = 2;
  const GLuint texcoordBindingIndex = 2;
  glEnableVertexArrayAttrib(id, texcoordIndex);
  glVertexArrayAttribFormat(id, texcoordIndex, 2, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(id,texcoordIndex, texcoordBindingIndex);
  glVertexArrayVertexBuffer(
    id, texcoordBindingIndex, vboTexcoord, 0, sizeof(glm::vec2));

  const GLuint normalIndex = 3;
  const GLuint normalBindingIndex = 3;
  glEnableVertexArrayAttrib(id, normalIndex);
  glVertexArrayAttribFormat(id, normalIndex, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(id,normalIndex, normalBindingIndex);
  glVertexArrayVertexBuffer(
    id, normalBindingIndex, vboNormal, 0, sizeof(glm::vec3));

  const GLuint groupIndex = 4;
  const GLuint groupBindingIndex = 4;
  glEnableVertexArrayAttrib(id, groupIndex);
  // NOTE: インテルドライバではglVertexArrayAttribIFormatが正しく動作しない
  //       以下のように旧来のAPIを使うか、floatで渡してシェーダでキャストする
  // 参考: https://community.khronos.org/t/data-alignment-in-shader/106560
#if 0
  glVertexArrayAttribIFormat(id, groupIndex, 2, GL_UNSIGNED_BYTE, 0);
#else
  glBindBuffer(GL_ARRAY_BUFFER, vboMaterialGroup);
  glBindVertexArray(id);
  glVertexAttribIPointer(groupIndex, 2, GL_UNSIGNED_BYTE, sizeof(glm::u8vec2), 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
  glVertexArrayAttribBinding(id,groupIndex, groupBindingIndex);
  glVertexArrayVertexBuffer(
    id, groupBindingIndex, vboMaterialGroup, 0, sizeof(glm::u8vec2));

  glVertexArrayElementBuffer(id, ibo);

  return id;
}

/**
* シェーダー・プログラムをビルドする.
*
* @param type シェーダーの種類.
* @param code シェーダー・プログラムへのポインタ.
*
* @retval 0より大きい 作成したプログラム・オブジェクト.
* @retval 0          プログラム・オブジェクトの作成に失敗.
*/
GLuint CreateProgram(GLenum type, const GLchar* code)
{
  GLuint program = glCreateShaderProgramv(type, 1, &code);

  GLint status = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint infoLen = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
    if (infoLen) {
      std::vector<char> buf;
      buf.resize(infoLen);
      if ((int)buf.size() >= infoLen) {
        glGetProgramInfoLog(program, infoLen, nullptr, buf.data());
        std::cerr << "[エラー]" << __func__ <<
          ":シェーダーのビルドに失敗.\n" << buf.data() << "\n";
      }
    }
    glDeleteProgram(program);
    return 0;
  }
  return program;
}

/**
* ファイルからシェーダ・プログラムを作成する.
*
* @param type     シェーダの種類.
* @param filename シェーダファイル名.
*
* @retval 0より大きい 作成したプログラム・オブジェクト.
* @retval 0           プログラム・オブジェクトの作成に失敗.
*/
GLuint CreateProgramFromFile(GLenum type, const char* filename)
{
  std::ifstream ifs(filename);
  if (!ifs) {
    std::cerr << "[エラー]" << __func__ << ":" << filename <<
      "を開けません.\n";
    return 0;
  }
  std::stringstream ss;
  ss << ifs.rdbuf();
  return GLContext::CreateProgram(type, ss.str().c_str());
}

/**
*
*/
GLuint CreateProgramFromFile(const char* vsFile, const char* fsFile)
{
  const char* files[] = { vsFile, fsFile };
  std::string str[2];
  for (int i = 0; i < 2; ++i) {
    std::ifstream ifs(files[i]);
    if (!ifs) {
      std::cerr << "[エラー]" << __func__ << ":" << files[i] <<
        "を開けません.\n";
      return 0;
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    str[i] = ss.str();
  }

  const GLenum type[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
  GLuint shader[2];
  for (int i = 0; i < 2; ++i) {
    shader[i] = glCreateShader(type[i]);
    const char* p = str[i].c_str();
    glShaderSource(shader[i], 1, &p, nullptr);
    glCompileShader(shader[i]);
  }
  GLuint program = glCreateProgram();
  //glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
  glAttachShader(program, shader[0]);
  glAttachShader(program, shader[1]);
  glLinkProgram(program);
  glDeleteShader(shader[0]);
  glDeleteShader(shader[1]);

  return program;
}

/**
*
*/
GLuint CreatePipeline(GLuint program)
{
  GLuint id;
  glCreateProgramPipelines(1, &id);
  glUseProgramStages(id, GL_ALL_SHADER_BITS, program);
  return id;
}

/**
* パイプライン・オブジェクトを作成する.
*
* @param vp  頂点シェーダー・プログラム.
* @param fp  フラグメントシェーダー・プログラム.
*
* @retval 0より大きい 作成したパイプライン・オブジェクト.
* @retval 0         パイプライン・オブジェクトの作成に失敗.
*/
GLuint CreatePipeline(GLuint vp, GLuint fp)
{
  glGetError(); // エラー状態をリセット.

  GLuint id;
  glCreateProgramPipelines(1, &id);
  glUseProgramStages(id, GL_VERTEX_SHADER_BIT, vp);
  glUseProgramStages(id, GL_FRAGMENT_SHADER_BIT, fp);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":プログラムパイプラインの作成に失敗.\n";
    glDeleteProgramPipelines(1, &id);
    return 0;
  }

  GLint testVp = 0;
  glGetProgramPipelineiv(id, GL_VERTEX_SHADER, &testVp);
  if (testVp != vp) {
    std::cerr << "[エラー]" << __func__ << ":頂点シェーダの設定に失敗.\n";
    glDeleteProgramPipelines(1, &id);
    return 0;
  }
  GLint testFp = 0;
  glGetProgramPipelineiv(id, GL_FRAGMENT_SHADER, &testFp);
  if (testFp != fp) {
    std::cerr << "[エラー]" << __func__ << ":フラグメントシェーダの設定に失敗.\n";
    glDeleteProgramPipelines(1, &id);
    return 0;
  }
  return id;
}

/**
* サンプラ・オブジェクトを作成する.
*
* @param wrapMode  ラップ・モード.
*
* @retval 0より大きい 作成したサンプラ・オブジェクト.
* @retval 0          サンプラ・オブジェクトの作成に失敗.
*/
GLuint CreateSampler(GLenum wrapMode)
{
  GLuint id;
  glCreateSamplers(1, &id);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":ラップモードではない値が指定された.\n";
    glDeleteSamplers(1, &id);
    return 0;
  }

  // ラップ・モードを設定する.
  glSamplerParameteri(id, GL_TEXTURE_WRAP_S, wrapMode);
  glSamplerParameteri(id, GL_TEXTURE_WRAP_T, wrapMode);
  if (glGetError() != GL_NO_ERROR) {
    glDeleteSamplers(1, &id);
    return 0;
  }

  // フィルタを設定する.
  glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);//GL_NEAREST_MIPMAP_NEAREST);
  glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);// GL_NEAREST);
  if (glGetError() != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << ":フィルタではない値が指定された.\n";
    glDeleteSamplers(1, &id);
    return 0;
  }

  // NOTE: AMDドライバでGL_PROGRAM_SEPARATABLEを有効にした場合、コンペアモード指定が無視されるバグがある
  //       このため、本テキストではハードウェアPCFを使っていない
  //glSamplerParameteri(id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
  //glSamplerParameteri(id, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

  return id;
}

/**
* 2Dテクスチャを作成する.
*
* @param width   画像の幅(ピクセル数).
* @param height  画像の高さ(ピクセル数).
* @param data    画像データのアドレス.
* @param pixelFormat  画像データ形式(GL_BGRAなど).
* @param type    画像データの型.
*
* @retval 0以外  作成したテクスチャ・オブジェクトのID.
* @retval 0      テクスチャの作成に失敗.
*/
GLuint CreateImage2D(GLsizei width, GLsizei height, const void* data,
  GLenum pixelFormat, GLenum type)
{
  GLuint id;

  // 画像データ形式に対応する内部データ形式を選択する
  GLenum internalFormat = GL_RGBA8;
  switch (pixelFormat) {
  case GL_RED: internalFormat = GL_R8; break;
  case GL_RGB: internalFormat = GL_RGB8; break;
  case GL_BGR: internalFormat = GL_RGB8; break;
  case GL_DEPTH_COMPONENT16:
  case GL_DEPTH_COMPONENT24:
  case GL_DEPTH_COMPONENT32F:
    internalFormat = pixelFormat;
    break;
  }

  if (type == GL_HALF_FLOAT) {
    switch (internalFormat) {
    case GL_R8:    internalFormat = GL_R16F; break;
    case GL_RGB8:  internalFormat = GL_RGB16F; break;
    case GL_RGBA8: internalFormat = GL_RGBA16F; break;
    }
  } else if (type == GL_FLOAT) {
    switch (internalFormat) {
    case GL_R8:    internalFormat = GL_R32F; break;
    case GL_RGB8:  internalFormat = GL_RGB32F; break;
    case GL_RGBA8: internalFormat = GL_RGBA32F; break;
    }
  }

  // テクスチャ・オブジェクトを作成し、GPUメモリを確保する.
  glCreateTextures(GL_TEXTURE_2D, 1, &id);
  glTextureStorage2D(id, 1, internalFormat, width, height);

  // GPUメモリにデータを転送する.
  if (data) {
    GLint alignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTextureSubImage2D(id, 0, 0, 0, width, height, pixelFormat, type, data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
  }

  const GLenum result = glGetError();
  if (result != GL_NO_ERROR) {
    std::cerr << "[エラー]" << __func__ << "テクスチャの作成に失敗\n";
    glDeleteTextures(1, &id);
    return 0;
  }

  // 白黒画像の場合、(R,R,R,1)として読み取られるように設定する.
  if (pixelFormat == GL_RED) {
    glTextureParameteri(id, GL_TEXTURE_SWIZZLE_G, GL_RED);
    glTextureParameteri(id, GL_TEXTURE_SWIZZLE_B, GL_RED);
  }

  return id;
}

/**
* ファイルから2Dテクスチャを読み込む.
*
* @param filename 2Dテクスチャとして読み込むファイル名.
*
* @retval 0以外 作成したテクスチャ・オブジェクトのID.
*         0     テクスチャの作成に失敗.
*/
GLuint CreateImage2D(const char* filename)
{
  // ファイルを開く.
  std::ifstream ifs;
  ifs.open(filename, std::ios_base::binary);
  if (!ifs) {
    std::cerr << "[エラー]" << __func__ << ":`" << filename << "`を開けません.\n";
    return 0;
  }

  // TGAヘッダを読み込む.
  uint8_t tgaHeader[18];
  ifs.read(reinterpret_cast<char*>(tgaHeader), 18);

  // イメージIDを読み飛ばす.
  ifs.ignore(tgaHeader[0]);

  // カラーマップを読み飛ばす.
  if (tgaHeader[1]) {
    const int colorMapLength = tgaHeader[5] + tgaHeader[6] * 0x100;
    const int colorMapEntrySize = tgaHeader[7];
    // エントリーサイズはビット数なので、8で割ってバイト数に変換する.
    const int colorMapSize = (colorMapLength * colorMapEntrySize + 7) / 8;
    ifs.ignore(colorMapSize);
  }

  // 画像データを読み込む.
  const int width = tgaHeader[12] + tgaHeader[13] * 0x100;
  const int height = tgaHeader[14] + tgaHeader[15] * 0x100;
  const int pixelDepth = tgaHeader[16];
  const int imageSize = width * height * pixelDepth / 8;
  std::vector<uint8_t> buf(imageSize);
  ifs.read(reinterpret_cast<char*>(buf.data()), imageSize);

  // TGAヘッダ18バイト目の第5ビットは、画像データの格納方向を表す.
  //   0の場合: 下から上
  //   1の場合: 上から下
  // OpenGLは画像データを「下から上」に格納するルールになっているので、
  // TGAが「上から下」に格納されている場合は画像を上下反転する.
  if (tgaHeader[17] & 0x20) {
    const size_t lineSize = width * pixelDepth / 8;
    std::vector<uint8_t> tmp(imageSize);
    std::vector<uint8_t>::iterator source = buf.begin();
    std::vector<uint8_t>::iterator destination = tmp.end();
    for (size_t i = 0; i < height; ++i) {
      destination -= lineSize;
      std::copy(source, source + lineSize, destination);
      source += lineSize;
    }
    buf.swap(tmp);
  }

  // データの型を選ぶ.
  GLenum type = GL_UNSIGNED_BYTE;
  if (tgaHeader[16] == 16) {
    type = GL_UNSIGNED_SHORT_1_5_5_5_REV;
  }

  // ピクセル形式を選ぶ.
  GLenum pixelFormat = GL_BGRA;
  if (tgaHeader[2] == 3) { // 圧縮なし、白黒画像.
    pixelFormat = GL_RED;
  }
  if (tgaHeader[16] == 24) {
    pixelFormat = GL_BGR;
  }

  // 読み込んだ画像データからテクスチャを作成する.
  return CreateImage2D(width, height, buf.data(),
    pixelFormat, type);
}

} // namespace GLContext