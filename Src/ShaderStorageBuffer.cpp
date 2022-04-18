/**
* @file ShaderStorageBuffer.cpp
*/
#include "ShaderStorageBuffer.h"
#include "GLContext.h"
#include <iostream>

/**
* コンストラクタ
*
* @param size バッファオブジェクトに格納可能なバイト数
*/
ShaderStorageBuffer::ShaderStorageBuffer(size_t size) :
  size(size)
{
  for (auto& e : buffer) {
    glCreateBuffers(1, &e.id);
    glNamedBufferStorage(e.id, size, nullptr, GL_DYNAMIC_STORAGE_BIT);
  }
}

/**
* デストラクタ
*/
ShaderStorageBuffer::~ShaderStorageBuffer()
{
  for (auto& e : buffer) {
    glDeleteBuffers(1, &e.id);
    glDeleteSync(e.sync);
  }
}

/**
* SSBOをGLコンテキストに割り当てる
*
* @param index  割り当て先のバインディングポイント番号
* @param offset 割り当て開始オフセット(256バイト境界に合わせること)
* @param size   割り当てるバイト数(0を指定すると全体を指定したことになる)
*/
void ShaderStorageBuffer::Bind(GLuint index, GLintptr offset, GLsizeiptr size)
{
  const Buffer& e = buffer[renderingBufferIndex];
  if (e.id) {
    if (size == 0) {
      size = GetSize();
    }
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, e.id, offset, size);
  }
}

/**
* SSBOの割り当てを解除する
*
* @param index  割り当て先のバインディングポイント番号
*/
void ShaderStorageBuffer::Unbind(GLuint index)
{
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, index, 0, 0, 0);
}

/**
* GPUメモリにデータをコピーする
*
* @param offset コピー先の先頭オフセット(256バイト境界に合わせること) 
* @param size   コピーするバイト数
* @param data   コピーするデータのアドレス
*/
void ShaderStorageBuffer::BufferSubData(GLintptr offset, GLsizeiptr size, const void* data)
{
  // サイズが0の場合は何もしない
  if (size <= 0) {
    return;
  }

  const size_t updatingBufferIndex =
    (renderingBufferIndex + std::size(buffer) - 1) % std::size(buffer);
  Buffer& e = buffer[updatingBufferIndex];
  if (e.id) {
    // 同期オブジェクトが存在する場合、処理の完了を待つ
    if (e.sync) {
      const GLenum result = glClientWaitSync(e.sync, 0, 0);
      switch (result) {
      case GL_ALREADY_SIGNALED:
        // 既に完了している(正常)
        break;
      case GL_TIMEOUT_EXPIRED:
        std::cerr << "[警告]" << __func__ << ":描画に時間がかかっています(sync=" << e.sync <<")\n";
        glClientWaitSync(e.sync, 0, 1'000'000); // 最大1秒間待つ
        break;
      default:
        std::cerr << "[エラー]" << __func__ << ":同期に失敗(" << result << ")\n";
        break;
      }
      // 同期オブジェクトを削除
      glDeleteSync(e.sync);
      e.sync = 0;
    }
    glNamedBufferSubData(e.id, offset, size, data);
  }
}

/**
* ダブルバッファを切り替える
*/
void ShaderStorageBuffer::SwapBuffers()
{
  renderingBufferIndex = (renderingBufferIndex + std::size(buffer) - 1) % std::size(buffer);
}

/**
* 同期オブジェクトを作成する
*
* SSBOを使う描画関数を実行した直後にこの関数を呼び出すこと。
*/
void ShaderStorageBuffer::FenceSync()
{
  GLsync& sync = buffer[renderingBufferIndex].sync;
  glDeleteSync(sync);
  sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

