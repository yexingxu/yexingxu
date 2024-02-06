

#pragma once

#include <cstdint>
#include <limits>
#include <memory>

#include "memory/bump_allocator.hpp"
#include "memory/chunk_header.hpp"
#include "memory/config.hpp"
#include "memory/memory_pool.hpp"
#include "memory/shared_chunk.hpp"
#include "shm/algorithm.hpp"

namespace shm {
namespace memory {

class MemoryManager {
  using MaxChunkPayloadSize_t =
      details::range<uint32_t, 1,
                     std::numeric_limits<uint32_t>::max() -
                         sizeof(memory::ChunkHeader)>;

 public:
  enum class Error {
    NO_MEMPOOLS_AVAILABLE,
    NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE,
    MEMPOOL_OUT_OF_CHUNKS,
  };

  MemoryManager() noexcept = default;
  MemoryManager(const MemoryManager&) = delete;
  MemoryManager(MemoryManager&&) = delete;
  MemoryManager& operator=(const MemoryManager&) = delete;
  MemoryManager& operator=(MemoryManager&&) = delete;
  ~MemoryManager() noexcept = default;

  void configureMemoryManager(const Config& mePooConfig,
                              BumpAllocator& managementAllocator,
                              BumpAllocator& chunkMemoryAllocator) noexcept;

  /// @brief Obtains a chunk from the mempools
  /// @param[in] chunkSettings for the requested chunk
  /// @return a SharedChunk if successful, otherwise a MemoryManager::Error
  tl::expected<SharedChunk, Error> getChunk(
      const ChunkSettings& chunkSettings) noexcept;

  uint32_t getNumberOfMemPools() const noexcept;

  MemPoolInfo getMemPoolInfo(const uint32_t index) const noexcept;

  static uint64_t requiredChunkMemorySize(const Config& mePooConfig) noexcept;
  static uint64_t requiredManagementMemorySize(
      const Config& mePooConfig) noexcept;
  static uint64_t requiredFullMemorySize(const Config& mePooConfig) noexcept;

 private:
  static uint32_t sizeWithChunkHeaderStruct(
      const MaxChunkPayloadSize_t size) noexcept;

  void printMemPoolVector(std::ostream& log) const noexcept;
  void addMemPool(
      BumpAllocator& managementAllocator, BumpAllocator& chunkMemoryAllocator,
      const greater_or_equal<uint32_t, MemPool::CHUNK_MEMORY_ALIGNMENT>
          chunkPayloadSize,
      const greater_or_equal<uint32_t, 1> numberOfChunks) noexcept;
  void generateChunkManagementPool(BumpAllocator& managementAllocator) noexcept;

 private:
  bool m_denyAddMemPool{false};
  uint32_t m_totalNumberOfChunks{0};

  static constexpr std::uint32_t kMaxMemoryPoolSize = 32U;

  std::vector<std::unique_ptr<MemPool>> m_memPoolVector;
  std::vector<std::unique_ptr<MemPool>> m_chunkManagementPool;
};

inline constexpr const char* asStringLiteral(
    const MemoryManager::Error value) noexcept {
  switch (value) {
    case MemoryManager::Error::NO_MEMPOOLS_AVAILABLE:
      return "MemoryManager::Error::NO_MEMPOOLS_AVAILABLE";
    case MemoryManager::Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE:
      return "MemoryManager::Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE";
    case MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS:
      return "MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS";
    default:
      return "[Undefined MemoryManager::Error]";
  }

  return "[Undefined MemoryManager::Error]";
}

}  // namespace memory
}  // namespace shm