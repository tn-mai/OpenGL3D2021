/**
* @file MoveIfFlagged.h
*/
#ifndef MOVEIFFLAGGED_H_INCLUDED
#define MOVEIFFLAGGED_H_INCLUDED
#include "../Actor.h"

/**
* フラグの状態を見て位置を変えるアクター
*/
class MoveIfFlagged : public Actor
{
public:
  MoveIfFlagged(
    const std::string& name,
    const Primitive& prim,
    std::shared_ptr<Texture> tex,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment);

  MoveIfFlagged(
    const std::string& name,
    const MeshPtr& mesh,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment);

  virtual ~MoveIfFlagged() = default;
  virtual std::shared_ptr<Actor> Clone() const override;
  virtual void OnUpdate(float deltaTime) override;

  void SetFlagNo(int no) { flagNo = no; }
  int GetFlagNo() const { return flagNo; }
  void SetPosition(bool flag, const glm::vec3& p) { pos[flag] = p; }
  const glm::vec3& GetPosition(bool flag) const { return pos[flag]; }

private:
  int flagNo = 0;
  glm::vec3 pos[2] = {};
};

#endif // MOVEIFFLAGGED_H_INCLUDED

