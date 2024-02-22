#pragma once

#include "memory/chunk_distributor.hpp"
#include "memory/chunk_header.hpp"
#include "memory/memory_manager.hpp"
#include "types/expected.hpp"
#include "types/unique_port_id.hpp"

namespace shm {
namespace memory {

enum class AllocationError {
  UNDEFINED_ERROR,
  NO_MEMPOOLS_AVAILABLE,
  RUNNING_OUT_OF_CHUNKS,
  TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL,
  INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER,
  INVALID_PARAMETER_FOR_REQUEST_HEADER,
};

// template <>
// constexpr AllocationError from<details::MemoryManager::Error,
// AllocationError>(
//     const details::MemoryManager::Error error) noexcept;

/// @brief Converts the AllocationError to a string literal
/// @param[in] value to convert to a string literal
/// @return pointer to a string literal
inline constexpr const char* asStringLiteral(
    const AllocationError value) noexcept;

/// @brief Convenience stream operator to easily use the 'asStringLiteral'
/// function with std::ostream
/// @param[in] stream sink to write the message to
/// @param[in] value to convert to a string literal
/// @return the reference to 'stream' which was provided as input parameter
inline std::ostream& operator<<(std::ostream& stream,
                                AllocationError value) noexcept;

/// @brief The ChunkSender is a building block of the shared memory
/// communication infrastructure. It extends the functionality of a
/// ChunkDistributor with the abililty to allocate and free memory chunks. For
/// getting chunks of memory the MemoryManger is used. Together with the
/// ChunkReceiver, they are the next abstraction layer on top of
/// ChunkDistributor and ChunkQueuePopper. The ChunkSender holds the ownership
/// of the SharedChunks and does a bookkeeping which chunks are currently passed
/// to the user side.
template <typename ChunkSenderDataType>
class ChunkSender : public ChunkDistributor<
                        typename ChunkSenderDataType::ChunkDistributorData_t> {
 public:
  using MemberType_t = ChunkSenderDataType;
  using Base_t =
      ChunkDistributor<typename ChunkSenderDataType::ChunkDistributorData_t>;

  explicit ChunkSender(
      details::not_null<MemberType_t* const> chunkSenderDataPtr) noexcept;

  ChunkSender(const ChunkSender& other) = delete;
  ChunkSender& operator=(const ChunkSender&) = delete;
  ChunkSender(ChunkSender&& rhs) noexcept = default;
  ChunkSender& operator=(ChunkSender&& rhs) noexcept = default;
  ~ChunkSender() noexcept = default;

  /// @brief allocate a chunk, the ownership of the SharedChunk remains in the
  /// ChunkSender for being able to cleanup if the user process disappears
  /// @param[in] originId, the unique id of the entity which requested this
  /// allocate
  /// @param[in] userPayloadSize, size of the user-payload without additional
  /// headers
  /// @param[in] userPayloadAlignment, alignment of the user-payload
  /// @param[in] userHeaderSize, size of the user-header; use
  /// iox::CHUNK_NO_USER_HEADER_SIZE to omit a user-header
  /// @param[in] userHeaderAlignment, alignment of the user-header; use
  /// iox::CHUNK_NO_USER_HEADER_ALIGNMENT to omit a user-header
  /// @return on success pointer to a ChunkHeader which can be used to access
  /// the chunk-header, user-header and user-payload fields, error if not
  tl::expected<ChunkHeader*, AllocationError> tryAllocate(
      const types::UniquePortId originId, const uint32_t userPayloadSize,
      const uint32_t userPayloadAlignment, const uint32_t userHeaderSize,
      const uint32_t userHeaderAlignment) noexcept;

  /// @brief Release an allocated chunk without sending it
  /// @param[in] chunkHeader, pointer to the ChunkHeader to release
  void release(const ChunkHeader* const chunkHeader) noexcept;

  /// @brief Send an allocated chunk to all connected ChunkQueuePopper
  /// @param[in] chunkHeader, pointer to the ChunkHeader to send; the ownership
  /// of the pointer is transferred to this method
  /// @return the number of receiver the chunk was send to
  uint64_t send(ChunkHeader* const chunkHeader) noexcept;

  /// @brief Send an allocated chunk to a specific ChunkQueuePopper
  /// @param[in] chunkHeader, pointer to the ChunkHeader to send; the ownership
  /// of the pointer is transferred to this method
  /// @param[in] uniqueQueueId is an unique ID which identifies the queue to
  /// which this chunk shall be delivered
  /// @param[in] lastKnownQueueIndex is used for a fast lookup of the queue with
  /// uniqueQueueId
  /// @return true when successful, false otherwise
  /// @note This method does not add the chunk to the history
  bool sendToQueue(ChunkHeader* const chunkHeader,
                   const types::UniqueId uniqueQueueId,
                   const uint32_t lastKnownQueueIndex) noexcept;

  /// @brief Push an allocated chunk to the history without sending it
  /// @param[in] chunkHeader, pointer to the ChunkHeader to push to the history
  void pushToHistory(ChunkHeader* const chunkHeader) noexcept;

  /// @brief Returns the last sent chunk if there is one
  /// @return pointer to the ChunkHeader of the last sent Chunk if there is one,
  /// empty optional if not
  tl::optional<const ChunkHeader*> tryGetPreviousChunk() const noexcept;

  /// @brief Release all the chunks that are currently held. Caution: Only call
  /// this if the user process is no more running E.g. This cleans up chunks
  /// that were held by a user process that died unexpectetly, for avoiding lost
  /// chunks in the system
  void releaseAll() noexcept;

 private:
  /// @brief Get the SharedChunk from the provided ChunkHeader and do all that
  /// is required to send the chunk
  /// @param[in] chunkHeader of the chunk that shall be send
  /// @param[in][out] chunk that corresponds to the chunk header
  /// @return true if there was a matching chunk with this header, false if not
  bool getChunkReadyForSend(const ChunkHeader* const chunkHeader,
                            SharedChunk& chunk) noexcept;

  const MemberType_t* getMembers() const noexcept;
  MemberType_t* getMembers() noexcept;
};

// template <>
// constexpr AllocationError from<memory::MemoryManager::Error,
// AllocationError>(
//     const memory::MemoryManager::Error error) noexcept {
//   switch (error) {
//     case memory::MemoryManager::Error::NO_MEMPOOLS_AVAILABLE:
//       return AllocationError::NO_MEMPOOLS_AVAILABLE;
//     case memory::MemoryManager::Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE:
//       return AllocationError::NO_MEMPOOLS_AVAILABLE;
//     case memory::MemoryManager::Error::MEMPOOL_OUT_OF_CHUNKS:
//       return AllocationError::RUNNING_OUT_OF_CHUNKS;
//   }
//   return AllocationError::UNDEFINED_ERROR;
// }

inline constexpr const char* asStringLiteral(
    const AllocationError value) noexcept {
  switch (value) {
    case AllocationError::UNDEFINED_ERROR:
      return "AllocationError::UNDEFINED_ERROR";
    case AllocationError::NO_MEMPOOLS_AVAILABLE:
      return "AllocationError::NO_MEMPOOLS_AVAILABLE";
    case AllocationError::RUNNING_OUT_OF_CHUNKS:
      return "AllocationError::RUNNING_OUT_OF_CHUNKS";
    case AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL:
      return "AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL";
    case AllocationError::INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER:
      return "AllocationError::INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_"
             "HEADER";
    case AllocationError::INVALID_PARAMETER_FOR_REQUEST_HEADER:
      return "AllocationError::INVALID_PARAMETER_FOR_REQUEST_HEADER";
    default:
      return "[Undefined AllocationError]";
  }

  return "[Undefined AllocationError]";
}

inline std::ostream& operator<<(std::ostream& stream,
                                AllocationError value) noexcept {
  stream << asStringLiteral(value);
  return stream;
}

template <typename ChunkSenderDataType>
inline ChunkSender<ChunkSenderDataType>::ChunkSender(
    not_null<MemberType_t* const> chunkSenderDataPtr) noexcept
    : Base_t(static_cast<
             typename ChunkSenderDataType::ChunkDistributorData_t* const>(
          chunkSenderDataPtr)) {}

template <typename ChunkSenderDataType>
inline const typename ChunkSender<ChunkSenderDataType>::MemberType_t*
ChunkSender<ChunkSenderDataType>::getMembers() const noexcept {
  return reinterpret_cast<const MemberType_t*>(Base_t::getMembers());
}

template <typename ChunkSenderDataType>
inline typename ChunkSender<ChunkSenderDataType>::MemberType_t*
ChunkSender<ChunkSenderDataType>::getMembers() noexcept {
  return reinterpret_cast<MemberType_t*>(Base_t::getMembers());
}

template <typename ChunkSenderDataType>
inline tl::expected<memory::ChunkHeader*, AllocationError>
ChunkSender<ChunkSenderDataType>::tryAllocate(
    const types::UniquePortId originId, const uint32_t userPayloadSize,
    const uint32_t userPayloadAlignment, const uint32_t userHeaderSize,
    const uint32_t userHeaderAlignment) noexcept {
  // use the chunk stored in m_lastChunkUnmanaged if:
  //   - there is a valid chunk
  //   - there is no other owner
  //   - the new user-payload still fits in it
  const auto chunkSettingsResult =
      memory::ChunkSettings::create(userPayloadSize, userPayloadAlignment,
                                    userHeaderSize, userHeaderAlignment);
  if (!chunkSettingsResult.has_value()) {
    return tl::make_unexpected(
        AllocationError::INVALID_PARAMETER_FOR_USER_PAYLOAD_OR_USER_HEADER);
  }

  const auto& chunkSettings = chunkSettingsResult.value();
  const uint32_t requiredChunkSize = chunkSettings.requiredChunkSize();

  auto& lastChunkUnmanaged = getMembers()->m_lastChunkUnmanaged;
  memory::ChunkHeader* lastChunkChunkHeader =
      lastChunkUnmanaged.isNotLogicalNullptrAndHasNoOtherOwners()
          ? lastChunkUnmanaged.getChunkHeader()
          : nullptr;

  if (lastChunkChunkHeader &&
      (lastChunkChunkHeader->chunkSize() >= requiredChunkSize)) {
    auto sharedChunk = lastChunkUnmanaged.cloneToSharedChunk();
    if (getMembers()->m_chunksInUse.insert(sharedChunk)) {
      auto chunkSize = lastChunkChunkHeader->chunkSize();
      lastChunkChunkHeader->~ChunkHeader();
      new (lastChunkChunkHeader) memory::ChunkHeader(chunkSize, chunkSettings);
      lastChunkChunkHeader->setOriginId(originId);
      return lastChunkChunkHeader;
    } else {
      return tl::make_unexpected(
          AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL);
    }
  } else {
    // BEGIN of critical section, chunk will be lost if the process terminates
    // in this section get a new chunk
    auto getChunkResult = getMembers()->m_memoryMgr->getChunk(chunkSettings);

    if (!getChunkResult.has_value()) {
      // TODO
      /// @todo iox-#1012 use error<E2>::from(E1); once available
      //   return err(into<AllocationError>(getChunkResult.error()));
    }

    auto& chunk = getChunkResult.value();

    // if the application allocated too much chunks, return no more chunks
    if (getMembers()->m_chunksInUse.insert(chunk)) {
      // END of critical section
      chunk.getChunkHeader()->setOriginId(originId);
      return chunk.getChunkHeader();
    } else {
      // release the allocated chunk
      chunk = nullptr;
      return tl::make_unexpected(
          AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL);
    }
  }
}

template <typename ChunkSenderDataType>
inline void ChunkSender<ChunkSenderDataType>::release(
    const memory::ChunkHeader* const chunkHeader) noexcept {
  memory::SharedChunk chunk(nullptr);
  // d'tor of SharedChunk will release the memory, we do not have to touch the
  // returned chunk
  if (!getMembers()->m_chunksInUse.remove(chunkHeader, chunk)) {
    // TODO
    // errorHandler(PoshError::POPO__CHUNK_SENDER_INVALID_CHUNK_TO_FREE_FROM_USER,
    //              ErrorLevel::SEVERE);
  }
}

template <typename ChunkSenderDataType>
inline uint64_t ChunkSender<ChunkSenderDataType>::send(
    memory::ChunkHeader* const chunkHeader) noexcept {
  uint64_t numberOfReceiverTheChunkWasDelivered{0};
  memory::SharedChunk chunk(nullptr);
  // BEGIN of critical section, chunk will be lost if the process terminates in
  // this section
  if (getChunkReadyForSend(chunkHeader, chunk)) {
    numberOfReceiverTheChunkWasDelivered =
        this->deliverToAllStoredQueues(chunk);

    getMembers()->m_lastChunkUnmanaged.releaseToSharedChunk();
    getMembers()->m_lastChunkUnmanaged = chunk;
  }
  // END of critical section

  return numberOfReceiverTheChunkWasDelivered;
}

template <typename ChunkSenderDataType>
inline bool ChunkSender<ChunkSenderDataType>::sendToQueue(
    memory::ChunkHeader* const chunkHeader, const types::UniqueId uniqueQueueId,
    const uint32_t lastKnownQueueIndex) noexcept {
  memory::SharedChunk chunk(nullptr);
  // BEGIN of critical section, chunk will be lost if the process terminates in
  // this section
  if (getChunkReadyForSend(chunkHeader, chunk)) {
    auto deliveryResult =
        this->deliverToQueue(uniqueQueueId, lastKnownQueueIndex, chunk);

    getMembers()->m_lastChunkUnmanaged.releaseToSharedChunk();
    getMembers()->m_lastChunkUnmanaged = chunk;

    return !deliveryResult.has_error();
  }
  // END of critical section

  return false;
}

template <typename ChunkSenderDataType>
inline void ChunkSender<ChunkSenderDataType>::pushToHistory(
    memory::ChunkHeader* const chunkHeader) noexcept {
  memory::SharedChunk chunk(nullptr);
  // BEGIN of critical section, chunk will be lost if the process terminates in
  // this section
  if (getChunkReadyForSend(chunkHeader, chunk)) {
    this->addToHistoryWithoutDelivery(chunk);

    getMembers()->m_lastChunkUnmanaged.releaseToSharedChunk();
    getMembers()->m_lastChunkUnmanaged = chunk;
  }
  // END of critical section
}

template <typename ChunkSenderDataType>
inline tl::optional<const memory::ChunkHeader*>
ChunkSender<ChunkSenderDataType>::tryGetPreviousChunk() const noexcept {
  if (!getMembers()->m_lastChunkUnmanaged.isLogicalNullptr()) {
    return getMembers()->m_lastChunkUnmanaged.getChunkHeader();
  } else {
    return tl::nullopt;
  }
}

template <typename ChunkSenderDataType>
inline void ChunkSender<ChunkSenderDataType>::releaseAll() noexcept {
  getMembers()->m_chunksInUse.cleanup();
  this->cleanup();
  getMembers()->m_lastChunkUnmanaged.releaseToSharedChunk();
}

template <typename ChunkSenderDataType>
inline bool ChunkSender<ChunkSenderDataType>::getChunkReadyForSend(
    const memory::ChunkHeader* const chunkHeader,
    memory::SharedChunk& chunk) noexcept {
  if (getMembers()->m_chunksInUse.remove(chunkHeader, chunk)) {
    chunk.getChunkHeader()->setSequenceNumber(getMembers()->m_sequenceNumber++);
    return true;
  } else {
    // TODO
    // errorHandler(PoshError::POPO__CHUNK_SENDER_INVALID_CHUNK_TO_SEND_FROM_USER,
    //              ErrorLevel::SEVERE);
    return false;
  }
}

}  // namespace memory
}  // namespace shm