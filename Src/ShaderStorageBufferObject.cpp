/**
* @file ShaderStorageBufferObject.cpp
*/
#include "ShaderStorageBufferObject.h"
#include "GLContext.h"
#include <iostream>

/**
* コンストラクタ.
*
* @param size  SSBOの大きさ(バイト数).
*/
ShaderStorageBufferObject::ShaderStorageBufferObject(size_t size)
{
  id = GLContext::CreateBuffer(size, nullptr,
    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  p = glMapNamedBufferRange(id, 0, size,
    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
  this->size = size;
}

/**
* デストラクタ.
*/
ShaderStorageBufferObject::~ShaderStorageBufferObject()
{
  glUnmapNamedBuffer(id);
  glDeleteBuffers(1, &id);
#ifdef AVOID_AMD_DRIVER_BUG_FOR_PERSISTENT_MAP
  glDeleteSync(sync);
#endif
}

/**
* データをSSBOにコピーする.
*
* @param data コピーするデータのアドレス.
* @param size コピーするバイト数.
* @param offset コピー先の先頭位置(バイト単位).
*/
void ShaderStorageBufferObject::CopyData(
  const void* data, size_t size, size_t offset) const
{
  if (offset + size > this->size) {
    std::cerr << "[エラー]" << __func__ <<
      ": サイズまたはオフセットが大きすぎます(size=" << size <<
      ",offset=" << offset << "/" << this->size << ").\n";
    return;
  }
#ifdef AVOID_AMD_DRIVER_BUG_FOR_PERSISTENT_MAP
  if (sync) {
    glClientWaitSync(sync, 0, 1000ULL*1000ULL);
    glDeleteSync(sync);
  }
#endif

  memcpy(static_cast<char*>(p) + offset, data, size);

#ifdef AVOID_AMD_DRIVER_BUG_FOR_PERSISTENT_MAP
  sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
#else
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
#endif
}

/**
* SSBOをグラフィックスパイプラインに割り当てる.
*
* @param location 割り当て先のロケーション番号.
*/
void ShaderStorageBufferObject::Bind(GLuint location) const
{
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, id);
}

/**
* SSBOのグラフィックスパイプラインへの割り当てを解除する.
*
* @param location 割り当て先のロケーション番号.
*/
void ShaderStorageBufferObject::Unbind(GLuint location) const
{
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location, 0);
}

