#pragma once

#include <cstdint>

#include "entity/port_policy.hpp"
#include "entity/shm_safe_unmanaged_chunk.hpp"
#include "entity/used_chunk_list.hpp"
#include "memory/memory_manager.hpp"
#include "shm/memory_info.hpp"
#include "shm/relative_pointer.hpp"

namespace shm {
namespace entity {

template <uint32_t MaxChunksAllocatedSimultaneously,
          typename ChunkDistributorDataType>
struct ChunkSenderData : public ChunkDistributorDataType {
  using SequenceNumber_t = std::uint64_t;
  explicit ChunkSenderData(
      details::not_null<memory::MemoryManager* const> memoryManager,
      const ConsumerTooSlowPolicy consumerTooSlowPolicy,
      const uint64_t historyCapacity = 0U,
      const memory::MemoryInfo& memoryInfo = memory::MemoryInfo()) noexcept;

  using ChunkDistributorData_t = ChunkDistributorDataType;

  const details::RelativePointer<memory::MemoryManager> m_memoryMgr;
  memory::MemoryInfo m_memoryInfo;
  UsedChunkList<MaxChunksAllocatedSimultaneously> m_chunksInUse;
  SequenceNumber_t m_sequenceNumber{0U};
  ShmSafeUnmanagedChunk m_lastChunkUnmanaged;
};

}  // namespace entity
}  // namespace shm