/**
* @file Light.h
*/
#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED
#include "glad/glad.h"
#include "ShaderStorageBufferObject.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>

namespace Light {

// 先行宣言.
struct Frustum;
using FrustumPtr = std::shared_ptr<Frustum>;

FrustumPtr CreateFrustum(const glm::mat4& matProj, float zNear, float zFar);

/**
* 点光源.
*/
struct Light
{
  // ライトの種類.
  enum class Type {
    PointLight,       // 点光源.
  };

  std::string name;   // ライトの名前.
  Type type = Type::PointLight; // ライトの種類.
  glm::vec3 position; // 光を放つ位置.
  glm::vec3 color;    // ライトの色.
};
using LightPtr = std::shared_ptr<Light>;

/**
* ライトを管理するクラス.
*/
class LightManager
{
public:
  LightManager();
  ~LightManager() = default;
  LightManager(const LightManager&) = delete;
  LightManager& operator=(const LightManager&) = delete;

  LightPtr CreateLight(const glm::vec3& position, const glm::vec3& color);
  void RemoveLight(const LightPtr& light);
  LightPtr GetLight(size_t n) const;
  size_t LightCount() const;

  void Update(const glm::mat4& matView, const FrustumPtr& frustum);
  void Bind(GLuint binding) const;
  void Unbind(GLuint binding) const;

private:
  std::vector<LightPtr> lights;
  std::shared_ptr<ShaderStorageBufferObject> ssbo[2];
  int writingSsboNo = 0;
};

// ライトマネージャクラスのポインタ型.
using LightManagerPtr = std::shared_ptr<LightManager>;

} // namespace Light

#endif//LIGHT_H_INCLUDED
