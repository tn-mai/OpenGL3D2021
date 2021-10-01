/**
* @file ImageActor.cpp
*/
#include "ImageActor.h"
#include "../GameEngine.h"

/**
* コンストラクタ
*/
ImageActor::ImageActor(
  const std::string& name,
  const char* filename,
  const glm::vec3& position,
  const glm::vec3& scale,
  float rotation,
  const glm::vec3& adjustment) :
  Actor(name,
    GameEngine::Get().GetPrimitive("Res/Plane.obj"),
    GameEngine::Get().LoadTexture(filename),
    position, scale, rotation, adjustment)
{
  isStatic = true;
  layer = Layer::UI;
}
