/**
* @file Light.cpp
*/
#include "Light.h"
#include "Actor.h"
#include <iostream>

namespace Light {

const glm::uint screenSizeX = 1280; // 画面の幅.
const glm::uint screenSizeY = 720; // 画面の高さ.
const glm::uint tileCountX = 8; // 視錐台のX方向の分割数.
const glm::uint tileCountY = 4; // 視錐台のY方向の分割数.

// 分割区画の幅.
const float tileSizeX =
  (float(screenSizeX) / float(tileCountX));

// 分割区画の高さ.
const float tileSizeY =
  (float(screenSizeY) / float(tileCountY));

const glm::uint maxLightCountInTile = 64; // 区画に含まれるライトの最大数.
const glm::uint maxLightCount = 1024; // シーン全体で使えるライトの最大数.

/**
* 表示するライトを選別するためのサブ視錐台.
*/
struct SubFrustum
{
  Plane planes[4]; // left, right, top, bottom
};

/**
* 表示するライトを選別するための視錐台.
*/
struct Frustum
{
  SubFrustum baseFrustum;
  float zNear;
  float zFar;
  SubFrustum tiles[tileCountY][tileCountX];
};

/**
* シェーダと同じ形式のライトデータ.
*/
struct LightForShader
{
  glm::vec4 position;
  glm::vec4 colorAndRange;
};

/**
* 描画に関係するライトの情報.
*/
struct LightData
{
  glm::uint lightIndices[tileCountY][tileCountX][maxLightCountInTile];
  glm::uint lightCounts[tileCountY][tileCountX];
  LightForShader lights[maxLightCount];
};

/**
* スクリーン座標をビュー座標に変換する.
*
* @param p          スクリーン座標.
* @param matInvProj 逆プロジェクション行列.
*
* @return ビュー座標系に変換したpの座標.
*/
glm::vec3 ScreenToView(const glm::vec3& p, const glm::mat4& matInvProj)
{
  glm::vec4 pp;
  pp.x = (p.x / screenSizeX) * 2 - 1;
  pp.y = (p.y / screenSizeY) * 2 - 1;
  pp.z = p.z;
  pp.w = 1;
  pp = matInvProj * pp;
  pp /= pp.w;
  return pp;
}

/**
* ビュー座標系のサブ視錐台を作成する.
*
* @param matProj プロジェクション行列.
* @param x0      タイルの左下のX座標. 
* @param y0      タイルの左下のY座標. 
* @param x1      タイルの右上のX座標. 
* @param y1      タイルの右上のY座標. 
*
* @return 作成したサブ視錐台.
*/
SubFrustum CreateSubFrustum(const glm::mat4& matInvProj,
  float x0, float y0, float x1, float y1)
{
  const glm::vec3 viewPos0 = ScreenToView(glm::vec3(x0, y0, 1), matInvProj);
  const glm::vec3 viewPos1 = ScreenToView(glm::vec3(x1, y0, 1), matInvProj);
  const glm::vec3 viewPos2 = ScreenToView(glm::vec3(x1, y1, 1), matInvProj);
  const glm::vec3 viewPos3 = ScreenToView(glm::vec3(x0, y1, 1), matInvProj);
  const glm::vec3 p = glm::vec3(0);

  return SubFrustum{
    Plane{ p, glm::normalize(glm::cross(viewPos0, viewPos3)) },
    Plane{ p, glm::normalize(glm::cross(viewPos2, viewPos1)) },
    Plane{ p, glm::normalize(glm::cross(viewPos3, viewPos2)) },
    Plane{ p, glm::normalize(glm::cross(viewPos1, viewPos0)) },
  };
}

/**
* ビュー座標系の視錐台を作成する.
*
* @param matProj プロジェクション行列.
* @param zNear   視点から近クリップ平面までの距離.
* @param zFar    視点から遠クリップ平面までの距離.
*
* @return 作成した視錐台.
*/
FrustumPtr CreateFrustum(const glm::mat4& matProj, float zNear, float zFar)
{
  FrustumPtr frustum = std::make_shared<Frustum>();

  const glm::mat4& matInvProj = glm::inverse(matProj);
  frustum->baseFrustum = CreateSubFrustum(matInvProj, 0, 0, screenSizeX, screenSizeY);
  frustum->zNear = -zNear;
  frustum->zFar = -zFar;

  static const glm::vec2 tileSize =
    glm::vec2(screenSizeX, screenSizeY) / glm::vec2(tileCountX, tileCountY);
  for (int y = 0; y < tileCountY; ++y) {
    for (int x = 0; x < tileCountX; ++x) {
      const float x0 = static_cast<float>(x) * tileSize.x;
      const float x1 = x0 + tileSize.x;
      const float y0 = static_cast<float>(y) * tileSize.y;
      const float y1 = y0 + tileSize.y;
      frustum->tiles[y][x] = CreateSubFrustum(matInvProj, x0, y0, x1, y1);
    }
  }
  return frustum;
}

/**
* 球とサブ視錐台の交差判定.
*
* @param sphere  球.
* @param frustum サブ視錐台.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool SphereInsideSubFrustum(const Sphere& sphere, const SubFrustum& frustum)
{
  for (const auto& plane : frustum.planes) {
    if (!SphereInsidePlane(sphere, plane)) {
      return false;
    }
  }
  return true;
}

/**
* 球と視錐台の交差判定.
*
* @param sphere  球.
* @param frustum 視錐台.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool SphereInsideFrustum(const Sphere& sphere, const Frustum& frustum)
{
  if (sphere.center.z - sphere.radius > frustum.zNear) {
    return false;
  }
  if (sphere.center.z + sphere.radius < frustum.zFar) {
    return false;
  }
  return SphereInsideSubFrustum(sphere, frustum.baseFrustum);
}

/**
* 円錐とサブ視錐台の交差判定.
*
* @param cone    円錐.
* @param frustum サブ視錐台.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool ConeInsideSubFrustum(const Cone& cone, const SubFrustum& frustum)
{
  for (const auto& plane : frustum.planes) {
    if (!ConeInsidePlane(cone, plane)) {
      return false;
    }
  }
  return true;
}

/**
* 円錐と視錐台の交差判定.
*
* @param cone    円錐.
* @param frustum 視錐台.
*
* @retval true  衝突している.
* @retval false 衝突していない.
*/
bool ConeInsideFrustum(const Cone& cone, const Frustum& frustum)
{
  if (!ConeInsidePlane(cone, Plane{
    glm::vec3(0, 0, frustum.zNear), glm::vec3(0, 0, -1) })) {
    return false;
  }
  if (!ConeInsidePlane(cone, Plane{
    glm::vec3(0, 0, frustum.zFar), glm::vec3(0, 0, 1) })) {
    return false;
  }
  return ConeInsideSubFrustum(cone, frustum.baseFrustum);
}

/**
* コンストラクタ.
*/
LightManager::LightManager()
{
  lights.reserve(1024);
  ssbo[0] = std::make_shared<ShaderStorageBufferObject>(sizeof(LightData));
  ssbo[1] = std::make_shared<ShaderStorageBufferObject>(sizeof(LightData));
}

/**
* ライトを作成する.
*
* @param position ライトの座標.
* @param color    ライトの明るさ.
*
* @return 作成したライトへのポインタ.
*/
LightPtr LightManager::CreateLight(const glm::vec3& position, const glm::vec3& color)
{
  LightPtr p = std::make_shared<Light>();
  p->position = position;
  p->color = color;
  lights.push_back(p);
  return p;
}

/**
* ライトを削除する.
*
* @param light 削除するライトへのポインタ.
*/
void LightManager::RemoveLight(const LightPtr& light)
{
  auto itr = std::find(lights.begin(), lights.end(), light);
  if (itr != lights.end()) {
    lights.erase(itr);
  }
}

/**
* ライトを取得する.
*
* @param n ライトの番号.
*
* @return n番目のライトへのポインタ.
*         ライトの数がn個未満の場合はnullptrを返す.
*/
LightPtr LightManager::GetLight(size_t n) const
{
  if (n >= lights.size()) {
    return nullptr;
  }
  return lights[n];
}

/**
* ライトの数を取得する.
*
* @return ライトの数.
*/
size_t LightManager::LightCount() const
{
  return lights.size();
}

/**
* GPUに転送するライトデータを更新する.
*
* @param matView 描画に使用するビュー行列.
* @param frustum 視錐台.
*/
void LightManager::Update(const glm::mat4& matView, const FrustumPtr& frustum)
{
  std::shared_ptr<LightData> tileData = std::make_unique<LightData>();
  std::vector<glm::vec3> posView;
  posView.reserve(lights.size());

  // メインフラスタムと交差しているライトだけをライトリストに登録.
  for (glm::uint i = 0; i < lights.size(); ++i) {
    const LightPtr& e = lights[i];
    const glm::vec3 pos = matView * glm::vec4(e->position, 1);
    const float range = sqrt(glm::max(e->color.r, glm::max(e->color.g, e->color.b))) * 3;
    if (SphereInsideFrustum(Sphere{ pos, range }, *frustum)) {
      tileData->lights[posView.size()] = LightForShader{
        glm::vec4(e->position, 1), glm::vec4(e->color, range) };
      posView.push_back(pos);
      if (posView.size() >= maxLightCount) {
        break;
      }
    }
  }

  // すべてのサブフラスタムについて、登録されたライトとの交差判定を行う.
  for (int y = 0; y < tileCountY; ++y) {
    for (int x = 0; x < tileCountX; ++x) {
      const SubFrustum& f = frustum->tiles[y][x];
      glm::uint count = 0;
      for (glm::uint i = 0; i < posView.size(); ++i) {
        if (SphereInsideSubFrustum(Sphere{ posView[i],
          tileData->lights[i].colorAndRange.a }, f)) {
          tileData->lightIndices[y][x][count] = i;
          ++count;
          if (count >= maxLightCountInTile) {
            std::cerr << "[情報]" << __func__ <<
              "サイズオーバー[" << y << "][" << x << "]\n";
            break;
          }
        }
      }
      tileData->lightCounts[y][x] = count;
    }
  }

  // LightDataをGPUメモリに転送.
  auto targetSsbo = ssbo[writingSsboNo];
  const size_t size = sizeof(LightData::lightCounts) +
    sizeof(LightData::lightIndices) + sizeof(LightForShader) * posView.size();
  targetSsbo->CopyData(tileData.get(), size, 0);
  writingSsboNo ^= 1;
}

/**
* SSBOをグラフィックスパイプラインに割り当てる.
*
* @param location 割り当て先のロケーション番号.
*/
void LightManager::Bind(GLuint location) const
{
  ssbo[writingSsboNo ^ 1]->Bind(location);
}

/**
* SSBOのグラフィックスパイプラインへの割り当てを解除する.
*
* @param location 割り当て先のロケーション番号.
*/
void LightManager::Unbind(GLuint location) const
{
  ssbo[writingSsboNo ^ 1]->Unbind(location);
}

} // namespace Light

