/**
* @file Primitive.cpp
*/
#include "Primitive.h"

/**
* プリミティブを描画する.
*/
void Primitive::Draw() const
{
  glDrawElementsBaseVertex(mode, count, GL_UNSIGNED_SHORT, indices, baseVertex);
}