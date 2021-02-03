/**
* @file ShaderStorageBufferObject.h
*/
#ifndef SHADERSTORAGEBUFFEROBJECT_H_INLCUDED
#define SHADERSTORAGEBUFFEROBJECT_H_INLCUDED
#include "glad/glad.h"

#define AVOID_AMD_DRIVER_BUG_FOR_PERSISTENT_MAP

/**
* シェーダー・ストレージ・バッファ・オブジェクト(SSBO).
*/
class ShaderStorageBufferObject
{
public:
  explicit ShaderStorageBufferObject(size_t size);
  ~ShaderStorageBufferObject();
  ShaderStorageBufferObject(const ShaderStorageBufferObject&) = delete;
  ShaderStorageBufferObject& operator=(const ShaderStorageBufferObject&) = delete;

  void CopyData(const void* data, size_t size, size_t offset) const;
  void Bind(GLuint location) const;
  void Unbind(GLuint location) const;

private:
  GLuint id = 0;     // オブジェクトID
  size_t size = 0;   // バッファの大きさ(単位=バイト).
  void* p = nullptr; // データの転送先を指すポインタ.
#ifdef AVOID_AMD_DRIVER_BUG_FOR_PERSISTENT_MAP
  mutable GLsync sync = nullptr;
#endif
};

#endif // SHADERSTORAGEBUFFEROBJECT_H_INLCUDED
