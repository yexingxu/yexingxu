#pragma once

#include <cstdint>
namespace shm {
namespace entity {

/// @brief Used by producers how to adjust to slow consumer
enum class ConsumerTooSlowPolicy : uint8_t {
  /// Waits for the consumer it it's queue is full
  WAIT_FOR_CONSUMER,
  /// Discards the oldest data and pushes the newest one into the queue
  DISCARD_OLDEST_DATA
};

/// @brief Used by consumers to request a specific behavior from the producer
enum class QueueFullPolicy : uint8_t {
  /// Requests the producer to block when the consumer queue is full
  BLOCK_PRODUCER,
  /// Request to discard the oldest data and push the newest one into the queue
  DISCARD_OLDEST_DATA
};

}  // namespace entity
}  // namespace shm