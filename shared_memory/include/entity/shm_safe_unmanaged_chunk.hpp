#pragma once

#include "memory/shared_chunk.hpp"
#include "shm/relative_pointer_data.hpp"

namespace shm {
namespace entity {

class ShmSafeUnmanagedChunk {
 public:
  ShmSafeUnmanagedChunk() noexcept = default;

  /// @brief takes a SharedChunk without decrementing the chunk reference
  /// counter
  ShmSafeUnmanagedChunk(memory::SharedChunk chunk) noexcept;

  /// @brief Creates a SharedChunk without incrementing the chunk reference
  /// counter and invalidates itself
  memory::SharedChunk releaseToSharedChunk() noexcept;

  /// @brief Creates a SharedChunk with incrementing the chunk reference counter
  /// and does not invalidate itself
  memory::SharedChunk cloneToSharedChunk() noexcept;

  /// @brief Checks if the underlying RelativePointerData to the chunk is
  /// logically a nullptr
  /// @return true if logically a nullptr otherwise false
  bool isLogicalNullptr() const noexcept;

  /// @brief Access to the ChunkHeader of the underlying chunk
  /// @return the pointer to the ChunkHeader of the underlying chunk or nullptr
  /// if isLogicalNullptr would return true
  memory::ChunkHeader* getChunkHeader() noexcept;

  /// @brief const access to the ChunkHeader of the underlying chunk
  /// @return the const pointer to the ChunkHeader of the underlying chunk or
  /// nullptr if isLogicalNullptr would return true
  const memory::ChunkHeader* getChunkHeader() const noexcept;

  /// @brief Checks if the underlying RelativePointerData to the chunk is
  /// neither logically a nullptr nor that the chunk has other owner
  /// @return true if neither logically a nullptr nor other owner chunk owners
  /// present, otherwise false
  bool isNotLogicalNullptrAndHasNoOtherOwners() const noexcept;

 private:
  details::RelativePointerData m_chunkManagement;
};

}  // namespace entity
}  // namespace shm