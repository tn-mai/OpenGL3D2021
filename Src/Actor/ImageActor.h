/**
* @file ImageActor.h
*/
#ifndef IMAGEACTOR_H_INCLUDED
#define IMAGEACTOR_H_INCLUDED
#include "../Actor.h"

/**
* 2Dアクター
*/
class ImageActor : public Actor
{
public:
  ImageActor(
    const std::string& name,
    const char* filename,
    const glm::vec3& position,
    const glm::vec3& scale,
    float rotation,
    const glm::vec3& adjustment);

  virtual ~ImageActor() = default;
};

#endif // IMAGEACTOR_H_INCLUDED

