

#pragma once

#include <cstdint>

#include "entity/chunk_queue_data.hpp"
#include "entity/chunk_receiver_data.hpp"
#include "entity/locking_policy.hpp"
namespace shm {
namespace entity {

struct DefaultChunkQueueConfig {
  static constexpr uint64_t MAX_QUEUE_CAPACITY = 256U;
};

struct DefaultChunkDistributorConfig {
  static constexpr uint32_t MAX_QUEUES = 256U;
  static constexpr uint64_t MAX_HISTORY_CAPACITY = 16U;
};

static constexpr std::uint32_t MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY =
    256U;

using ChunkQueueData_t =
    ChunkQueueData<DefaultChunkQueueConfig, ThreadSafePolicy>;

using SubscriberChunkReceiverData_t =
    ChunkReceiverData<MAX_CHUNKS_HELD_PER_SUBSCRIBER_SIMULTANEOUSLY,
                      ChunkQueueData_t>;

}  // namespace entity
}  // namespace shm