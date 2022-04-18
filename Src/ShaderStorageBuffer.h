/**
* @file ShaderStorageBuffer.h
*/
#ifndef SHADERSTORAGEBUFFER_H_INCLUDED
#define SHADERSTORAGEBUFFER_H_INCLUDED
#include "glad/glad.h"
#include <memory>

inline size_t CalcSsboAlign(size_t n) { return ((n + 255) / 256) * 256; };

/**
* SSBO
*/
class ShaderStorageBuffer
{
public:
  ShaderStorageBuffer(size_t size);
  ~ShaderStorageBuffer();

  void Bind(GLuint index, GLintptr offset = 0, GLsizeiptr size = 0);
  void Unbind(GLuint index);
  GLsizeiptr GetSize() const { return size; }
  void BufferSubData(GLintptr offet, GLsizeiptr size, const void* data);
  void FenceSync();
  void SwapBuffers();

private:
  struct Buffer {
    GLuint id = 0;
    GLsync sync = 0;
  } buffer[2];
  size_t size = 0; // 格納できるバイト数
  size_t renderingBufferIndex = 0; // 描画するバッファオブジェクトの番号
};

using ShaderStorageBufferPtr = std::shared_ptr<ShaderStorageBuffer>;

#endif // SHADERSTORAGEBUFFER_H_INCLUDED
