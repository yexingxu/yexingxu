
#pragma once

#include <cstdint>

#include "memory/memory_configs.hpp"
#include "types/expected.hpp"

namespace shm {
namespace memory {

class ChunkSettings {
 public:
  enum class Error {
    ALIGNMENT_NOT_POWER_OF_TWO,
    USER_HEADER_ALIGNMENT_EXCEEDS_CHUNK_HEADER_ALIGNMENT,
    USER_HEADER_SIZE_NOT_MULTIPLE_OF_ITS_ALIGNMENT,
    REQUIRED_CHUNK_SIZE_EXCEEDS_MAX_CHUNK_SIZE,
  };

  /// @brief constructs and initializes a ChunkSettings
  /// @param[in] chunkSize is the size of the chunk fulfilling the user-payload
  /// and user-header requirements
  /// @param[in] userPayloadSize is the size of the user-payload
  /// @param[in] userPayloadAlignment is the alignment of the user-payload
  /// @param[in] userHeaderSize is the size of the user-header
  /// @param[in] userHeaderAlignment is the alignment for the user-header
  static tl::expected<ChunkSettings, ChunkSettings::Error> create(
      const uint32_t userPayloadSize,
      const uint32_t userPayloadAlignment =
          CHUNK_DEFAULT_USER_PAYLOAD_ALIGNMENT,
      const uint32_t userHeaderSize = CHUNK_NO_USER_HEADER_SIZE,
      const uint32_t userHeaderAlignment =
          CHUNK_NO_USER_HEADER_ALIGNMENT) noexcept;

  /// @brief getter method for the chunk size fulfilling the user-payload and
  /// user-header requirements
  /// @return the chunk size
  uint32_t requiredChunkSize() const noexcept;

  /// @brief getter method for the user-payload size
  /// @return the user-payload size
  uint32_t userPayloadSize() const noexcept;

  /// @brief getter method for the user-payload alignment
  /// @return the user-payload alignment
  uint32_t userPayloadAlignment() const noexcept;

  /// @brief getter method for the user-header size
  /// @return the user-header size
  uint32_t userHeaderSize() const noexcept;

  /// @brief getter method for the user-header alignment
  /// @return the user-header alignment
  uint32_t userHeaderAlignment() const noexcept;

 private:
  ChunkSettings(const uint32_t userPayloadSize,
                const uint32_t userPayloadAlignment,
                const uint32_t userHeaderSize,
                const uint32_t userHeaderAlignment,
                const uint32_t requiredChunkSize) noexcept;

  static uint64_t calculateRequiredChunkSize(
      const uint32_t userPayloadSize, const uint32_t userPayloadAlignment,
      const uint32_t userHeaderSize) noexcept;

 private:
  uint32_t m_userPayloadSize{0U};
  uint32_t m_userPayloadAlignment{0U};
  uint32_t m_userHeaderSize{0U};
  uint32_t m_userHeaderAlignment{0U};
  uint32_t m_requiredChunkSize{0U};
};

}  // namespace memory
}  // namespace shm