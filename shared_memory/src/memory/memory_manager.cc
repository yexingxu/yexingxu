

#include "memory/memory_manager.hpp"

#include <spdlog/spdlog.h>

#include <cstdlib>
#include <ostream>

#include "shm/algorithm.hpp"
#include "shm/memory.hpp"
#include "shm/utils.h"

namespace shm {
namespace memory {

void MemoryManager::printMemPoolVector(std::ostream& log) const noexcept {
  for (auto& l_mempool : m_memPoolVector) {
    log << "  MemPool [ ChunkSize = " << l_mempool->getChunkSize()
        << ", ChunkPayloadSize = "
        << l_mempool->getChunkSize() - sizeof(ChunkHeader)
        << ", ChunkCount = " << l_mempool->getChunkCount() << " ]";
  }
}

void MemoryManager::addMemPool(
    BumpAllocator& managementAllocator, BumpAllocator& chunkMemoryAllocator,
    const greater_or_equal<uint32_t, MemPool::CHUNK_MEMORY_ALIGNMENT>
        chunkPayloadSize,
    const greater_or_equal<uint32_t, 1> numberOfChunks) noexcept {
  uint32_t adjustedChunkSize =
      sizeWithChunkHeaderStruct(static_cast<uint32_t>(chunkPayloadSize));
  if (m_denyAddMemPool) {
    spdlog::error(
        "After the generation of the chunk management pool you are not "
        "allowed to create new mempools.");
    std::exit(EXIT_FAILURE);
    // errorHandler(
    //     iox::PoshError::
    //         MEPOO__MEMPOOL_ADDMEMPOOL_AFTER_GENERATECHUNKMANAGEMENTPOOL);
  } else if (m_memPoolVector.size() > 0 &&
             adjustedChunkSize <= m_memPoolVector.back()->getChunkSize()) {
    spdlog::error(
        "The following mempools were already added to the mempool handler:");
    std::exit(EXIT_FAILURE);
  }

  m_memPoolVector.emplace_back(
      std::make_unique<MemPool>(adjustedChunkSize, numberOfChunks,
                                managementAllocator, chunkMemoryAllocator));
  m_totalNumberOfChunks += numberOfChunks;
}

void MemoryManager::generateChunkManagementPool(
    BumpAllocator& managementAllocator) noexcept {
  m_denyAddMemPool = true;
  uint32_t chunkSize = sizeof(ChunkManagement);
  m_chunkManagementPool.emplace_back(
      std::make_unique<MemPool>(chunkSize, m_totalNumberOfChunks,
                                managementAllocator, managementAllocator));
}

uint32_t MemoryManager::getNumberOfMemPools() const noexcept {
  return static_cast<uint32_t>(m_memPoolVector.size());
}

MemPoolInfo MemoryManager::getMemPoolInfo(const uint32_t index) const noexcept {
  if (index >= m_memPoolVector.size()) {
    return {0, 0, 0, 0};
  }
  return m_memPoolVector[index]->getInfo();
}

uint32_t MemoryManager::sizeWithChunkHeaderStruct(
    const MaxChunkPayloadSize_t size) noexcept {
  return size + static_cast<uint32_t>(sizeof(ChunkHeader));
}

uint64_t MemoryManager::requiredChunkMemorySize(
    const Config& mePooConfig) noexcept {
  uint64_t memorySize{0};
  for (const auto& mempoolConfig : mePooConfig.m_mempoolConfig) {
    // for the required chunk memory size only the size of the ChunkHeader
    // and the the chunk-payload size is taken into account;
    // the user has the option to further partition the chunk-payload with
    // a user-header and therefore reduce the user-payload size
    memorySize += details::align(
        static_cast<uint64_t>(mempoolConfig.m_chunkCount) *
            MemoryManager::sizeWithChunkHeaderStruct(mempoolConfig.m_size),
        MemPool::CHUNK_MEMORY_ALIGNMENT);
  }
  return memorySize;
}

uint64_t MemoryManager::requiredManagementMemorySize(
    const Config& mePooConfig) noexcept {
  uint64_t memorySize{0U};
  uint64_t sumOfAllChunks{0U};
  for (const auto& mempool : mePooConfig.m_mempoolConfig) {
    sumOfAllChunks += mempool.m_chunkCount;
    memorySize += align(
        MemPool::freeList_t::requiredIndexMemorySize(mempool.m_chunkCount),
        MemPool::CHUNK_MEMORY_ALIGNMENT);
  }

  memorySize += align(sumOfAllChunks * sizeof(ChunkManagement),
                      MemPool::CHUNK_MEMORY_ALIGNMENT);
  memorySize +=
      align(MemPool::freeList_t::requiredIndexMemorySize(sumOfAllChunks),
            MemPool::CHUNK_MEMORY_ALIGNMENT);

  return memorySize;
}

uint64_t MemoryManager::requiredFullMemorySize(
    const Config& mePooConfig) noexcept {
  return requiredManagementMemorySize(mePooConfig) +
         requiredChunkMemorySize(mePooConfig);
}

void MemoryManager::configureMemoryManager(
    const Config& mePooConfig, BumpAllocator& managementAllocator,
    BumpAllocator& chunkMemoryAllocator) noexcept {
  for (auto entry : mePooConfig.m_mempoolConfig) {
    addMemPool(managementAllocator, chunkMemoryAllocator, entry.m_size,
               entry.m_chunkCount);
  }

  generateChunkManagementPool(managementAllocator);
}

tl::expected<SharedChunk, MemoryManager::Error> MemoryManager::getChunk(
    const ChunkSettings& chunkSettings) noexcept {
  void* chunk{nullptr};
  MemPool* memPoolPointer{nullptr};
  const auto requiredChunkSize = chunkSettings.requiredChunkSize();

  uint32_t aquiredChunkSize = 0U;

  for (auto& memPool : m_memPoolVector) {
    uint32_t chunkSizeOfMemPool = memPool->getChunkSize();
    if (chunkSizeOfMemPool >= requiredChunkSize) {
      chunk = memPool->getChunk();
      memPoolPointer = &(*memPool);
      aquiredChunkSize = chunkSizeOfMemPool;
      break;
    }
  }

  if (m_memPoolVector.size() == 0) {
    spdlog::error("There are no mempools available!");

    return tl::make_unexpected(Error::NO_MEMPOOLS_AVAILABLE);
  } else if (memPoolPointer == nullptr) {
    spdlog::error("The following mempools are available:");
    return tl::make_unexpected(Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE);
  } else if (chunk == nullptr) {
    spdlog::error(
        "MemoryManager: unable to acquire a chunk with a chunk-payload size "
        "of ");
    return tl::make_unexpected(Error::MEMPOOL_OUT_OF_CHUNKS);
  } else {
    auto chunkHeader = new (chunk) ChunkHeader(aquiredChunkSize, chunkSettings);
    auto chunkManagement =
        new (m_chunkManagementPool.front()->getChunk()) ChunkManagement(
            chunkHeader, memPoolPointer, &(*m_chunkManagementPool.front()));
    return SharedChunk(chunkManagement);
  }
}

std::ostream& operator<<(std::ostream& stream,
                         const MemoryManager::Error value) noexcept {
  stream << asStringLiteral(value);
  return stream;
}

}  // namespace memory
}  // namespace shm