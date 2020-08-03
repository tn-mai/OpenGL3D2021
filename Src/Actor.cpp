/**
* @file Actor.cpp
*/
#include "Actor.h"
#include <glm/gtc/matrix_transform.hpp>

/**
* コンストラクタ.
*
* @param actorName 作成するアクターに付ける名前.
* @param pid       アクターの外見を表すプリミティブのID.
* @param tex       プリミティブに貼り付けるテクスチャ.
* @param pos       アクターの座標.
*/
Actor::Actor(std::string actorName, const Mesh::Primitive* prim,
  std::shared_ptr<Texture::Image2D> tex, const glm::vec3& pos) :
  name(name), primitive(prim), texture(tex), position(pos)
{
}

/**
* アクターの状態を更新する.
*
* @param deltaTime 前回の更新からの経過時間(秒).
*/
void Actor::Update(float deltaTime)
{
  position += velocity * deltaTime;

  // アニメーションを更新.
  if (!animation.empty()) {
    animationTimer += deltaTime;
    if (animationTimer >= animationInterval) {
      animationTimer -= animationInterval;
      ++animationNo;
      if (animationNo >= animation.size()) {
        animationNo = 0;
      }
      primitive = animation[animationNo];
    }
  }
}

/**
* アクターを描画する.
*
* @param pipeline 行列の設定先となるパイプラインオブジェクト.
* @param matVP    描画に使用するビュープロジェクション行列.
*/
void Actor::Draw(const Shader::Pipeline& pipeline, const glm::mat4& matVP, const glm::mat4& matShadow) const
{
  // プリミティブが設定されていないときは何もせず終了.
  if (!primitive) {
    return;
  }

  // 平行移動させる行列を作る.
  const glm::mat4 matTranslate = glm::translate(glm::mat4(1), position);
  // X軸回転させる行列を作る.
  const glm::mat4 matRotateX = glm::rotate(
    glm::mat4(1), rotation.x, glm::vec3(1, 0, 0));
  // Y軸回転させる行列を作る.
  const glm::mat4 matRotateY = glm::rotate(
    glm::mat4(1), rotation.y, glm::vec3(0, 1, 0));
  // Z軸回転させる行列を作る.
  const glm::mat4 matRotateZ = glm::rotate(
    glm::mat4(1), rotation.z, glm::vec3(0, 0, 1));
  // 大きさを変える行列を作る.
  const glm::mat4 matScale = glm::scale(glm::mat4(1), scale);

  // 平行移動・回転・大きさ変更の行列を掛け算して、ひとつのモデル行列にまとめる.
  const glm::mat4 matModel =
    matTranslate * matRotateY * matRotateZ * matRotateX * matScale;

  // GPUメモリに行列を転送.
  pipeline.SetModelMatrix(matModel);
  pipeline.SetMVP(matVP * matModel);

  // テクスチャイメージスロット0番にテクスチャを割り当てる.
  texture->Bind(0);

  // プリミティブを描画.
  primitive->Draw();

    // 影を描画.
  if (hasShadow) {
    // 平行移動・回転・大きさ変更の行列を掛け算して、ひとつのモデル行列にまとめる.
    const glm::mat4 matModelShadow =
      matTranslate * matShadow * matRotateY * matRotateZ * matRotateX * matScale;

    // GPUメモリに影行列を転送.
    pipeline.SetModelMatrix(matModelShadow);
    pipeline.SetMVP(matVP * matModelShadow);

    // プリミティブを描画.
    primitive->Draw();
  }
}

/**
* アクターリストを更新する.
*
* @param actorList  更新するアクターリスト.
* @param deltaTime  前回の更新からの経過時間(秒).
*/
void UpdateActorList(ActorList& actorList, float deltaTime)
{
  for (size_t i = 0; i < actorList.size(); ++i) {
    actorList[i]->Update(deltaTime);
  }
}

/**
* アクターリストを描画する.
*
* @param actorList 描画するアクターリスト.
* @param matVP     描画に使用するビュープロジェクション行列.
*/
void RenderActorList(const ActorList& actorList,
  const glm::mat4& matVP, const glm::mat4& matShadow)
{
  Global& global = Global::Get();
  for (size_t i = 0; i < actorList.size(); ++i) {
    actorList[i]->Draw(*global.pipeline, matVP, matShadow);
  }
}
