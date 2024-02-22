

#pragma once

#include "entity/port_policy.hpp"
#include "memory/shared_chunk.hpp"
#include "shm/algorithm.hpp"
#include "types/expected.hpp"
#include "types/optional.hpp"
#include "types/unique_id.hpp"

namespace shm {
namespace memory {

enum class ChunkDistributorError {
  QUEUE_CONTAINER_OVERFLOW,
  QUEUE_NOT_IN_CONTAINER
};

/// @brief The ChunkDistributor is the low layer building block to send
/// SharedChunks to a dynamic number of ChunkQueus. Together with the
/// ChunkQueuePusher, the ChunkDistributor builds the infrastructure to exchange
/// memory chunks between different data producers and consumers that could be
/// located in different processes. Besides a modifiable container of
/// ChunkQueues to which a SharedChunk can be deliverd, it holds a configurable
/// history of last sent chunks. This allows to provide a newly added queue a
/// number of last chunks to start from. This is needed for functionality known
/// as latched topic in ROS or field in ara::com. A ChunkDistributor is used to
/// build elements of higher abstraction layers that also do memory managemet
/// and provide an API towards the real user
///
/// About Concurrency:
/// This ChunkDistributor can be used with different LockingPolicies for
/// different scenarios When different threads operate on it (e.g. application
/// sends chunks and RouDi adds and removes queues), a locking policy must be
/// used that ensures consistent data in the ChunkDistributorData.
/// @todo iox-#1713 There are currently some challenges:
/// For the stored queues and the history, containers are used which are not
/// thread safe. Therefore we use an inter-process mutex. But this can lead to
/// deadlocks if a user process gets terminated while one of its threads is in
/// the ChunkDistributor and holds a lock. An easier setup would be if changing
/// the queues by a middleware thread and sending chunks by the user process
/// would not interleave. I.e. there is no concurrent access to the containers.
/// Then a memory synchronization would be sufficient. The cleanup() call is the
/// biggest challenge. This is used to free chunks that are still held by a not
/// properly terminated user application. Even if access from middleware and
/// user threads do not overlap, the history container to cleanup could be in an
/// inconsistent state as the application was hard terminated while changing it.
/// We would need a container like the UsedChunkList to have one that is robust
/// against such inconsistencies.... A perfect job for our future selves
template <typename ChunkDistributorDataType>
class ChunkDistributor {
 public:
  using MemberType_t = ChunkDistributorDataType;
  using ChunkQueueData_t = typename ChunkDistributorDataType::ChunkQueueData_t;
  using ChunkQueuePusher_t =
      typename ChunkDistributorDataType::ChunkQueuePusher_t;

  explicit ChunkDistributor(
      details::not_null<MemberType_t* const> chunkDistrubutorDataPtr) noexcept;

  ChunkDistributor(const ChunkDistributor& other) = delete;
  ChunkDistributor& operator=(const ChunkDistributor&) = delete;
  ChunkDistributor(ChunkDistributor&& rhs) noexcept = default;
  ChunkDistributor& operator=(ChunkDistributor&& rhs) noexcept = default;
  virtual ~ChunkDistributor() noexcept = default;

  /// @brief Add a queue to the internal list of chunk queues to which chunks
  /// are delivered when calling deliverToAllStoredQueues
  /// @param[in] queueToAdd chunk queue to add to the list
  /// @param[in] requestedHistory number of last chunks from history to send if
  /// available. If history size is smaller then the available history size
  /// chunks are provided
  /// @return if the queue could be added it returns success, otherwiese a
  /// ChunkDistributor error
  tl::expected<void, ChunkDistributorError> tryAddQueue(
      details::not_null<ChunkQueueData_t* const> queueToAdd,
      const uint64_t requestedHistory = 0U) noexcept;

  /// @brief Remove a queue from the internal list of chunk queues
  /// @param[in] queueToRemove is the queue to remove from the list
  /// @return if the queue could be removed it returns success, otherwiese a
  /// ChunkDistributor error
  tl::expected<void, ChunkDistributorError> tryRemoveQueue(
      details::not_null<ChunkQueueData_t* const> queueToRemove) noexcept;

  /// @brief Delete all the stored chunk queues
  void removeAllQueues() noexcept;

  /// @brief Get the information whether there are any stored chunk queues
  /// @return true if there are stored chunk queues, false if not
  bool hasStoredQueues() const noexcept;

  /// @brief Deliver the provided shared chunk to all the stored chunk queues.
  /// The chunk will be added to the chunk history
  /// @param[in] chunk is the SharedChunk to be delivered
  /// @return the number of queues the chunk was delivered to
  uint64_t deliverToAllStoredQueues(SharedChunk chunk) noexcept;

  /// @brief Deliver the provided shared chunk to the chunk queue with the
  /// provided ID. The chunk will NOT be added to the chunk history
  /// @param[in] uniqueQueueId is an unique ID which identifies the queue to
  /// which this chunk shall be delivered
  /// @param[in] lastKnownQueueIndex is used for a fast lookup of the queue with
  /// uniqueQueueId
  /// @param[in] chunk is the SharedChunk to be delivered
  /// @return ChunkDistributorError if the queue was not found
  tl::expected<void, ChunkDistributorError> deliverToQueue(
      const types::UniqueId uniqueQueueId, const uint32_t lastKnownQueueIndex,
      SharedChunk chunk) noexcept;

  /// @brief Lookup for the index of a queue with a specific iox::UniqueId
  /// @param[in] uniqueQueueId is the unique ID of the queue to query the index
  /// @param[in] lastKnownQueueIndex is used for a fast lookup of the queue with
  /// uniqueQueueId; if the queue is not found at the index, the queue is
  /// searched by iteration over all stored queues
  /// @return the index of the queue with uniqueQueueId or nullopt if the queue
  /// was not found
  tl::optional<uint32_t> getQueueIndex(
      const types::UniqueId uniqueQueueId,
      const uint32_t lastKnownQueueIndex) const noexcept;

  /// @brief Update the chunk history but do not deliver the chunk to any chunk
  /// queue. E.g. use case is to to update a non offered field in ara
  /// @param[in] chunk to add to the chunk history
  void addToHistoryWithoutDelivery(SharedChunk chunk) noexcept;

  /// @brief Get the current size of the chunk history
  /// @return chunk history size
  uint64_t getHistorySize() noexcept;

  /// @brief Get the capacity of the chunk history
  /// @return chunk history capacity
  uint64_t getHistoryCapacity() const noexcept;

  /// @brief Clears the chunk history
  void clearHistory() noexcept;

  /// @brief cleanup the used shrared memory chunks
  void cleanup() noexcept;

 protected:
  const MemberType_t* getMembers() const noexcept;
  MemberType_t* getMembers() noexcept;

  bool pushToQueue(details::not_null<ChunkQueueData_t* const> queue,
                   SharedChunk chunk) noexcept;

 private:
  MemberType_t* m_chunkDistrubutorDataPtr{nullptr};
};

template <typename ChunkDistributorDataType>
inline ChunkDistributor<ChunkDistributorDataType>::ChunkDistributor(
    not_null<MemberType_t* const> chunkDistrubutorDataPtr) noexcept
    : m_chunkDistrubutorDataPtr(chunkDistrubutorDataPtr) {}

template <typename ChunkDistributorDataType>
inline const typename ChunkDistributor<ChunkDistributorDataType>::MemberType_t*
ChunkDistributor<ChunkDistributorDataType>::getMembers() const noexcept {
  return m_chunkDistrubutorDataPtr;
}

template <typename ChunkDistributorDataType>
inline typename ChunkDistributor<ChunkDistributorDataType>::MemberType_t*
ChunkDistributor<ChunkDistributorDataType>::getMembers() noexcept {
  return m_chunkDistrubutorDataPtr;
}

template <typename ChunkDistributorDataType>
inline tl::expected<void, ChunkDistributorError>
ChunkDistributor<ChunkDistributorDataType>::tryAddQueue(
    not_null<ChunkQueueData_t* const> queueToAdd,
    const uint64_t requestedHistory) noexcept {
  typename MemberType_t::LockGuard_t lock(*getMembers());

  const auto alreadyKnownReceiver =
      std::find_if(getMembers()->m_queues.begin(), getMembers()->m_queues.end(),
                   [&](const RelativePointer<ChunkQueueData_t> queue) {
                     return queue.get() == queueToAdd;
                   });

  // check if the queue is not already in the list
  if (alreadyKnownReceiver == getMembers()->m_queues.end()) {
    if (getMembers()->m_queues.size() < getMembers()->m_queues.capacity()) {
      // AXIVION Next Construct AutosarC++19_03-A0.1.2, AutosarC++19_03-M0-3-2 :
      // we checked the capacity, so pushing will be fine
      getMembers()->m_queues.push_back(
          RelativePointer<ChunkQueueData_t>(queueToAdd));

      const auto currChunkHistorySize = getMembers()->m_history.size();

      if (requestedHistory > getMembers()->m_historyCapacity) {
        // IOX_LOG(WARN,
        //         "Chunk history request exceeds history capacity! Request is "
        //             << requestedHistory << ". Capacity is "
        //             << getMembers()->m_historyCapacity << ".");
      }

      // if the current history is large enough we send the requested number of
      // chunks, else we send the total history
      const auto startIndex = (requestedHistory <= currChunkHistorySize)
                                  ? currChunkHistorySize - requestedHistory
                                  : 0u;
      for (auto i = startIndex; i < currChunkHistorySize; ++i) {
        pushToQueue(queueToAdd,
                    getMembers()->m_history[i].cloneToSharedChunk());
      }

      return {};
    } else {
      // that's not the fault of the chunk distributor user, we report a
      // moderate error and indicate that adding the queue was not possible
      // TODO
      //   errorHandler(
      //       PoshError::POPO__CHUNK_DISTRIBUTOR_OVERFLOW_OF_QUEUE_CONTAINER,
      //       ErrorLevel::MODERATE);

      return tl::make_unexpected(
          ChunkDistributorError::QUEUE_CONTAINER_OVERFLOW);
    }
  }

  return {};
}

template <typename ChunkDistributorDataType>
inline tl::expected<void, ChunkDistributorError>
ChunkDistributor<ChunkDistributorDataType>::tryRemoveQueue(
    not_null<ChunkQueueData_t* const> queueToRemove) noexcept {
  typename MemberType_t::LockGuard_t lock(*getMembers());

  const auto iter =
      std::find(getMembers()->m_queues.begin(), getMembers()->m_queues.end(),
                static_cast<ChunkQueueData_t* const>(queueToRemove));
  if (iter != getMembers()->m_queues.end()) {
    // AXIVION Next Construct AutosarC++19_03-A0.1.2 : we don't use iter any
    // longer so return value can be ignored
    getMembers()->m_queues.erase(iter);

    return {};
  } else {
    return tl::make_unexpected(ChunkDistributorError::QUEUE_NOT_IN_CONTAINER);
  }
}

template <typename ChunkDistributorDataType>
inline void
ChunkDistributor<ChunkDistributorDataType>::removeAllQueues() noexcept {
  typename MemberType_t::LockGuard_t lock(*getMembers());

  getMembers()->m_queues.clear();
}

template <typename ChunkDistributorDataType>
inline bool ChunkDistributor<ChunkDistributorDataType>::hasStoredQueues()
    const noexcept {
  typename MemberType_t::LockGuard_t lock(*getMembers());

  return !getMembers()->m_queues.empty();
}

template <typename ChunkDistributorDataType>
inline uint64_t
ChunkDistributor<ChunkDistributorDataType>::deliverToAllStoredQueues(
    memory::SharedChunk chunk) noexcept {
  uint64_t numberOfQueuesTheChunkWasDeliveredTo{0U};
  using QueueContainer = decltype(getMembers()->m_queues);
  QueueContainer fullQueuesAwaitingDelivery;
  {
    typename MemberType_t::LockGuard_t lock(*getMembers());

    bool willWaitForConsumer = getMembers()->m_consumerTooSlowPolicy ==
                               entity::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;
    // send to all the queues
    for (auto& queue : getMembers()->m_queues) {
      bool isBlockingQueue =
          (willWaitForConsumer &&
           queue->m_queueFullPolicy == entity::QueueFullPolicy::BLOCK_PRODUCER);

      if (pushToQueue(queue.get(), chunk)) {
        ++numberOfQueuesTheChunkWasDeliveredTo;
      } else {
        if (isBlockingQueue) {
          fullQueuesAwaitingDelivery.emplace_back(queue);
        } else {
          ++numberOfQueuesTheChunkWasDeliveredTo;
          ChunkQueuePusher_t(queue.get()).lostAChunk();
        }
      }
    }
  }

  // busy waiting until every queue is served
  //   detail::adaptive_wait adaptiveWait;
  while (!fullQueuesAwaitingDelivery.empty()) {
    // adaptiveWait.wait();
    {
      // create intersection of current queues and fullQueuesAwaitingDelivery
      // reason: it is possible that since the last iteration some subscriber
      // have already unsubscribed
      //          and without this intersection we would deliver to dead queues
      typename MemberType_t::LockGuard_t lock(*getMembers());
      QueueContainer remainingQueues;
      using QueueContainerValue = typename QueueContainer::value_type;
      auto greaterThan = [](QueueContainerValue& a,
                            QueueContainerValue& b) -> bool {
        return reinterpret_cast<uint64_t>(a.get()) >
               reinterpret_cast<uint64_t>(b.get());
      };
      std::sort(getMembers()->m_queues.begin(), getMembers()->m_queues.end(),
                greaterThan);

#if (defined(__GNUC__) && __GNUC__ == 13 && !defined(__clang__))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
      std::sort(fullQueuesAwaitingDelivery.begin(),
                fullQueuesAwaitingDelivery.end(), greaterThan);
#if (defined(__GNUC__) && __GNUC__ == 13 && !defined(__clang__))
#pragma GCC diagnostic pop
#endif

      std::set_intersection(
          getMembers()->m_queues.begin(), getMembers()->m_queues.end(),
          fullQueuesAwaitingDelivery.begin(), fullQueuesAwaitingDelivery.end(),
          std::back_inserter(remainingQueues), greaterThan);
      fullQueuesAwaitingDelivery.clear();

      // deliver to remaining queues
      for (auto& queue : remainingQueues) {
        if (pushToQueue(queue.get(), chunk)) {
          ++numberOfQueuesTheChunkWasDeliveredTo;
        } else {
          fullQueuesAwaitingDelivery.push_back(queue);
        }
      }
    }
  }

  addToHistoryWithoutDelivery(chunk);

  return numberOfQueuesTheChunkWasDeliveredTo;
}

template <typename ChunkDistributorDataType>
inline bool ChunkDistributor<ChunkDistributorDataType>::pushToQueue(
    not_null<ChunkQueueData_t* const> queue,
    memory::SharedChunk chunk) noexcept {
  return ChunkQueuePusher_t(queue).push(chunk);
}

template <typename ChunkDistributorDataType>
inline tl::expected<void, ChunkDistributorError>
ChunkDistributor<ChunkDistributorDataType>::deliverToQueue(
    const types::UniqueId uniqueQueueId, const uint32_t lastKnownQueueIndex,
    memory::SharedChunk chunk [[maybe_unused]]) noexcept {
  bool retry{false};
  do {
    typename MemberType_t::LockGuard_t lock(*getMembers());

    auto queueIndex = getQueueIndex(uniqueQueueId, lastKnownQueueIndex);

    if (!queueIndex.has_value()) {
      return tl::make_unexpected(ChunkDistributorError::QUEUE_NOT_IN_CONTAINER);
    }

    auto& queue = getMembers()->m_queues[queueIndex.value()];

    bool willWaitForConsumer = getMembers()->m_consumerTooSlowPolicy ==
                               entity::ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER;

    bool isBlockingQueue =
        (willWaitForConsumer &&
         queue->m_queueFullPolicy == entity::QueueFullPolicy::BLOCK_PRODUCER);

    retry = false;
    if (!pushToQueue(queue.get(), chunk)) {
      if (isBlockingQueue) {
        retry = true;
      } else {
        ChunkQueuePusher_t(queue.get()).lostAChunk();
      }
    }
  } while (retry);

  return {};
}

template <typename ChunkDistributorDataType>
inline tl::optional<uint32_t>
ChunkDistributor<ChunkDistributorDataType>::getQueueIndex(
    const types::UniqueId uniqueQueueId,
    const uint32_t lastKnownQueueIndex) const noexcept {
  typename MemberType_t::LockGuard_t lock(*getMembers());

  auto& queues = getMembers()->m_queues;

  if (queues.size() > lastKnownQueueIndex &&
      queues[lastKnownQueueIndex]->m_uniqueId == uniqueQueueId) {
    return lastKnownQueueIndex;
  }

  uint32_t index{0};
  for (auto& queue : queues) {
    if (queue->m_uniqueId == uniqueQueueId) {
      return index;
    }
    ++index;
  }
  return tl::nullopt;
}

template <typename ChunkDistributorDataType>
inline void
ChunkDistributor<ChunkDistributorDataType>::addToHistoryWithoutDelivery(
    memory::SharedChunk chunk) noexcept {
  typename MemberType_t::LockGuard_t lock(*getMembers());

  if (0u < getMembers()->m_historyCapacity) {
    if (getMembers()->m_history.size() >= getMembers()->m_historyCapacity) {
      auto chunkToRemove = getMembers()->m_history.begin();
      chunkToRemove->releaseToSharedChunk();
      // AXIVION Next Construct AutosarC++19_03-A0.1.2 : we are not iterating
      // here, so return value can be ignored
      getMembers()->m_history.erase(chunkToRemove);
    }
    // AXIVION Next Construct AutosarC++19_03-A0.1.2, AutosarC++19_03-M0-3-2 :
    // we ensured that there is space in the history, so return value can be
    // ignored
    getMembers()->m_history.push_back(chunk);
  }
}

template <typename ChunkDistributorDataType>
inline uint64_t
ChunkDistributor<ChunkDistributorDataType>::getHistorySize() noexcept {
  typename MemberType_t::LockGuard_t lock(*getMembers());

  return getMembers()->m_history.size();
}

template <typename ChunkDistributorDataType>
inline uint64_t ChunkDistributor<ChunkDistributorDataType>::getHistoryCapacity()
    const noexcept {
  return getMembers()->m_historyCapacity;
}

template <typename ChunkDistributorDataType>
inline void
ChunkDistributor<ChunkDistributorDataType>::clearHistory() noexcept {
  typename MemberType_t::LockGuard_t lock(*getMembers());

  for (auto& unmanagedChunk : getMembers()->m_history) {
    unmanagedChunk.releaseToSharedChunk();
  }

  getMembers()->m_history.clear();
}

template <typename ChunkDistributorDataType>
inline void ChunkDistributor<ChunkDistributorDataType>::cleanup() noexcept {
  if (getMembers()->tryLock()) {
    clearHistory();
    getMembers()->unlock();
  } else {
    /// @todo iox-#1711 currently we have a deadlock / mutex destroy
    /// vulnerability if the ThreadSafePolicy is used and a sending application
    /// dies when having the lock for sending. If the RouDi daemon wants to
    /// cleanup or does discovery changes we have a deadlock or an exception
    /// when destroying the mutex As long as we don't have a multi-threaded
    /// lock-free ChunkDistributor or another concept we die here
    // TODO
    // errorHandler(
    //     PoshError::
    //         POPO__CHUNK_DISTRIBUTOR_CLEANUP_DEADLOCK_BECAUSE_BAD_APPLICATION_TERMINATION,
    //     ErrorLevel::FATAL);
  }
}

}  // namespace memory
}  // namespace shm