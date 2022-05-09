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
* ファイルを読み込む
*
* @param filename 読み込むファイル名
*/
std::vector<char> ReadFile(const char* filename)
{
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "[エラー]" << __func__ << ":`" << filename << "`を開けません.\n";
    return {};
  }
  std::vector<char> buf(std::filesystem::file_size(filename));
  file.read(buf.data(), buf.size());
  return buf;
}

/**
* アクセッサが示すバッファのオフセットを取得する
*
* @param accessor    アクセッサ
* @param bufferViews バッファビュー配列
* @param binOffset   バイナリデータの先頭オフセット配列
*/
GLsizeiptr GetBufferOffset(const Json& accessor,
  const Json& bufferViews, const std::vector<GLsizeiptr>& binOffset)
{
  // アクセッサから必要な情報を取得
  const int byteOffset = accessor["byteOffset"].int_value();
  const int bufferViewId = accessor["bufferView"].int_value();

  // バッファビューから必要な情報を取得
  const Json bufferView = bufferViews[bufferViewId];
  const int bufferId = bufferView["buffer"].int_value();
  const int baesByteOffset = bufferView["byteOffset"].int_value();

  // オフセットを計算
  return binOffset[bufferId] + baesByteOffset + byteOffset;
}

/**
* アクセッサが示すバッファのオフセットを取得する
*
* @param accessor    アクセッサ
* @param bufferViews バッファビュー配列
*/
GLsizei GetBufferStride(const Json& accessor, const Json& bufferViews)
{
  const int bufferViewId = accessor["bufferView"].int_value();
  const Json bufferView = bufferViews[bufferViewId];

  // byteStrideが0より大きい値で定義されていたら、その値を返す
  const GLsizei stride = bufferView["byteStride"].int_value();
  if (stride > 0) {
    return stride;
  }

  // byteStrideが未定義または0だった場合、データ1個分のサイズを計算して返す

  // 要素型のサイズ
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
    std::cerr << "[警告]" << __func__ << ":glTFの仕様にない型" <<
      componentType << "が使われています\n";
    break;
  }

  // 型名と要素数の対応表
  const struct {
    const char* type; // 型の名前
    int elementCount; // 要素数
  } elementCountList[] = {
    { "SCALAR", 1 },
    { "VEC2", 2 }, { "VEC3", 3 }, { "VEC4", 4 },
    { "MAT2", 4 }, { "MAT3", 9 }, { "MAT4", 16 },
  };

  // 要素数
  const std::string& type = accessor["type"].string_value();
  int elementCount = 1;
  for (const auto& e : elementCountList) {
    if (type == e.type) {
      elementCount = e.elementCount;
      break;
    }
  }

  // 要素のサイズをストライドとする
  return componentSize * elementCount;
}

/**
* 頂点アトリビュートを設定する
* 
* @param vao         頂点アトリビュートを設定するVAO
* @param index       頂点アトリビュート番号及びバインディングポイント
* @param buffer      VBOとして扱うバッファオブジェクト
* @param accessor    頂点アトリビュートデータを持つアクセッサ
* @param bufferViews バッファビュー配列
* @param binOffset   バイナリデータのオフセット配列
*/
bool SetAttribute(VertexArrayObjectPtr& vao, int index, GLuint buffer,
  const Json& accessor, const Json& bufferViews,
  const std::vector<GLsizeiptr>& binOffset)
{
  // 型名と要素数の対応表(頂点アトリビュート用)
  const struct {
    const char* type; // 型の名前
    int elementCount; // 要素数
  } elementCountList[] = {
    { "SCALAR", 1 }, { "VEC2", 2 }, { "VEC3", 3 }, { "VEC4", 4 },
  };

  // 要素数
  const std::string& type = accessor["type"].string_value();
  int elementCount = -1;
  for (const auto& e : elementCountList) {
    if (type == e.type) {
      elementCount = e.elementCount;
      break;
    }
  }
  if (elementCount < 0) {
    std::cerr << "[エラー]" << __func__ << ": " << type << "は頂点属性に設定できません.\n";
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
* コンストラクタ
*
* @param maxBufferSize メッシュ格納用バッファの最大バイト数
*/
GltfFileBuffer::GltfFileBuffer(size_t maxBufferSize)
{
  // ダミーデータ
  static const struct {
    glm::vec4 color = glm::vec4(1);
    glm::vec4 texcoord = glm::vec4(0);
    glm::vec4 normal = glm::vec4(0, 0, -1, 0);
  } dummyData;
  const GLsizei dummyDataSize = static_cast<GLsizei>(sizeof(dummyData));

  this->maxBufferSize = static_cast<GLsizei>(maxBufferSize + dummyDataSize);
  buffer = GLContext::CreateBuffer(this->maxBufferSize, nullptr);

  // バッファの先頭にダミーデータを設定
  CopyData(buffer, 1, 0, sizeof(dummyData), &dummyData);
  curBufferSize = dummyDataSize;
}

/**
* デストラクタ
*/
GltfFileBuffer::~GltfFileBuffer()
{
  glDeleteBuffers(1, &buffer);
}

/**
* ファイルからメッシュを読み込む
*
* @param filename glTFファイル名
*/
bool GltfFileBuffer::AddFromFile(const char* filename)
{
  // glTFファイルを読み込む
  std::vector<char> buf = ReadFile(filename);
  if (buf.empty()) {
    return false;
  }
  buf.push_back('\0');

  // json解析
  std::string err;
  const Json gltf = Json::parse(buf.data(), err);
  if (!err.empty()) {
    std::cerr << "[エラー]" << __func__ << " '" << filename << "'の読み込みに失敗しました.\n";
    std::cerr << err << "\n";
    return false;
  }

  // フォルダ名を取り出す
  std::string foldername(filename);
  const size_t lastSlashPos = foldername.find_last_of("/\\");
  if (lastSlashPos == std::string::npos) {
    foldername.clear();
  } else {
    foldername.resize(lastSlashPos + 1);
  }

  // バイナリファイルを読み込む
  const GLsizei prevBufferSize = curBufferSize;
  std::vector<GLsizeiptr> binOffset;
  for (const Json& e : gltf["buffers"].array_items()) {
    const Json& uri = e["uri"];
    if (!uri.is_string()) {
      std::cerr << "[エラー]" << __func__ << ": " << filename << "に不正なuriがあります.\n";
      return false;
    }
    const std::string binPath = foldername + uri.string_value();
    const std::vector<char> bin = ReadFile(binPath.c_str());
    if (bin.empty()) {
      curBufferSize = prevBufferSize;
      return false;
    }

    // バイナリデータをGPUメモリにコピー
    CopyData(buffer, 1, curBufferSize, bin.size(), bin.data());
    binOffset.push_back(curBufferSize); // バイナリデータのオフセットを設定
    curBufferSize += static_cast<GLsizei>(bin.size());
  }

  // アクセッサからデータを取得してGPUへ転送
  const Json& accessors = gltf["accessors"];
  const Json& bufferViews = gltf["bufferViews"];

  // インデックスデータと頂点属性データのアクセッサIDを取得
  GltfFilePtr file = std::make_shared<GltfFile>();
  const std::vector<Json>& mesheArray = gltf["meshes"].array_items();
  file->meshes.reserve(mesheArray.size());
  for (const Json& currentMesh : mesheArray) {
    GltfMesh mesh;

    // メッシュ名を取得
    mesh.name = currentMesh["name"].string_value();

    // プリミティブを作成
    const std::vector<Json>& primitiveArray = currentMesh["primitives"].array_items();
    mesh.primitives.reserve(primitiveArray.size());
    for (const Json& currentPrim : primitiveArray) {
      GltfPrimitive prim;

      // 頂点インデックス
      {
        const int id = currentPrim["indices"].int_value();
        const Json& accessor = accessors[id];
        if (accessor["type"].string_value() != "SCALAR") {
          std::cerr << "[エラー]" << __func__ << "インデックスデータ・タイプはSCALARでなくてはなりません\n";
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

      // 頂点属性
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

      // 作成したプリミティブを配列に追加
      mesh.primitives.push_back(prim);
    }

    // 作成したメッシュを配列に追加
    file->meshes.push_back(mesh);
  }

  // マテリアル
  GameEngine& engine = GameEngine::Get();
  const std::vector<Json>& materials = gltf["materials"].array_items();
  const std::vector<Json>& textures = gltf["textures"].array_items();
  const std::vector<Json>& images = gltf["images"].array_items();
  file->materials.reserve(materials.size());
  for (const Json& material : materials) {
    const Json& pbr = material["pbrMetallicRoughness"];

    // テクスチャを読み込む
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

    // マテリアルカラーを取得
    glm::vec4 baseColor(1);
    const std::vector<Json>& baseColorFactor = pbr["baseColorFactor"].array_items();
    if (baseColorFactor.size() >= 4) {
      for (int i = 0; i < 4; ++i) {
        baseColor[i] = static_cast<float>(baseColorFactor[i].number_value());
      }
    }

    // 取得したデータからマテリアルを作成
    file->materials.push_back({ baseColor, texBaseColor });
  }

  // 作成したメッシュを連想配列に追加
  file->name = filename;
  files.emplace(filename, file);

  // 読み込んだメッシュ名をデバッグ情報として出力
  std::cout << "[情報]" << __func__ << ": '" << filename << "'を読み込みました\n";
  for (size_t i = 0; i < file->meshes.size(); ++i) {
    std::cout << "  [" << i << "] " << file->meshes[i].name << "\n";
  }

  return true;
}

/**
* ファイルを取得する
*/
GltfFilePtr GltfFileBuffer::GetFile(const char* filename) const
{
  const auto itr = files.find(filename);
  if (itr == files.end()) {
    return nullptr;
  }
  return itr->second;
}

