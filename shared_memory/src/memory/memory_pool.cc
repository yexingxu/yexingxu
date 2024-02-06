

#include "memory/memory_pool.hpp"

#include <spdlog/spdlog.h>

#include <cstdlib>
#include <string>

namespace shm {
namespace memory {

MemPoolInfo::MemPoolInfo(const uint32_t usedChunks,
                         const uint32_t minFreeChunks, const uint32_t numChunks,
                         const uint32_t chunkSize) noexcept
    : m_usedChunks(usedChunks),
      m_minFreeChunks(minFreeChunks),
      m_numChunks(numChunks),
      m_chunkSize(chunkSize) {}

constexpr uint64_t MemPool::CHUNK_MEMORY_ALIGNMENT;

MemPool::MemPool(
    const greater_or_equal<uint32_t, CHUNK_MEMORY_ALIGNMENT> chunkSize,
    const greater_or_equal<uint32_t, 1> numberOfChunks,
    BumpAllocator& managementAllocator,
    BumpAllocator& chunkMemoryAllocator) noexcept
    : m_chunkSize(chunkSize),
      m_numberOfChunks(numberOfChunks),
      m_minFree(numberOfChunks) {
  if (isMultipleOfAlignment(chunkSize)) {
    auto allocationResult = chunkMemoryAllocator.allocate(
        static_cast<uint64_t>(m_numberOfChunks) * m_chunkSize,
        CHUNK_MEMORY_ALIGNMENT);
    EXPECTS(allocationResult.has_value());
    m_rawMemory = static_cast<uint8_t*>(allocationResult.value());

    allocationResult = managementAllocator.allocate(
        freeList_t::requiredIndexMemorySize(m_numberOfChunks),
        CHUNK_MEMORY_ALIGNMENT);
    EXPECTS(allocationResult.has_value());
    auto* memoryLoFFLi = allocationResult.value();
    m_freeIndices.init(static_cast<LoFFLi::Index_t*>(memoryLoFFLi),
                       m_numberOfChunks);
  } else {
    spdlog::error(
        "Chunk size must be multiple of '" +
        std::to_string(CHUNK_MEMORY_ALIGNMENT) + "'! Requested size is " +
        std::to_string(static_cast<uint32_t>(chunkSize)) + " for " +
        std::to_string(static_cast<uint32_t>(numberOfChunks)) + " chunks!");
    // TODO
    std::exit(EXIT_FAILURE);
    // errorHandler(
    //     PoshError::
    //         MEPOO__MEMPOOL_CHUNKSIZE_MUST_BE_MULTIPLE_OF_CHUNK_MEMORY_ALIGNMENT);
  }
}

bool MemPool::isMultipleOfAlignment(const uint32_t value) const noexcept {
  return (value % CHUNK_MEMORY_ALIGNMENT == 0U);
}

void MemPool::adjustMinFree() noexcept {
  // @todo iox-#1714 rethink the concurrent change that can happen. do we need a
  // CAS loop?
  m_minFree.store(
      std::min(m_numberOfChunks - m_usedChunks.load(std::memory_order_relaxed),
               m_minFree.load(std::memory_order_relaxed)));
}

void* MemPool::getChunk() noexcept {
  uint32_t index{0U};
  if (!m_freeIndices.pop(index)) {
    spdlog::warn("Mempool [m_chunkSize = " + std::to_string(m_chunkSize) +
                 ", numberOfChunks = " + std::to_string(m_numberOfChunks) +
                 ", used_chunks = " + std::to_string(m_usedChunks.load()) +
                 " ] has no more space left");
    return nullptr;
  }

  /// @todo iox-#1714 verify that m_usedChunk is not changed during
  /// adjustMInFree
  ///         without changing m_minFree
  m_usedChunks.fetch_add(1U, std::memory_order_relaxed);
  adjustMinFree();

  return indexToPointer(index, m_chunkSize, m_rawMemory.get());
}

void* MemPool::indexToPointer(uint32_t index, uint32_t chunkSize,
                              void* const rawMemoryBase) noexcept {
  const auto offset = static_cast<uint64_t>(index) * chunkSize;
  return static_cast<void*>(static_cast<uint8_t*>(rawMemoryBase) + offset);
}

uint32_t MemPool::pointerToIndex(const void* const chunk,
                                 const uint32_t chunkSize,
                                 const void* const rawMemoryBase) noexcept {
  const auto offset =
      static_cast<uint64_t>(static_cast<const uint8_t*>(chunk) -
                            static_cast<const uint8_t*>(rawMemoryBase));
  EXPECTS(offset % chunkSize == 0);

  const auto index = static_cast<uint32_t>(offset / chunkSize);
  return index;
}

void MemPool::freeChunk(const void* chunk) noexcept {
  const auto offsetToLastChunk =
      static_cast<uint64_t>(m_chunkSize) * (m_numberOfChunks - 1U);
  EXPECTS(m_rawMemory.get() <= chunk &&
          chunk <=
              static_cast<uint8_t*>(m_rawMemory.get()) + offsetToLastChunk);

  const auto index = pointerToIndex(chunk, m_chunkSize, m_rawMemory.get());

  if (!m_freeIndices.push(index)) {
    // TODO
    // errorHandler(PoshError::POSH__MEMPOOL_POSSIBLE_DOUBLE_FREE);
  }

  m_usedChunks.fetch_sub(1U, std::memory_order_relaxed);
}

uint32_t MemPool::getChunkSize() const noexcept { return m_chunkSize; }

uint32_t MemPool::getChunkCount() const noexcept { return m_numberOfChunks; }

uint32_t MemPool::getUsedChunks() const noexcept {
  return m_usedChunks.load(std::memory_order_relaxed);
}

uint32_t MemPool::getMinFree() const noexcept {
  return m_minFree.load(std::memory_order_relaxed);
}

MemPoolInfo MemPool::getInfo() const noexcept {
  return {m_usedChunks.load(std::memory_order_relaxed),
          m_minFree.load(std::memory_order_relaxed), m_numberOfChunks,
          m_chunkSize};
}

}  // namespace memory
}  // namespace shm