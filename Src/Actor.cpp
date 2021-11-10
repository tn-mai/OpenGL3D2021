/**
* @file Actor.cpp
*/
#include "Actor.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>

/**
* コンストラクタ
*/
Actor::Actor(
  const std::string& name,
  const Primitive& prim,
  std::shared_ptr<Texture> tex,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment) :
  name(name),
  prim(prim),
  tex(tex),
  position(position),
  scale(scale),
  rotation(rotation),
  adjustment(adjustment)
{
}

/**
* アクターの状態を更新する
*
* @param deltaTime 前回の更新からの経過時間(秒)
*/
void Actor::OnUpdate(float deltaTime)
{
  // 何もしない
}

/**
* 衝突を処理する
*
* @param contact 衝突情報
*/
void Actor::OnCollision(const struct Contact& contact)
{
  // 何もしない
}

/**
* 物体を描画する.
*/
void Draw(
  const Actor& actor,              // 物体の制御パラメータ
  const ProgramPipeline& pipeline, // 描画に使うプログラムパイプライン
  glm::mat4 matProj,               // 描画に使うプロジェクション行列
  glm::mat4 matView)               // 描画に使うビュー行列  
{
  // モデル行列を計算する
  glm::mat4 matT = glm::translate(glm::mat4(1), actor.position);
  glm::mat4 matR = glm::rotate(glm::mat4(1), actor.rotation, glm::vec3(0, 1, 0));
  glm::mat4 matS = glm::scale(glm::mat4(1), actor.scale);
  glm::mat4 matA = glm::translate(glm::mat4(1), actor.adjustment);
  glm::mat4 matModel = matT * matR * matS * matA;

  // MVP行列を計算する
  glm::mat4 matMVP = matProj * matView * matModel;

  // モデル行列とMVP行列をGPUメモリにコピーする
  const GLint locMatTRS = 0;
  const GLint locMatModel = 1;
  pipeline.SetUniform(locMatTRS, matMVP);
  if (actor.layer == Layer::Default) {
    pipeline.SetUniform(locMatModel, matModel);
  }

  // TODO: テキスト未追加
  const GLint locColor = 200;
  pipeline.SetUniform(locColor, actor.color);

  if (actor.tex) {
    actor.tex->Bind(0); // テクスチャを割り当てる
  } else {
  }
  actor.prim.Draw();  // プリミティブを描画する
}

/**
* 名前の一致するアクターを検索する.
*
* @param actors 検索対象の配列.
* @param name   検索するアクターの名前.
*
* @retval nullptr以外 最初にnameと名前の一致したアクター.
* @retval nullptr     actorsの中に名前の一致するアクターがない.
*/
std::shared_ptr<Actor> Find(std::vector<std::shared_ptr<Actor>>& actors, const char* name)
{
  for (int i = 0; i < actors.size(); ++i) {
    if (actors[i]->name == name) {
      return actors[i];
    }
  }
  return nullptr;
}

/**
* AとBを入れ替える.
*/
Contact Reverse(const Contact& contact)
{
  Contact result;

  result.a = contact.b;
  result.b = contact.a;
  result.velocityA = contact.velocityB;
  result.velocityB = contact.velocityA;
  result.accelA = contact.accelB;
  result.accelB = contact.accelA;
  result.penetration = -contact.penetration;
  result.normal = -contact.normal;
  result.position = contact.position;
  result.penLength = contact.penLength;
  result.massB = contact.a->mass;

  return result;
}

bool DummyCollisionFunc(Actor&, Actor&, Contact&)
{
  return false;
}

/**
* 衝突を検出する
*
* @param actorA  衝突しているか調べるアクター
* @param actorB  衝突しているか調べるアクター
* @param contact 衝突情報
*
* @retval true  衝突している
* @retval false 衝突していない
*/
bool DetectCollision(Actor& actorA, Actor& actorB, Contact& contact)
{
  // 質量を持たない物体同士は衝突しない
  if (actorA.mass <= 0 && actorB.mass <= 0) {
    return false;
  }

  // 動かせない物体同士は衝突しない
  if (actorA.isStatic && actorB.isStatic) {
    return false;
  }

  // コライダーが設定されていない物体は衝突しない
  if (!actorA.collider || !actorB.collider) {
    return false;
  }

  using CollisionFunc = bool(*)(Actor&, Actor&, Contact&);
  static const CollisionFunc funcArray[3][3] = {
    //              box, sphere, cylinder
    /* box      */ { CollisionBoxBox, CollisionBoxSphere, CollisionBoxCylinder },
    /* sphere   */ { CollisionSphereBox, CollisionSphereSphere, DummyCollisionFunc },
    /* cylinder */ { CollisionCylinderBox, DummyCollisionFunc, CollisionCylinderCylinder },
  };
  const int y = static_cast<int>(actorA.collider->GetShapeType());
  const int x = static_cast<int>(actorB.collider->GetShapeType());
  return funcArray[y][x](actorA, actorB, contact);
}

/**
* 重なりを解決する
*
* @param contact 衝突情報

@todo contactをアクター毎に生成(つまり1衝突につき2個)
@todo アクター及び法線の等しいcontactを統合(Equal関数の修正)
*/
void SolveContact(Contact& contact)
{
  Actor& actorA = *contact.a;
  Actor& actorB = *contact.b;
  glm::vec3 normal = contact.normal;
  glm::vec3 penetration = contact.penetration;

  // 反発係数の式 e = (Va' - Vb') / (Vb - Va) から e = 1 のとき
  //   Va' - Vb' = Vb - Va ...式(1)
  // 運動量保存則より
  //   Ma*Va + Mb*Vb = Ma*Va' + Mb*Vb' ...式(2)
  // 式(1)をVb'について解くと
  //  -Vb' = Vb - Va - Va'
  //   Vb' = -Vb + Va + Va'
  //   Vb' = Va - Vb + Va' ...式(3)
  // 式(3)を式(2)に代入すると
  //   Ma*Va + Mb*Vb = Ma*Va' + Mb(Va - Vb + Va') ...式(4)
  // 式(4)をVa'について解くと
  //   Ma*Va + Mb*Vb = Ma*Va' + Mb*Va - Mb*Vb + Mb*Va'
  //   Ma*Va + Mb*Vb = (Ma+Mb)Va' + Mb*Va - Mb*Vb
  //   Ma*Va + Mb*Vb - (Ma+Mb)Va' = Mb*Va - Mb*Vb
  //   -(Ma+Mb)Va' = Mb*Va - Mb*Vb - Ma*Va - Mb*Vb
  //   (Ma+Mb)Va' = -Mb*Va + Mb*Vb + Ma*Va + Mb*Vb
  //   (Ma+Mb)Va' = Ma*Va + Mb*Vb + Mb(Vb - Va)
  //   Va' = (Ma*Va + Mb*Vb + Mb(Vb - Va)) / (Ma+Mb) ...式(5)
  //
  // 同様にVb'について解くと
  //   Ma*Va + Mb*Vb = Ma*Vb - Ma*Va + Ma*Vb' + Mb*Vb'
  //   Ma*Va + Mb*Vb - Ma*Vb + Ma*Va = (Ma+Mb)Vb'
  //   Vb' = (Ma*Va + Mb*Vb + Ma(Va - Vb)) / (Ma+Mb)
  //
  // Aが静止物の場合、Bの質量Mbを0として式(5)を解く. すると
  //   Va' = (Ma*Va + 0*Vb + 0(Vb - Va)) / (Ma+0)
  //   Va' = Ma*Va / Ma
  //   Va' = Va
  //
  // NOTE: 摩擦と反発は非常に複雑な物理現象なので、そもそも式を立てることは不可能に近い.
  //       そのため、物理エンジンでは物理的な正確性を無視して適当に計算している.
  //       Bullet3は乗算、UE4、Unityはデフォルトでは平均を使っているようだ.
  //       Box2D-liteでは乗算して平方根を取っている. 重くなるが結果を制御しやすいようだ.
  //       今回は長いものに巻かれることにして、平均を採用する.

  // 反発係数の平均値を計算
  float cor = (actorA.cor + actorB.cor) * 0.5f;

  // 摩擦係数の平均値を計算
  float friction = (actorA.friction + actorB.friction) * 0.5f;

  // アクターBに対するアクターAの相対ベロシティを計算
  glm::vec3 rv = contact.velocityA - contact.velocityB;

  // 衝突面と相対ベロシティに平行なベクトル(タンジェント)を計算
  glm::vec3 tangent = glm::cross(normal, glm::cross(normal, rv));
  if (glm::length(tangent) > 0.000001f) {
    tangent = glm::normalize(tangent);
  } else {
    tangent = glm::vec3(0);
  }
  
  // 摩擦から受けるベロシティを計算
#if 1
  // 摩擦力
  float frictionForce = friction * 9.8f / 60.0f;

  // 摩擦力の最大値を計算
  float maxForce = std::abs(glm::dot(tangent, rv));

  // 摩擦力を最大値に制限
  frictionForce = std::min(frictionForce, maxForce);

  // タンジェント方向の摩擦力を計算
  glm::vec3 frictionVelocity = normal.y * frictionForce * tangent;
#else
  glm::vec3 relAccel = (contact.accelA - contact.accelB);
  float maxf = abs(glm::dot(tangent, rv));
  float tmp = glm::dot(normal, relAccel);
  float ff = std::min(maxf, abs(tmp));
  if (tmp < 0) {
    ff *= -1;
  }
  glm::vec3 frictionVelocity = friction * ff * tangent;
#endif

  // 速度の更新: 法線方向の速度を計算
  float ua = glm::dot(normal, contact.velocityA);
  float ub = glm::dot(normal, contact.velocityB);
  if (actorA.isStatic) {
    //const float ratio = 1.0f / actorB.contactCount;
    const float ratio = 1.0f;
    float vb = ua + cor * (ua - ub);     // 衝突後の速度を計算
    actorB.velocity -= normal * ub * ratio;      // 衝突前の速度を0にする
    actorB.velocity += normal * vb * ratio;      // 衝突後の速度を加算する
    actorB.velocity += frictionVelocity * ratio; // 摩擦による速度を加算する

    // 重なりの解消: アクターAは動かないので、アクターBを動かす
    actorB.position += penetration * ratio;
    if (normal.y < 0) {
      //float vy = actorB.velocity.y - actorA.velocity.y;
      //if ( vy < 1.0f) {
        actorB.isOnActor = true;
      //}
    }
  }
  else if (actorB.isStatic) {
    //const float ratio = 1.0f / actorA.contactCount;
    const float ratio = 1.0f;
    float va = ub + cor * (ub - ua);     // 衝突後の速度を計算
    actorA.velocity -= normal * ua * ratio;      // 衝突前の速度を0にする
    actorA.velocity += normal * va * ratio;      // 衝突後の速度を加算する
    actorA.velocity += frictionVelocity * ratio; // 摩擦による速度を加算する

    // 重なりの解消: アクターBは動かないので、アクターAを動かす
    actorA.position -= penetration * ratio;

    if (normal.y > 0) {
      //float vy = actorA.velocity.y - actorB.velocity.y;
      //if ( vy < 1.0f) {
        actorA.isOnActor = true;
      //}
    }
  }
  else {
    // 速度の更新: 運動エネルギーの分配量を計算
    const float ratioA = 1.0f;// / actorA.contactCount;
    const float ratioB = 1.0f;// / actorB.contactCount;
    float massA = actorA.mass * ratioA;
    float massB = actorB.mass * ratioB;
    float massAB = massA + massB;
    float c = massA * ua + massB * ub;
    float va = (c + cor * massB * (ub - ua)) / massAB;
    float vb = (c + cor * massA * (ua - ub)) / massAB;

    // 衝突前の速度を0にする
    actorA.velocity -= normal * ua;
    actorB.velocity -= normal * ub;

    // 衝突後の速度を加算する
    actorA.velocity += normal * va;
    actorB.velocity += normal * vb;

    // 摩擦による速度を加算する
    actorA.velocity += frictionVelocity * ratioA;
    actorB.velocity -= frictionVelocity * ratioB;

    // 重なりの解消: 運動エネルギーの比率で動かす距離を決める
    //              運動エネルギーがゼロの場合は質量の比率で決める
    float rA = abs(va);
    float rB = abs(vb);
    if (rA <= 0.0f && rB <= 0.0f) {
      rA = massA;
      rB = massB;
    }
    glm::vec3 pa = penetration * rA / (rA + rB);
    glm::vec3 pb = penetration * rB / (rA + rB);
    actorA.position -= pa * ratioA;
    actorB.position += pb * ratioB;

    if (normal.y > 0) {
      //float vy = actorA.velocity.y - actorB.velocity.y;
      //if ( vy < 1.0f) {
        actorA.isOnActor = true;
      //}
    } else if (normal.y < 0) {
      //float vy = actorB.velocity.y - actorB.velocity.y;
      //if ( vy < 1.0f) {
//        actorB.isOnActor = true;
      //}
    }
  }
}

/**
* 2つのコンタクト構造体が似ているか調べる
*
* @param ca 比較するコンタクト構造体その1.
* @param cb 比較するコンタクト構造体その2.
*
* @return true  似ている
* @return false 似ていない
*/
bool Equal(const Contact& ca, const Contact& cb)
{
  if (!ca.a || !ca.b || !cb.a || !cb.b) {
    return false;
  }

  // 衝突面の距離が離れている場合は似ていない
  // NOTE: この判定は完全ではない.
  //       DetectCollisionは引数で渡された2つのアクターA, Bのうち、Bの表面に接触点を定義する.
  //       そのため、アクターXとアクターYがあったとき、A=X, B=YとするかA=Y, B=Xとするかで接触点と法線が異なる.
  //       すべての交点に接触点を生成すると、この問題を解決できるかもしれない.
  if (glm::length(ca.position - cb.position) > 0.01f) {
    return false; // 似ていない
  }

#if 0 // 現在の実装では座標が一致するなら法線も一致する
  // 法線の方向が一致しない場合は似ていない
  if (glm::dot(ca.normal, normalB) <= 0) {
    return false; // 似ていない
  }
#endif

  // 静物アクターの有無によって判定を分ける
  glm::vec3 normalB = cb.normal;
  bool hasStaticA = ca.a->isStatic || ca.b->isStatic;
  bool hasStaticB = cb.a->isStatic || cb.b->isStatic;
  switch (hasStaticA + hasStaticB * 2) {
  case 0b00: // A,Bともに動くアクターのみ
    // アクターが両方とも一致したら似ている
    if (ca.a == cb.a && ca.b == cb.b) {
      break;
    }
    if (ca.a == cb.b && ca.b == cb.a) {
      break;
    }
    return false;

  case 0b01: // A=動かないアクターを含む, B=動くアクターのみ
    // 常に似ていないと判定する
    return false;

  case 0b10: // A=動くアクターのみ B=動かないアクターを含む
    // 常に似ていないと判定する
    return false;

  case 0b11: // A,Bともに動かないアクターを含む 
    {
    // 動くアクター同士が一致したら似ている
    Actor* a = ca.a;
    if (ca.a->isStatic) {
      a = ca.b;
    }
    Actor* b = cb.a;
    if (cb.a->isStatic) {
      b = cb.b;
    }
    if (a == b) {
      break;
    }
    }
    return false;
  }

  return true; // 似ている
}

/**
* Contact構造体をA,Bそれぞれに作成する場合の比較関数.
* 
* アクターAと法線が等しい場合にtrue
* つまり、Aに同じ方向から接触した場合は等しいと判定する.
*/
bool Equal2(const Contact& ca, const Contact& cb)
{
  if (ca.a != cb.a) {
    return false;
  }
  if (glm::dot(ca.normal, cb.normal) <= glm::cos(glm::radians(1.0f))) {
    return false;
  }
  return true;
}

