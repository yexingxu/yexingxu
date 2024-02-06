#pragma once

#include "entity/chunk_queue_data.hpp"
#include "entity/used_chunk_list.hpp"
#include "shm/memory_info.hpp"

namespace shm {
namespace entity {

template <uint32_t MaxChunksHeldSimultaneously, typename ChunkQueueDataType>
struct ChunkReceiverData : public ChunkQueueDataType {
  explicit ChunkReceiverData(
      const VariantQueueTypes queueType, const QueueFullPolicy queueFullPolicy,
      const memory::MemoryInfo& memoryInfo = memory::MemoryInfo()) noexcept;

  using ChunkQueueData_t = ChunkQueueDataType;

  memory::MemoryInfo m_memoryInfo;

  /// we use one more than MaxChunksHeldSimultaneously for being able to provide
  /// one new chunk to the user if they already have the allowed
  /// MaxChunksHeldSimultaneously. But then the user has to return one to not
  /// brake the contract. This is aligned with AUTOSAR Adaptive ara::com
  static constexpr uint32_t MAX_CHUNKS_IN_USE =
      MaxChunksHeldSimultaneously + 1U;
  UsedChunkList<MAX_CHUNKS_IN_USE> m_chunksInUse;
};

}  // namespace entity
}  // namespace shm