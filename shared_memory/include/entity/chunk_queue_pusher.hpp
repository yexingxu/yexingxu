#pragma once

#include "memory/shared_chunk.hpp"
#include "shm/algorithm.hpp"

namespace shm {
namespace entity {

/// @brief The ChunkQueuePusher is the low layer building block to push
/// SharedChunks in a chunk queue. Together with the ChunkDistributor and
/// ChunkQueuePopper the ChunkQueuePusher builds the infrastructure to exchange
/// memory chunks between different data producers and consumers that could be
/// located in different processes. A ChunkQueuePusher is the part of the chunk
/// queue that is knwon by the ChunkDistributor
template <typename ChunkQueueDataType>
class ChunkQueuePusher {
 public:
  using MemberType_t = ChunkQueueDataType;

  explicit ChunkQueuePusher(
      details::not_null<MemberType_t* const> chunkQueueDataPtr) noexcept;

  ChunkQueuePusher(const ChunkQueuePusher& other) = delete;
  ChunkQueuePusher& operator=(const ChunkQueuePusher&) = delete;
  ChunkQueuePusher(ChunkQueuePusher&& rhs) noexcept = default;
  ChunkQueuePusher& operator=(ChunkQueuePusher&& rhs) noexcept = default;
  ~ChunkQueuePusher() noexcept = default;

  /// @brief push a new chunk to the chunk queue
  /// @param[in] shared chunk object
  /// @return false if a queue overflow occurred, otherwise true
  bool push(memory::SharedChunk chunk) noexcept;

  /// @brief tell the queue that it lost a chunk (e.g. because push failed and
  /// there will be no retry)
  void lostAChunk() noexcept;

 protected:
  const MemberType_t* getMembers() const noexcept;
  MemberType_t* getMembers() noexcept;

 private:
  MemberType_t* m_chunkQueueDataPtr{nullptr};
};

}  // namespace entity
}  // namespace shm