/**
* @file Light.h
*/
#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED
#include <glm/vec3.hpp>

// 平行光源
struct DirectionalLight
{
  glm::vec3 direction = { 0.7f, -0.6f, -0.4f }; // ライトの向き
  glm::vec3 color = { 1.7f, 1.7f, 1.4f }; // ライトの色(明るさ)
};

#endif // LIGHT_H_INCLUDED
