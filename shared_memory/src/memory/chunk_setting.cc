

#include "memory/chunk_setting.hpp"

#include <limits>

#include "memory/chunk_header.hpp"
#include "shm/algorithm.hpp"
#include "shm/memory.hpp"

namespace shm {
namespace memory {

ChunkSettings::ChunkSettings(const uint32_t userPayloadSize,
                             const uint32_t userPayloadAlignment,
                             const uint32_t userHeaderSize,
                             const uint32_t userHeaderAlignment,
                             const uint32_t requiredChunkSize) noexcept
    : m_userPayloadSize(userPayloadSize),
      m_userPayloadAlignment(userPayloadAlignment),
      m_userHeaderSize(userHeaderSize),
      m_userHeaderAlignment(userHeaderAlignment),
      m_requiredChunkSize(requiredChunkSize) {}

tl::expected<ChunkSettings, ChunkSettings::Error> ChunkSettings::create(
    const uint32_t userPayloadSize, const uint32_t userPayloadAlignment,
    const uint32_t userHeaderSize,
    const uint32_t userHeaderAlignment) noexcept {
  // since alignas accepts 0, we also do but we adjust it to 1 in case there are
  // some division or modulo operations with the alignment later on
  uint32_t adjustedUserPayloadAlignment =
      userPayloadAlignment == 0U ? 1U : userPayloadAlignment;
  uint32_t adjustedUserHeaderAlignment =
      userHeaderAlignment == 0U ? 1U : userHeaderAlignment;

  if (!details::isPowerOfTwo(adjustedUserPayloadAlignment) ||
      !details::isPowerOfTwo(adjustedUserHeaderAlignment)) {
    return tl::make_unexpected(
        ChunkSettings::Error::ALIGNMENT_NOT_POWER_OF_TWO);
  }

  if (adjustedUserHeaderAlignment > alignof(ChunkHeader)) {
    // for ease of calculation, the alignment of the user-header is restricted
    // to not exceed the alignment of the ChunkHeader
    return tl::make_unexpected(
        ChunkSettings::Error::
            USER_HEADER_ALIGNMENT_EXCEEDS_CHUNK_HEADER_ALIGNMENT);
  }

  if (userHeaderSize % adjustedUserHeaderAlignment != 0U) {
    return tl::make_unexpected(
        ChunkSettings::Error::USER_HEADER_SIZE_NOT_MULTIPLE_OF_ITS_ALIGNMENT);
  }

  uint64_t requiredChunkSize = calculateRequiredChunkSize(
      userPayloadSize, adjustedUserPayloadAlignment, userHeaderSize);

  if (requiredChunkSize > std::numeric_limits<uint32_t>::max()) {
    return tl::make_unexpected(
        ChunkSettings::Error::REQUIRED_CHUNK_SIZE_EXCEEDS_MAX_CHUNK_SIZE);
  }

  return ChunkSettings{userPayloadSize, adjustedUserPayloadAlignment,
                       userHeaderSize, adjustedUserHeaderAlignment,
                       static_cast<uint32_t>(requiredChunkSize)};
}
uint64_t ChunkSettings::calculateRequiredChunkSize(
    const uint32_t userPayloadSize, const uint32_t userPayloadAlignment,
    const uint32_t userHeaderSize) noexcept {
  // have a look at »Required Chunk Size Calculation« in chunk_header.md for
  // more details regarding the calculation
  if (userHeaderSize == 0) {
    // the most simple case with no user-header and the user-payload adjacent to
    // the ChunkHeader
    if (userPayloadAlignment <= alignof(ChunkHeader)) {
      uint64_t requiredChunkSize = sizeof(ChunkHeader) + userPayloadSize;

      return requiredChunkSize;
    }

    // the second most simple case with no user-header but the user-payload
    // alignment exceeds the ChunkHeader alignment and is therefore not
    // necessarily adjacent
    uint64_t preUserPayloadAlignmentOverhang =
        sizeof(ChunkHeader) - alignof(ChunkHeader);
    uint64_t requiredChunkSize = preUserPayloadAlignmentOverhang +
                                 userPayloadAlignment + userPayloadSize;

    return requiredChunkSize;
  }

  // the most complex case with a user-header
  constexpr uint64_t SIZE_OF_USER_PAYLOAD_OFFSET_T{
      sizeof(ChunkHeader::UserPayloadOffset_t)};
  constexpr uint64_t ALIGNMENT_OF_USER_PAYLOAD_OFFSET_T{
      alignof(ChunkHeader::UserPayloadOffset_t)};
  uint64_t headerSize = sizeof(ChunkHeader) + userHeaderSize;
  uint64_t preUserPayloadAlignmentOverhang =
      details::align(headerSize, ALIGNMENT_OF_USER_PAYLOAD_OFFSET_T);
  uint64_t maxPadding =
      details::maxVal(SIZE_OF_USER_PAYLOAD_OFFSET_T,
                      static_cast<uint64_t>(userPayloadAlignment));
  uint64_t requiredChunkSize =
      preUserPayloadAlignmentOverhang + maxPadding + userPayloadSize;

  return requiredChunkSize;
}

uint32_t ChunkSettings::requiredChunkSize() const noexcept {
  return m_requiredChunkSize;
}

uint32_t ChunkSettings::userPayloadSize() const noexcept {
  return m_userPayloadSize;
}

uint32_t ChunkSettings::userPayloadAlignment() const noexcept {
  return m_userPayloadAlignment;
}

uint32_t ChunkSettings::userHeaderSize() const noexcept {
  return m_userHeaderSize;
}

uint32_t ChunkSettings::userHeaderAlignment() const noexcept {
  return m_userHeaderAlignment;
}

}  // namespace memory
}  // namespace shm