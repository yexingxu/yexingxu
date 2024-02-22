#pragma once

#include "entity/condition_notifier.hpp"
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

template <typename ChunkQueueDataType>
inline ChunkQueuePusher<ChunkQueueDataType>::ChunkQueuePusher(
    details::not_null<MemberType_t* const> chunkQueueDataPtr) noexcept
    : m_chunkQueueDataPtr(chunkQueueDataPtr) {}

template <typename ChunkQueueDataType>
inline const typename ChunkQueuePusher<ChunkQueueDataType>::MemberType_t*
ChunkQueuePusher<ChunkQueueDataType>::getMembers() const noexcept {
  return m_chunkQueueDataPtr;
}

template <typename ChunkQueueDataType>
inline typename ChunkQueuePusher<ChunkQueueDataType>::MemberType_t*
ChunkQueuePusher<ChunkQueueDataType>::getMembers() noexcept {
  return m_chunkQueueDataPtr;
}

template <typename ChunkQueueDataType>
inline bool ChunkQueuePusher<ChunkQueueDataType>::push(
    memory::SharedChunk chunk) noexcept {
  auto pushRet = getMembers()->m_queue.push(chunk);
  bool hasQueueOverflow = false;

  // drop the chunk if one is returned by an overflow
  if (pushRet.has_value()) {
    pushRet.value().releaseToSharedChunk();
    // tell the ChunkDistributor that we had an overflow and dropped a sample
    hasQueueOverflow = true;
  }

  {
    typename MemberType_t::LockGuard_t lock(*getMembers());
    if (getMembers()->m_conditionVariableDataPtr) {
      ConditionNotifier(*getMembers()->m_conditionVariableDataPtr.get(),
                        *getMembers()->m_conditionVariableNotificationIndex)
          .notify();
    }
  }

  return !hasQueueOverflow;
}

template <typename ChunkQueueDataType>
inline void ChunkQueuePusher<ChunkQueueDataType>::lostAChunk() noexcept {
  getMembers()->m_queueHasLostChunks.store(true, std::memory_order_relaxed);
}

}  // namespace entity
}  // namespace shm