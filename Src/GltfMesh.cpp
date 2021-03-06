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
#include <algorithm>

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
* バイナリデータ型
*/
struct BinaryData
{
  GLsizeiptr offset;
  std::vector<char> bin;
};
using BinaryList = std::vector<BinaryData>;

/**
* アクセッサが示すバッファのオフセットを取得する
*
* @param accessor    アクセッサ
* @param bufferViews バッファビュー配列
* @param binaryList  バイナリデータ配列
*/
GLsizeiptr GetBufferOffset(const Json& accessor,
  const Json& bufferViews, const BinaryList& binaryList)
{
  // アクセッサから必要な情報を取得
  const int byteOffset = accessor["byteOffset"].int_value();
  const int bufferViewId = accessor["bufferView"].int_value();

  // バッファビューから必要な情報を取得
  const Json bufferView = bufferViews[bufferViewId];
  const int bufferId = bufferView["buffer"].int_value();
  const int baseByteOffset = bufferView["byteOffset"].int_value();

  // オフセットを計算
  return binaryList[bufferId].offset + baseByteOffset + byteOffset;
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
* アクセッサが示すバッファ内のアドレスを取得する
*
* @param accessor    アクセッサ
* @param bufferViews バッファビュー配列
* @param binaryList  バイナリデータ配列
*
* @return バッファ内のアドレス
*/
const void* GetBuffer(const Json& accessor, const Json& bufferViews, const BinaryList& binaryList)
{
  // アクセッサから必要な情報を取得
  const int byteOffset = accessor["byteOffset"].int_value();
  const int bufferViewId = accessor["bufferView"].int_value();

  // バッファビューから必要な情報を取得
  const Json& bufferView = bufferViews[bufferViewId];
  const int bufferId = bufferView["buffer"].int_value();
  const int baseByteOffset = bufferView["byteOffset"].int_value();

  // アクセッサが参照するデータの先頭を計算
  return binaryList[bufferId].bin.data() + baseByteOffset + byteOffset;
}

/**
* 頂点アトリビュートを設定する
*
* @param vao         頂点アトリビュートを設定するVAO
* @param index       頂点アトリビュート番号及びバインディングポイント
* @param buffer      VBOとして扱うバッファオブジェクト
* @param accessor    頂点アトリビュートデータを持つアクセッサ
* @param bufferViews バッファビュー配列
* @param binaryList  バイナリデータ配列
*/
bool SetAttribute(VertexArrayObjectPtr& vao, int index, GLuint buffer,
  const Json& accessor, const Json& bufferViews,
  const BinaryList& binaryList)
{
  // 型名と要素数の対応表(頂点アトリビュート用)
  const struct {
    const char* type; // 型の名前
    int elementCount; // 要素数
  } elementCountList[] = {
    { "SCALAR", 1 }, { "VEC2", 2 }, { "VEC3", 3 }, { "VEC4", 4 },
  };

  // 対応表から要素数を取得
  const std::string& type = accessor["type"].string_value();
  int elementCount = -1;
  for (const auto& e : elementCountList) {
    if (type == e.type) {
      elementCount = e.elementCount;
      break;
    }
  }
  if (elementCount < 0) {
    std::cerr << "[エラー]" << __func__ << ": " << type <<
      "は頂点属性に設定できません.\n";
    return false;
  }

  // VAOに頂点アトリビュートを設定する
  const GLsizei byteStride = GetBufferStride(accessor, bufferViews);
  const GLsizeiptr offset = GetBufferOffset(accessor, bufferViews, binaryList);
  const GLenum componentType = accessor["componentType"].int_value();
  vao->SetAttribute(index, index, elementCount, componentType, GL_FALSE, 0);
  vao->SetVBO(index, buffer, offset, byteStride);

  return true;
}

/**
* 頂点アトリビュートのデフォルト値を設定する
*
* @param vao          頂点アトリビュートを設定するVAO
* @param index        頂点アトリビュート番号及びバインディングポイント
* @param buffer       VBOとして扱うバッファオブジェクト
* @param elementCount 要素数
* @param offset       buffer内のデータが配置位置(単位=バイト)
*
* シェーダが必要とする頂点アトリビュートについて、プリミティブが対応する頂点データを
* 持たない場合、この関数によってデフォルト値を設定する。
*/
void SetDefaultAttribute(VertexArrayObjectPtr& vao, int index, GLuint buffer,
  GLint elementCount, GLsizeiptr offset)
{
  vao->SetAttribute(index, index, elementCount, GL_FLOAT, GL_FALSE, 0);
  vao->SetVBO(index, buffer, offset, 0);
}

/**
* JSONの配列データをglm::vec3に変換する
*
* @param json 変換元となる配列データ
*
* @return jsonを変換してできたvec3の値
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
* JSONの配列データをglm::quatに変換する
*
* @param json 変換元となる配列データ
*
* @return jsonを変換してできたquatの値
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
* JSONの配列データをglm::mat4に変換する
*
* @param json 変換元となる配列データ
*
* @return jsonを変換してできたmat4の値
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
* ノードのローカル姿勢行列を計算する
*
* @param node gltfノード
*
* @return nodeのローカル姿勢行列
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
* メッシュを持つノードをリストアップする
*/
void GetMeshNodeList(const GltfNode* node, std::vector<const GltfNode*>& list)
{
  if (node->mesh >= 0) {
    list.push_back(node);
  }
  for (const GltfNode* child : node->children) {
    GetMeshNodeList(child, list);
  }
}

/**
* アニメーションチャネルを作成する
*
* @param pTimes       時刻の配列のアドレス
* @param pValues      値の配列のアドレス
* @param inputCount   配列の要素数
* @param targetNodeId 値の適用対象となるノードID
* @param interp       補間方法
* @param totalTime    総再生時間を格納する変数のアドレス
*
* @return 作成したアニメーションチャネル
*/
template<typename T>
GltfChannel<T> MakeAnimationChannel(
  const GLfloat* pTimes, const void* pValues, size_t inputCount,
  int targetNodeId, GltfInterpolation interp, float* totalTime)
{
  // 時刻と値の配列からキーフレーム配列を作成
  const T* pData = static_cast<const T*>(pValues);
  GltfChannel<T> channel;
  channel.keyframes.resize(inputCount);
  for (int i = 0; i < inputCount; ++i) {
    *totalTime = std::max(*totalTime, pTimes[i]);
    channel.keyframes[i] = { pTimes[i], pData[i] };
  }

  // 適用対象ノードIDと補間方法を設定
  channel.targetNodeId = targetNodeId;
  channel.interpolation = interp;

  return channel; // 作成したチャネルを返す
}

/**
* チャネル上の指定した時刻の値を求める
*
* @param channel 対象のチャネル
* @param time    値を求める時刻
*
* @return 時刻に対応する値
*/
template<typename T>
T Interpolate(const GltfChannel<T>& channel, float time)
{
  // time以上の時刻を持つ、最初のキーフレームを検索
  const auto curOrOver = std::lower_bound(
    channel.keyframes.begin(), channel.keyframes.end(), time,
    [](const GltfKeyframe<T>& keyFrame, float time) {
      return keyFrame.time < time; });

  // timeが先頭キーフレームの時刻と等しい場合、先頭キーフレームの値を返す
  if (curOrOver == channel.keyframes.begin()) {
    return channel.keyframes.front().value;
  }

  // timeが末尾キーフレームの時刻より大きい場合、末尾キーフレームの値を返す
  if (curOrOver == channel.keyframes.end()) {
    return channel.keyframes.back().value;
  }

  // timeが先頭と末尾の間だった場合...

  // キーフレーム間の時間におけるtimeの比率を計算
  const auto prev = curOrOver - 1; // time未満の時刻を持つキーフレーム
  const float frameTime = curOrOver->time - prev->time;
  const float ratio = glm::clamp((time - prev->time) / frameTime, 0.0f, 1.0f);

  // 比率によって補間した値を返す
  return glm::mix(prev->value, curOrOver->value, ratio);
}

/**
* アニメーション計算用の中間データ型
*/
struct NodeMatrix
{
  glm::mat4 m = glm::mat4(1); // 姿勢行列
  bool isCalculated = false;  // 計算済みフラグ
};
using NodeMatrices = std::vector<NodeMatrix>;

/**
* ノードのグローバル姿勢行列を計算する
*/
const glm::mat4& CalcGlobalNodeMatrix(const std::vector<GltfNode>& nodes,
  const GltfNode& node, NodeMatrices& matrices)
{
  const intptr_t currentNodeId = &node - &nodes[0];
  NodeMatrix& nodeMatrix = matrices[currentNodeId];

  // 「計算済み」の場合は自分の姿勢行列を返す
  if (nodeMatrix.isCalculated) {
    return nodeMatrix.m;
  }

  // 「計算済みでない」場合、親の姿勢行列を合成する
  if (node.parent) {
    // 親の行列を取得(再帰呼び出し)
    const glm::mat4& matParent =
      CalcGlobalNodeMatrix(nodes, *node.parent, matrices);

    // 親の姿勢行列を合成
    nodeMatrix.m = matParent * nodeMatrix.m;
  }

  // 「計算済み」にする
  nodeMatrix.isCalculated = true;

  // 自分の姿勢行列を返す
  return nodeMatrix.m;
}

// バインディングポイント
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
* デフォルト頂点データ
*
* 頂点データに対応する要素がない場合に使う汎用データ
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
* アニメーションを適用した姿勢行列を計算する
*
* @param file             meshNodeを所有するファイルオブジェクト
* @param meshNode         メッシュを持つノード
* @param animation        計算の元になるアニメーション
* @param nonAnimatedNodes アニメーションしないノードIDの配列
* @param time             アニメーションの再生位置
*
* @return アニメーションを適用した姿勢行列の配列
*/
GltfAnimationMatrices CalcAnimationMatrices(const GltfFilePtr& file,
  const GltfNode* meshNode, const GltfAnimation* animation,
  const std::vector<int>& nonAnimatedNodes, float time)
{
  GltfAnimationMatrices matBones;
  if (!file || !meshNode) {
    return matBones;
  }

  // アニメーションが設定されていない場合
  if (!animation) {
    size_t size = 1;
    if (meshNode->skin >= 0) {
      size = file->skins[meshNode->skin].joints.size();
    }
    matBones.resize(size, meshNode->matGlobal);
    return matBones;
  }

  // アニメーションが設定されている場合...

  NodeMatrices matrices;
  const auto& nodes = file->nodes;
  matrices.resize(nodes.size());

  // アニメーションしないノードのローカル姿勢行列を設定
  for (const auto e : nonAnimatedNodes) {
    matrices[e].m = nodes[e].matLocal;
  }

  // アニメーションするノードのローカル姿勢行列を計算
  for (const auto& e : animation->translations) {
    NodeMatrix& nodeMatrix = matrices[e.targetNodeId];
    const glm::vec3 translation = Interpolate(e, time);
    nodeMatrix.m *= glm::translate(glm::mat4(1), translation);
  }
  for (const auto& e : animation->rotations) {
    NodeMatrix& nodeMatrix = matrices[e.targetNodeId];
    const glm::quat rotation = Interpolate(e, time);
    nodeMatrix.m *= glm::mat4_cast(rotation);
  }
  for (const auto& e : animation->scales) {
    NodeMatrix& nodeMatrix = matrices[e.targetNodeId];
    const glm::vec3 scale = Interpolate(e, time);
    nodeMatrix.m *= glm::scale(glm::mat4(1), scale);
  }

  // アニメーションを適用したグローバル姿勢行列を計算
  if (meshNode->skin >= 0) {
    for (const auto& joint : file->skins[meshNode->skin].joints) {
      CalcGlobalNodeMatrix(nodes, nodes[joint.nodeId], matrices);
    }
  } else {
    // ジョイントがないのでメッシュノードだけ計算
    CalcGlobalNodeMatrix(nodes, *meshNode, matrices);
  }

  // 逆バインドポーズ行列を合成
  if (meshNode->skin >= 0) {
    // jointsにはノード番号が格納されているが、頂点データのJOINTS_nには
    // ノード番号ではなく「joints配列のインデックス」が格納されている。
    // つまり、姿勢行列をjoints配列の順番でSSBOに格納する必要がある。
    const auto& joints = file->skins[meshNode->skin].joints;
    matBones.resize(joints.size());
    for (size_t i = 0; i < joints.size(); ++i) {
      const auto& joint = joints[i];
      matBones[i] = matrices[joint.nodeId].m * joint.matInverseBindPose;
    }
  } else {
    // ジョイントがないので逆バインドポーズ行列も存在しない
    const size_t nodeId = meshNode - &nodes[0];
    matBones.resize(1, matrices[nodeId].m);
  }

  return matBones;
}

/**
* コンストラクタ
*
* @param maxBufferSize メッシュ格納用バッファの最大バイト数
* @param maxMatrixSize アニメーション用SSBOに格納できる最大行列数
*/
GltfFileBuffer::GltfFileBuffer(size_t maxBufferSize, size_t maxMatrixSize)
{
  const GLsizei defaultDataSize = static_cast<GLsizei>(sizeof(DefaultVertexData));

  this->maxBufferSize = static_cast<GLsizei>(maxBufferSize + defaultDataSize);
  buffer = GLContext::CreateBuffer(this->maxBufferSize, nullptr);

  // バッファの先頭にダミーデータを設定
  const DefaultVertexData defaultData;
  CopyData(buffer, 1, 0, defaultDataSize, &defaultData);
  curBufferSize = defaultDataSize;

  // アニメーションメッシュ用のバッファを作成
  ssbo = std::make_shared<ShaderStorageBuffer>(maxMatrixSize * sizeof(glm::mat4));
  matrixBuffer.reserve(maxMatrixSize);
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
    std::cerr << "[エラー]" << __func__ << " '" << filename <<
      "'の読み込みに失敗しました.\n";
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
  const std::vector<Json>& buffers = gltf["buffers"].array_items();
  BinaryList binaryList(buffers.size());
  for (size_t i = 0; i < buffers.size(); ++i) {
    const Json& uri = buffers[i]["uri"];
    if (!uri.is_string()) {
      std::cerr << "[エラー]" << __func__ << ": " << filename <<
        "に不正なuriがあります.\n";
      return false;
    }
    const std::string binPath = foldername + uri.string_value();
    binaryList[i].bin = ReadFile(binPath.c_str());
    if (binaryList[i].bin.empty()) {
      curBufferSize = prevBufferSize;
      return false;
    }

    // バイナリデータをGPUメモリにコピー
    CopyData(buffer, 1, curBufferSize, binaryList[i].bin.size(), binaryList[i].bin.data());

    // オフセットを更新
    binaryList[i].offset = curBufferSize;
    curBufferSize += static_cast<GLsizei>(binaryList[i].bin.size());
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

      // インデックスデータ
      {
        const int id = currentPrim["indices"].int_value();
        const Json& accessor = accessors[id];
        const std::string type = accessor["type"].string_value();
        if (type != "SCALAR") {
          std::cerr << "[エラー]" << __func__ << "インデックスデータ・タイプはSCALARでなくてはなりません\n";
          std::cerr << "  type = " << type << "\n";
          return false;
        }

        // プリミティブの種類
        const Json& mode = currentPrim["mode"];
        if (mode.is_number()) {
          prim.mode = mode.int_value();
        }

        // インデックス数
        prim.count = accessor["count"].int_value();

        // インデックスデータの型
        prim.type = accessor["componentType"].int_value();

        // オフセット
        const GLsizeiptr offset = GetBufferOffset(accessor, bufferViews, binaryList);
        prim.indices = reinterpret_cast<const GLvoid*>(offset);
      }

      // 頂点アトリビュート(頂点座標)
      prim.vao = std::make_shared<VertexArrayObject>();
      prim.vao->SetIBO(buffer);
      const Json& attributes = currentPrim["attributes"];
      if (attributes["POSITION"].is_number()) {
        const int id = attributes["POSITION"].int_value();
        SetAttribute(prim.vao, bpPosition, buffer,
          accessors[id], bufferViews, binaryList);
      }

      // 頂点アトリビュート(頂点の色)
      if (attributes["COLOR"].is_number()) {
        const int id = attributes["COLOR"].int_value();
        SetAttribute(prim.vao, bpColor, buffer,
          accessors[id], bufferViews, binaryList);
      } else {
        SetDefaultAttribute(prim.vao, bpColor, buffer,
          4, offsetof(DefaultVertexData, color));
      }

      // 頂点アトリビュート(テクスチャ座標)
      if (attributes["TEXCOORD_0"].is_number()) {
        const int id = attributes["TEXCOORD_0"].int_value();
        SetAttribute(prim.vao, bpTexcoord0, buffer,
          accessors[id], bufferViews, binaryList);
      } else {
        SetDefaultAttribute(prim.vao, bpTexcoord0, buffer,
          2, offsetof(DefaultVertexData, texcoord));
      }

      // 頂点アトリビュート(法線)
      if (attributes["NORMAL"].is_number()) {
        const int id = attributes["NORMAL"].int_value();
        SetAttribute(prim.vao, bpNormal, buffer,
          accessors[id], bufferViews, binaryList);
      } else {
        SetDefaultAttribute(prim.vao, bpNormal, buffer,
          3, offsetof(DefaultVertexData, normal));
      }

      // 頂点アトリビュート(ジョイント)
      if (attributes["JOINTS_0"].is_number()) {
        const int id = attributes["JOINTS_0"].int_value();
        SetAttribute(prim.vao, bpJoints0, buffer,
          accessors[id], bufferViews, binaryList);
      } else {
        SetDefaultAttribute(prim.vao, bpJoints0, buffer,
          4, offsetof(DefaultVertexData, joints));
      }

      // 頂点アトリビュート(ウェイト)
      if (attributes["WEIGHTS_0"].is_number()) {
        const int id = attributes["WEIGHTS_0"].int_value();
        SetAttribute(prim.vao, bpWeights0, buffer,
          accessors[id], bufferViews, binaryList);
      } else {
        SetDefaultAttribute(prim.vao, bpWeights0, buffer,
          4, offsetof(DefaultVertexData, weights));
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

  // ノード
  {
    const std::vector<Json>& nodes = gltf["nodes"].array_items();
    file->nodes.resize(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) {
      const Json& node = nodes[i];
      GltfNode& n = file->nodes[i];

      // ノード名を設定
      n.name = node["name"].string_value();

      // メッシュIDを取得
      const Json& meshId = node["mesh"];
      if (meshId.is_number()) {
        n.mesh = meshId.int_value();
      }

      // スキンIDを取得
      const Json& skinId = node["skin"];
      if (skinId.is_number()) {
        n.skin = skinId.int_value();
      }

      // 自分の子ノードに対して親ノードポインタを設定
      // NOTE: ポインタよりインデックスのほうが安全かつメモリ効率がいいかも？
      const std::vector<Json>& children = node["children"].array_items();
      n.children.reserve(children.size());
      for (const Json& e : children) {
        GltfNode& childNode = file->nodes[e.int_value()];
        n.children.push_back(&childNode);
        if (!childNode.parent) {
          childNode.parent = &n;
        }
      }

      // ローカル座標変換行列を計算
      n.matLocal = CalcLocalMatrix(node);
    }

    // すべてのノードの親子関係とローカル座標行列を設定したので、
    // 親をたどってグローバル座標変換行列を計算する
    for (GltfNode& e : file->nodes) {
      e.matGlobal = e.matLocal;
      const GltfNode* parent = e.parent;
      while (parent) {
        e.matGlobal = parent->matLocal * e.matGlobal;
        parent = parent->parent;
      }
    }
  } // ノード

  // シーン
  {
    // シーンに含まれるメッシュノードを取得
    // @note メッシュノードだけを描画すればよいので、scenesのnodes配列に
    //       対応するノードを保持する必要はない。
    //       ノード行列の更新は親ノードを辿ることで実現できる。
    const std::vector<Json>& scenes = gltf["scenes"].array_items();
    file->scenes.resize(scenes.size());
    for (size_t i = 0; i < scenes.size(); ++i) {
      GltfScene& s = file->scenes[i];
      const std::vector<Json>& nodes = scenes[i]["nodes"].array_items();
      s.nodes.resize(nodes.size());
      for (size_t j = 0; j < nodes.size(); ++j) {
        const int nodeId = nodes[j].int_value();
        const GltfNode* n = &file->nodes[nodeId];
        s.nodes[j] = n;
        GetMeshNodeList(n, s.meshNodes);
      }
    }
  } // シーン

  // スキン
  {
    const std::vector<Json>& skins = gltf["skins"].array_items();
    file->skins.resize(skins.size());
    for (size_t i = 0; i < skins.size(); ++i) {
      const Json& skin = skins[i];
      GltfSkin& s = file->skins[i];

      // スキン名を設定
      s.name = skin["name"].string_value();

      // 逆バインドポーズ行列アクセッサの取得
      const int inverseBindMatricesAccessorId = skin["inverseBindMatrices"].int_value();
      const Json& accessor = accessors[inverseBindMatricesAccessorId];
      if (accessor["type"].string_value() != "MAT4") {
        std::cerr << "ERROR: バインドポーズのtypeはMAT4でなくてはなりません \n";
        std::cerr << "  type = " << accessor["type"].string_value() << "\n";
        return false;
      }
      if (accessor["componentType"].int_value() != GL_FLOAT) {
        std::cerr << "ERROR: バインドポーズのcomponentTypeはGL_FLOATでなくてはなりません \n";
        std::cerr << "  type = 0x" << std::hex << accessor["componentType"].string_value() << "\n";
        return false;
      }

      // 逆バインドポーズ行列の配列を取得
      // @note glTFのバッファデータはリトルエンディアン. 仕様に書いてある
      //       実行環境によっては変換の必要あり
      const glm::mat4* inverseBindMatrices =
        static_cast<const glm::mat4*>(GetBuffer(accessor, bufferViews, binaryList));

      // 関節データを取得
      const std::vector<Json>& joints = skin["joints"].array_items();
      s.joints.resize(joints.size());
      for (size_t jointId = 0; jointId < joints.size(); ++jointId) {
        s.joints[jointId].nodeId = joints[jointId].int_value();
        s.joints[jointId].matInverseBindPose = inverseBindMatrices[jointId];
      }
    }
  } // スキン

  // アニメーション
  {
    const std::vector<Json>& animations = gltf["animations"].array_items();
    file->animations.resize(animations.size());
    for (size_t i = 0; i < animations.size(); ++i) {
      const Json& animation = animations[i];
      const std::vector<Json>& channels = animation["channels"].array_items();
      const std::vector<Json>& samplers = animation["samplers"].array_items();

      // 名前を設定
      GltfAnimationPtr a = std::make_shared<GltfAnimation>();
      a->name = animation["name"].string_value();

      // チャネル配列の容量を予約
      const size_t predictedSize = channels.size() / 3 + 1; // 予測サイズ
      a->translations.reserve(predictedSize);
      a->rotations.reserve(predictedSize);
      a->scales.reserve(predictedSize);

      // チャネル配列を設定
      a->totalTime = 0;
      for (const Json& e : channels) {
        const int samplerId = e["sampler"].int_value();
        const Json& sampler = samplers[samplerId];
        const Json& target = e["target"];
        const int targetNodeId = target["node"].int_value();
        if (targetNodeId < 0) {
          continue; // 対象ノードIDが無効
        }

        // 時刻の配列を取得
        const int inputAccessorId = sampler["input"].int_value();
        const Json& inputAccessor = accessors[inputAccessorId];
        const int inputCount = inputAccessor["count"].int_value();
        const GLfloat* pTimes = static_cast<const GLfloat*>(
          GetBuffer(inputAccessor, bufferViews, binaryList));

        // 値の配列を取得
        const int outputAccessorId = sampler["output"].int_value();
        //const int outputCount = accessors[outputAccessorId]["count"].int_value();
        const void* pValues =
          GetBuffer(accessors[outputAccessorId], bufferViews, binaryList);

        // 補間方法を取得
        const std::string& interpolation = target["interpolation"].string_value();
        GltfInterpolation interp = GltfInterpolation::linear;
        if (interpolation != "LINEAR") {
          if (interpolation == "STEP") {
            interp = GltfInterpolation::step;
          } else if (interpolation == "CUBICSPLINE") {
            interp = GltfInterpolation::cubicSpline;
          }
        }

        // 時刻と値の配列からチャネルを作成し、pathに対応する配列に追加
        const std::string& path = target["path"].string_value();
        if (path == "translation") {
          a->translations.push_back(MakeAnimationChannel<glm::vec3>(
            pTimes, pValues, inputCount, targetNodeId, interp, &a->totalTime));
        } else if (path == "rotation") {
          a->rotations.push_back(MakeAnimationChannel<glm::quat>(
            pTimes, pValues, inputCount, targetNodeId, interp, &a->totalTime));
        } else if (path == "scale") {
          a->scales.push_back(MakeAnimationChannel<glm::vec3>(
            pTimes, pValues, inputCount, targetNodeId, interp, &a->totalTime));
        }
      }
      file->animations[i] = a;
    }
  } // アニメーション

  // 作成したメッシュを連想配列に追加
  file->name = filename;
  files.emplace(filename, file);

  // 読み込んだメッシュ名をデバッグ情報として出力
  std::cout << "[情報]" << __func__ << ": '" << filename << "'を読み込みました\n";
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

/**
* アニメーションメッシュの描画用データをすべて削除
*/
void GltfFileBuffer::ClearAnimationBuffer()
{
  matrixBuffer.clear();
}

/**
* アニメーションメッシュの描画用データを追加
*/
GLintptr GltfFileBuffer::AddAnimationMatrices(const GltfAnimationMatrices& matBones)
{
  GLintptr offset = static_cast<GLintptr>(matrixBuffer.size() * sizeof(glm::mat4));
  matrixBuffer.insert(matrixBuffer.end(), matBones.begin(), matBones.end());

  // SSBOのオフセットアライメント条件を満たすために、256バイト境界に配置する。
  // 256はOpenGL仕様で許される最大値(GeForce系がこの値を使っている)。
  // https://www.khronos.org/registry/OpenGL/specs/gl/glspec45.core.pdf
  // の表23.64を参照
  matrixBuffer.resize(((matrixBuffer.size() + 3) / 4) * 4);
  return offset;
}

/**
* アニメーションメッシュの描画用データをGPUメモリにコピー
*/
void GltfFileBuffer::UploadAnimationBuffer()
{
  ssbo->BufferSubData(0, matrixBuffer.size() * sizeof(glm::mat4), matrixBuffer.data());
  ssbo->SwapBuffers();
}

/**
* アニメーションメッシュの描画に使うSSBO領域を割り当てる
*/
void GltfFileBuffer::BindAnimationBuffer(GLuint bindingPoint, GLintptr offset, GLsizeiptr size)
{
  if (size > 0) {
    ssbo->Bind(bindingPoint, offset, size);
  }
}

/**
* アニメーションメッシュの描画に使うSSBO領域の割り当てを解除する
*/
void GltfFileBuffer::UnbindAnimationBuffer(GLuint bindingPoint)
{
  ssbo->Unbind(bindingPoint);
}

