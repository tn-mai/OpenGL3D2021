/**
* @file Primitive.cpp
*/
#include "Primitive.h"

/**
* �v���~�e�B�u��`�悷��.
*/
void Primitive::Draw() const
{
  glDrawElementsBaseVertex(mode, count, GL_UNSIGNED_SHORT, indices, baseVertex);
}