#include "memory/chunk_header.hpp"

#include "memory/memory_pool.hpp"
#include "shm/algorithm.hpp"
#include "shm/memory.hpp"
#include "shm/system_call.hpp"
#include "types/unique_port_id.hpp"

namespace shm {
namespace memory {

constexpr uint8_t ChunkHeader::CHUNK_HEADER_VERSION;

ChunkHeader::ChunkHeader(const uint32_t chunkSize,
                         const ChunkSettings& chunkSettings) noexcept
    : m_chunkSize(chunkSize),
      m_userHeaderSize(chunkSettings.userHeaderSize()),
      m_userPayloadSize(chunkSettings.userPayloadSize()),
      m_userPayloadAlignment(chunkSettings.userPayloadAlignment()) {
  static_assert(alignof(ChunkHeader) >= 8U,
                "All the calculations expect the ChunkHeader alignment to be "
                "at least 8!");
  static_assert(alignof(ChunkHeader) <= MemPool::CHUNK_MEMORY_ALIGNMENT,
                "The ChunkHeader must not exceed the alignment of the mempool "
                "chunks, which are aligned to "
                "'MemPool::CHUNK_MEMORY_ALIGNMENT'!");

  const auto userPayloadAlignment = chunkSettings.userPayloadAlignment();
  const auto userHeaderSize = chunkSettings.userHeaderSize();

  static_assert(
      std::is_same<
          UserPayloadOffset_t,
          std::remove_const<decltype(userPayloadAlignment)>::type>::value,
      "UserPayloadOffset_t and userPayloadAlignment must have same type in "
      "order to prevent an overflow for the "
      "user-payload offset calculation for extremely large alignments");

  // have a look at »User-Payload Offset Calculation« in chunk_header.md for
  // more details regarding the calculation
  if (userHeaderSize == 0U) {
    if (userPayloadAlignment <= alignof(ChunkHeader)) {
      // the most simple case with no user-header and the user-payload adjacent
      // to the ChunkHeader
      m_userPayloadOffset = sizeof(ChunkHeader);
    } else {
      // the second most simple case with no user-header but the user-payload
      // alignment exceeds the ChunkHeader alignment and is therefore not
      // necessarily adjacent
      uint64_t addressOfChunkHeader = reinterpret_cast<uint64_t>(this);
      uint64_t headerEndAddress = addressOfChunkHeader + sizeof(ChunkHeader);
      uint64_t alignedUserPayloadAddress = details::align(
          headerEndAddress, static_cast<uint64_t>(userPayloadAlignment));
      uint64_t offsetToUserPayload =
          alignedUserPayloadAddress - addressOfChunkHeader;
      // the cast is safe since userPayloadOffset and userPayloadAlignment have
      // the same type and since the alignment must be a power of 2, the max
      // alignment is about half of the max value the type can hold
      m_userPayloadOffset =
          static_cast<UserPayloadOffset_t>(offsetToUserPayload);

      // this is safe since the alignment of the user-payload is larger than the
      // one from the ChunkHeader
      // -> the user-payload is either adjacent and 'backOffset' is at the same
      // location as 'userPayloadOffset'
      //    or the user-payload is not adjacent and there is space of at least
      //    the alignment of ChunkHeader between the ChunkHeader and the
      //    user-payload
      auto addressOfBackOffset =
          alignedUserPayloadAddress - sizeof(UserPayloadOffset_t);
      auto backOffset =
          reinterpret_cast<UserPayloadOffset_t*>(addressOfBackOffset);
      *backOffset = m_userPayloadOffset;
    }
  } else {
    // currently there is no way to set the user-header id; this is just a
    // preparation for future functionality
    m_userHeaderId = UNKNOWN_USER_HEADER;

    // the most complex case with a user-header
    auto addressOfChunkHeader = reinterpret_cast<uint64_t>(this);
    uint64_t headerEndAddress =
        addressOfChunkHeader + sizeof(ChunkHeader) + userHeaderSize;
    uint64_t userPayloadOffsetAlignment = alignof(UserPayloadOffset_t);
    uint64_t anticipatedBackOffsetAddress =
        details::align(headerEndAddress, userPayloadOffsetAlignment);
    uint64_t unalignedUserPayloadAddress =
        anticipatedBackOffsetAddress + sizeof(UserPayloadOffset_t);
    uint64_t alignedUserPayloadAddress =
        details::align(unalignedUserPayloadAddress,
                       static_cast<uint64_t>(userPayloadAlignment));
    uint64_t offsetToUserPayload =
        alignedUserPayloadAddress - addressOfChunkHeader;
    // the cast is safe since userPayloadOffset and userPayloadAlignment have
    // the same type and since the alignment must be a power of 2, the max
    // alignment is about half of the max value the type can hold
    m_userPayloadOffset = static_cast<UserPayloadOffset_t>(offsetToUserPayload);

    // this always works if the alignment of UserPayloadOffset_t and
    // userPayloadAlignment are equal, if not there are two options:
    //   - either the alignment of the UserPayloadOffset_t is larger than
    //   userPayloadAlignment
    //     -> the userPayload is adjacent to the back-offset and therefore this
    //     also works
    //   - or the alignment of the UserPayloadOffset_t is smaller than
    //   userPayloadAlignment
    //     -> the back-offset can be put adjacent to to the Topic since the
    //     smaller alignment always fits
    auto backOffsetAddress =
        alignedUserPayloadAddress - sizeof(UserPayloadOffset_t);
    auto backOffset = reinterpret_cast<UserPayloadOffset_t*>(backOffsetAddress);
    *backOffset = m_userPayloadOffset;
  }

  ENSURES(overflowSafeUsedSizeOfChunk() <= chunkSize &&
          "Used size of chunk would exceed the actual chunk size!");
}

uint8_t ChunkHeader::chunkHeaderVersion() const noexcept {
  return m_chunkHeaderVersion;
}

uint16_t ChunkHeader::userHeaderId() const noexcept { return m_userHeaderId; }

void* ChunkHeader::userHeader() noexcept {
  if (m_userHeaderId == NO_USER_HEADER) {
    return nullptr;
  }
  // the UserHeader is always adjacent to the ChunkHeader
  return reinterpret_cast<void*>(reinterpret_cast<uint64_t>(this) +
                                 sizeof(ChunkHeader));
}

const void* ChunkHeader::userHeader() const noexcept {
  return const_cast<ChunkHeader*>(this)->userHeader();
}

void* ChunkHeader::userPayload() noexcept {
  // user-payload is always located relative to "this" in this way
  return reinterpret_cast<void*>(reinterpret_cast<uint64_t>(this) +
                                 m_userPayloadOffset);
}

const void* ChunkHeader::userPayload() const noexcept {
  return const_cast<ChunkHeader*>(this)->userPayload();
}

ChunkHeader* ChunkHeader::fromUserPayload(void* const userPayload) noexcept {
  if (userPayload == nullptr) {
    return nullptr;
  }
  uint64_t userPayloadAddress = reinterpret_cast<uint64_t>(userPayload);
  // the back-offset is always stored in front of the user-payload, no matter if
  // a user-header is used or not or if the user-payload has a custom alignment
  auto backOffset = reinterpret_cast<UserPayloadOffset_t*>(
      userPayloadAddress - sizeof(UserPayloadOffset_t));
  return reinterpret_cast<ChunkHeader*>(userPayloadAddress - *backOffset);
}

const ChunkHeader* ChunkHeader::fromUserPayload(
    const void* const userPayload) noexcept {
  return ChunkHeader::fromUserPayload(const_cast<void*>(userPayload));
}

ChunkHeader* ChunkHeader::fromUserHeader(void* const userHeader) noexcept {
  if (userHeader == nullptr) {
    return nullptr;
  }
  // the UserHeader is always adjacent to the ChunkHeader
  return reinterpret_cast<ChunkHeader*>(reinterpret_cast<uint64_t>(userHeader) -
                                        sizeof(ChunkHeader));
}

const ChunkHeader* ChunkHeader::fromUserHeader(
    const void* const userHeader) noexcept {
  return ChunkHeader::fromUserHeader(const_cast<void*>(userHeader));
}

uint32_t ChunkHeader::usedSizeOfChunk() const noexcept {
  return static_cast<uint32_t>(overflowSafeUsedSizeOfChunk());
}

uint32_t ChunkHeader::chunkSize() const noexcept { return m_chunkSize; }

uint32_t ChunkHeader::userHeaderSize() const noexcept {
  return m_userHeaderSize;
}

uint32_t ChunkHeader::userPayloadSize() const noexcept {
  return m_userPayloadSize;
}

uint32_t ChunkHeader::userPayloadAlignment() const noexcept {
  return m_userPayloadAlignment;
}

types::UniquePortId ChunkHeader::originId() const noexcept {
  return m_originId;
}

void ChunkHeader::setOriginId(const types::UniquePortId originId) noexcept {
  m_originId = originId;
}

uint64_t ChunkHeader::sequenceNumber() const noexcept {
  return m_sequenceNumber;
}

void ChunkHeader::setSequenceNumber(const uint64_t sequenceNumber) noexcept {
  m_sequenceNumber = sequenceNumber;
}

uint64_t ChunkHeader::overflowSafeUsedSizeOfChunk() const noexcept {
  return static_cast<uint64_t>(m_userPayloadOffset) +
         static_cast<uint64_t>(m_userPayloadSize);
}

}  // namespace memory
}  // namespace shm