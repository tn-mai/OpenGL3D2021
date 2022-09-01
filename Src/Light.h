/**
* @file Light.h
*/
#ifndef LIGHT_H_INCLUDED
#define LIGHT_H_INCLUDED
#include <glm/vec3.hpp>

// ���s����
struct DirectionalLight
{
  glm::vec3 direction = { 0.7f, -0.6f, -0.4f }; // ���C�g�̌���
  glm::vec3 color = { 1.7f, 1.7f, 1.4f }; // ���C�g�̐F(���邳)
};

#endif // LIGHT_H_INCLUDED
