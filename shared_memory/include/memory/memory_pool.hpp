

#pragma once

#include <atomic>
#include <cstdint>

#include "memory/bump_allocator.hpp"
#include "memory/loffli.hpp"
#include "shm/algorithm.hpp"
#include "shm/relative_pointer.hpp"

namespace shm {
namespace memory {

struct MemPoolInfo {
  MemPoolInfo(const uint32_t usedChunks, const uint32_t minFreeChunks,
              const uint32_t numChunks, const uint32_t chunkSize) noexcept;

  uint32_t m_usedChunks{0};
  uint32_t m_minFreeChunks{0};
  uint32_t m_numChunks{0};
  uint32_t m_chunkSize{0};
};

class MemPool {
 public:
  using freeList_t = LoFFLi;
  static constexpr uint64_t CHUNK_MEMORY_ALIGNMENT =
      8U;  // default alignment for 64 bit

  MemPool(const greater_or_equal<uint32_t, CHUNK_MEMORY_ALIGNMENT> chunkSize,
          const greater_or_equal<uint32_t, 1> numberOfChunks,
          BumpAllocator& managementAllocator,
          BumpAllocator& chunkMemoryAllocator) noexcept;

  MemPool(const MemPool&) = delete;
  MemPool(MemPool&&) = delete;
  MemPool& operator=(const MemPool&) = delete;
  MemPool& operator=(MemPool&&) = delete;

  void* getChunk() noexcept;
  uint32_t getChunkSize() const noexcept;
  uint32_t getChunkCount() const noexcept;
  uint32_t getUsedChunks() const noexcept;
  uint32_t getMinFree() const noexcept;
  MemPoolInfo getInfo() const noexcept;

  void freeChunk(const void* chunk) noexcept;

  /// @brief Converts an index to a chunk in the MemPool to a pointer
  /// @param[in] index of the chunk
  /// @param[in] chunkSize is the size of the chunk
  /// @param[in] rawMemoryBase it the pointer to the raw memory of the MemPool
  /// @return the pointer to the chunk
  static void* indexToPointer(const uint32_t index, const uint32_t chunkSize,
                              void* const rawMemoryBase) noexcept;

  /// @brief Converts a pointer to a chunk in the MemPool to an index
  /// @param[in] chunk is the pointer to the chunk
  /// @param[in] chunkSize is the size of the chunk
  /// @param[in] rawMemoryBase it the pointer to the raw memory of the MemPool
  /// @return the index to the chunk
  static uint32_t pointerToIndex(const void* const chunk,
                                 const uint32_t chunkSize,
                                 const void* const rawMemoryBase) noexcept;

 private:
  void adjustMinFree() noexcept;
  bool isMultipleOfAlignment(const uint32_t value) const noexcept;

  RelativePointer<void> m_rawMemory;

  uint32_t m_chunkSize{0U};
  /// needs to be 32 bit since loffli supports only 32 bit numbers
  /// (cas is only 64 bit and we need the other 32 bit for the aba counter)
  uint32_t m_numberOfChunks{0U};

  std::atomic<uint32_t> m_usedChunks{0U};
  std::atomic<uint32_t> m_minFree{0U};

  freeList_t m_freeIndices;
};

}  // namespace memory
}  // namespace shm