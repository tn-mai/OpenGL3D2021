/*
* @file FlagIfDied.h
*/
#ifndef FLAGIFDIED_H_INCLUDED
#define FLAGIFDIED_H_INCLUDED
#include "../Actor.h"

/**
* ���񂾂�t���O�𗧂Ă�A�N�^�[
*/
class FlagIfDied : public Actor
{
public:
  FlagIfDied(
    const std::string& name,
    const Primitive& prim,
    std::shared_ptr<Texture> tex,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment);

  FlagIfDied(
    const std::string& name,
    const MeshPtr& mesh,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment);

  virtual ~FlagIfDied() = default;
  virtual std::shared_ptr<Actor> Clone() const override;
  virtual void OnCollision(const struct Contact& contact) override;

  void SetFlagNo(int no) { flagNo = no; }
  int GetFlagNo() const { return flagNo; }

private:
  int flagNo = 0; // ���삷��t���O�ԍ�
};

#endif // FLAGIFDIED_H_INCLUDED
