#pragma once

#include "memory/memory_pool.hpp"
#include "shm/algorithm.hpp"
#include "shm/relative_pointer.hpp"

namespace shm {
namespace memory {

class MemPool;
struct ChunkHeader;

struct ChunkManagement {
  using base_t = ChunkHeader;
  using referenceCounterBase_t = uint64_t;
  using referenceCounter_t = std::atomic<referenceCounterBase_t>;

  ChunkManagement(
      const details::not_null<base_t*> chunkHeader,
      const details::not_null<MemPool*> mempool,
      const details::not_null<MemPool*> chunkManagementPool) noexcept
      : m_chunkHeader(chunkHeader),
        m_mempool(mempool),
        m_chunkManagementPool(chunkManagementPool) {
    static_assert(
        alignof(ChunkManagement) <= MemPool::CHUNK_MEMORY_ALIGNMENT,
        "The ChunkManagement must not exceed the alignment of the mempool "
        "chunks, which are aligned to "
        "'MemPool::CHUNK_MEMORY_ALIGNMENT'!");
  }

  details::RelativePointer<base_t> m_chunkHeader;
  referenceCounter_t m_referenceCounter{1U};

  details::RelativePointer<MemPool> m_mempool;
  details::RelativePointer<MemPool> m_chunkManagementPool;
};

}  // namespace memory
}  // namespace shm