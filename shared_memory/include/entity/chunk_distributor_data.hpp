#pragma once

#include <mutex>

#include "entity/port_policy.hpp"
#include "entity/shm_safe_unmanaged_chunk.hpp"
#include "shm/relative_pointer.hpp"

namespace shm {
namespace entity {

template <typename ChunkDistributorDataProperties, typename LockingPolicy,
          typename ChunkQueuePusherType>
struct ChunkDistributorData : public LockingPolicy {
  using ThisType_t = ChunkDistributorData<ChunkDistributorDataProperties,
                                          LockingPolicy, ChunkQueuePusherType>;
  using LockGuard_t = std::lock_guard<const ThisType_t>;
  using ChunkQueuePusher_t = ChunkQueuePusherType;
  using ChunkQueueData_t = typename ChunkQueuePusherType::MemberType_t;
  using ChunkDistributorDataProperties_t = ChunkDistributorDataProperties;

  ChunkDistributorData(const ConsumerTooSlowPolicy policy,
                       const uint64_t historyCapacity = 0u) noexcept;

  const uint64_t m_historyCapacity;

  using QueueContainer_t =
      std::vector<details::RelativePointer<ChunkQueueData_t>>;
  QueueContainer_t m_queues;

  /// @todo iox-#1710 If we would make the ChunkDistributor lock-free, can we
  /// than extend the UsedChunkList to be like a ring buffer and use this for
  /// the history? This would be needed to be able to safely cleanup. Using
  /// ShmSafeUnmanagedChunk since RouDi must access this list to cleanup the
  /// chunks in case of an application crash.
  using HistoryContainer_t = std::vector<ShmSafeUnmanagedChunk>;
  HistoryContainer_t m_history;
  const ConsumerTooSlowPolicy m_consumerTooSlowPolicy;
};

}  // namespace entity
}  // namespace shm