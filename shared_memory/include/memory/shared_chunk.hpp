#pragma once

#include "memory/chunk_management.hpp"

namespace shm {
namespace memory {

template <typename>
class SharedPointer;

/// @brief WARNING: SharedChunk is not thread safe! Don't share SharedChunk
/// objects between threads! Use for each thread a separate copy
class SharedChunk {
 public:
  SharedChunk() noexcept = default;
  SharedChunk(ChunkManagement* const resource) noexcept;
  ~SharedChunk() noexcept;

  SharedChunk(const SharedChunk& rhs) noexcept;
  SharedChunk(SharedChunk&& rhs) noexcept;

  SharedChunk& operator=(const SharedChunk& rhs) noexcept;
  SharedChunk& operator=(SharedChunk&& rhs) noexcept;

  ChunkHeader* getChunkHeader() const noexcept;
  void* getUserPayload() const noexcept;

  ChunkManagement* release() noexcept;

  bool operator==(const SharedChunk& rhs) const noexcept;
  /// @todo iox-#1617 use the newtype pattern to avoid the void pointer
  bool operator==(const void* const rhs) const noexcept;

  bool operator!=(const SharedChunk& rhs) const noexcept;
  bool operator!=(const void* const rhs) const noexcept;

  explicit operator bool() const noexcept;

  template <typename>
  friend class SharedPointer;

 private:
  void decrementReferenceCounter() noexcept;
  void incrementReferenceCounter() noexcept;
  void freeChunk() noexcept;

 private:
  ChunkManagement* m_chunkManagement{nullptr};
};

}  // namespace memory
}  // namespace shm