#pragma once

#include <mutex>

#include "entity/condition_variable_data.hpp"
#include "entity/port_policy.hpp"
#include "entity/shm_safe_unmanaged_chunk.hpp"
#include "entity/varaint_queue.hpp"
#include "shm/relative_pointer.hpp"
#include "types/optional.hpp"
#include "types/unique_id.hpp"

namespace shm {
namespace entity {

template <typename ChunkQueueDataProperties, typename LockingPolicy>
struct ChunkQueueData : public LockingPolicy {
  using ThisType_t = ChunkQueueData<ChunkQueueDataProperties, LockingPolicy>;
  using LockGuard_t = std::lock_guard<const ThisType_t>;
  using ChunkQueueDataProperties_t = ChunkQueueDataProperties;

  ChunkQueueData(const QueueFullPolicy policy,
                 const VariantQueueTypes queueType) noexcept;

  types::UniqueId m_uniqueId{};

  static constexpr uint64_t MAX_CAPACITY =
      ChunkQueueDataProperties_t::MAX_QUEUE_CAPACITY;
  VariantQueue<ShmSafeUnmanagedChunk, MAX_CAPACITY> m_queue;
  std::atomic_bool m_queueHasLostChunks{false};

  details::RelativePointer<ConditionVariableData> m_conditionVariableDataPtr;
  tl::optional<uint64_t> m_conditionVariableNotificationIndex;
  const QueueFullPolicy m_queueFullPolicy;
};

}  // namespace entity
}  // namespace shm