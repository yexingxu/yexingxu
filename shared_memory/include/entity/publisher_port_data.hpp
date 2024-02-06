
#pragma once

#include <cstdint>

#include "entity/base_port_data.hpp"
#include "entity/chunk_distributor_data.hpp"
#include "entity/chunk_queue_pusher.hpp"
#include "entity/chunk_sender_data.hpp"
#include "entity/port_data_types.hpp"
#include "entity/port_policy.hpp"
#include "memory/memory_manager.hpp"
#include "shm/memory_info.hpp"

namespace shm {
namespace entity {

struct PublisherOptions {
  /// @brief The size of the history chunk queue
  uint64_t historyCapacity{0U};

  /// @brief The name of the node where the publisher should belong to
  NodeName_t nodeName{""};

  /// @brief The option whether the publisher should already be offered when
  /// creating it
  bool offerOnCreate{true};

  /// @brief The option whether the publisher should block when the subscriber
  /// queue is full
  ConsumerTooSlowPolicy subscriberTooSlowPolicy{
      ConsumerTooSlowPolicy::DISCARD_OLDEST_DATA};

  //   /// @brief serialization of the PublisherOptions
  //   Serialization serialize() const noexcept;
  //   /// @brief deserialization of the PublisherOptions
  //   static expected<PublisherOptions, Serialization::Error> deserialize(
  //       const Serialization& serialized) noexcept;
};

struct PublisherPortData : public BasePortData {
  static constexpr std::uint32_t
      MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY = 8U;
  PublisherPortData(
      const ServiceDescription& serviceDescription,
      const RuntimeName_t& runtimeName,
      memory::MemoryManager* const memoryManager,
      const PublisherOptions& publisherOptions,
      const memory::MemoryInfo& memoryInfo = memory::MemoryInfo()) noexcept;

  using ChunkDistributorData_t =
      ChunkDistributorData<DefaultChunkDistributorConfig, ThreadSafePolicy,
                           ChunkQueuePusher<ChunkQueueData_t>>;
  using ChunkSenderData_t =
      ChunkSenderData<MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY,
                      ChunkDistributorData_t>;

  ChunkSenderData_t m_chunkSenderData;

  PublisherOptions m_options;

  std::atomic_bool m_offeringRequested{false};
  std::atomic_bool m_offered{false};
};

}  // namespace entity
}  // namespace shm