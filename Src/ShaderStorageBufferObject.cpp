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
  glDeleteSync(sync);
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
  // 書き込んだデータを使った描画の完了を待つ.
  if (sync) {
    glClientWaitSync(sync, 0, 1000ULL*1000ULL);
    glDeleteSync(sync);
    sync = nullptr;
  }
  memcpy(static_cast<char*>(p) + offset, data, size);
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
  // この時点でsyncが有効な場合、描画ループ中で2回以上Unbindが実行されている.
  // 最後のUnbind時点のフェンスだけが重要なので、以前のsyncを破棄してから取得する.
  glDeleteSync(sync);
  sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

