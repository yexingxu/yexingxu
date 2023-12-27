#pragma once

#include <cstdint>

#include "details/expected.hpp"
#include "details/file_access.hpp"

#define MAP_SHARED 0x01  /* Share changes.  */
#define MAP_PRIVATE 0x02 /* Changes are private.  */

#define MAP_FIXED 0x10 /* Interpret addr exactly.  */

namespace shm {
namespace details {

enum class MemoryMapError {
  ACCESS_FAILED,
  UNABLE_TO_LOCK,
  INVALID_FILE_DESCRIPTOR,
  MAP_OVERLAP,
  INVALID_PARAMETERS,
  OPEN_FILES_SYSTEM_LIMIT_EXCEEDED,
  FILESYSTEM_DOES_NOT_SUPPORT_MEMORY_MAPPING,
  NOT_ENOUGH_MEMORY_AVAILABLE,
  OVERFLOWING_PARAMETERS,
  PERMISSION_FAILURE,
  NO_WRITE_PERMISSION,
  UNKNOWN_ERROR
};

/// @brief Flags defining how the mapped data should be handled
enum class MemoryMapFlags : int32_t {
  /// @brief changes are shared
  SHARE_CHANGES = MAP_SHARED,

  /// @brief changes are private
  PRIVATE_CHANGES = MAP_PRIVATE,

  /// @brief SHARED and enforce the base address hint
  // NOLINTNEXTLINE(hicpp-signed-bitwise) enum type is defined by POSIX, no
  // logical fault
  SHARE_CHANGES_AND_FORCE_BASE_ADDRESS_HINT = MAP_SHARED | MAP_FIXED,

  /// @brief PRIVATE and enforce the base address hint
  // NOLINTNEXTLINE(hicpp-signed-bitwise) enum type is defined POSIX, no logical
  // fault
  PRIVATE_CHANGES_AND_FORCE_BASE_ADDRESS_HINT = MAP_PRIVATE | MAP_FIXED,
};

/// @brief C++ abstraction of mmap and munmap. When a 'MemoryMap' object is
///        created the configured memory is mapped into the process space until
///        that object goes out of scope - then munmap is called and the memory
///        region is removed from the process space.
class MemoryMap {
 public:
  /// @brief copy operations are removed since we are handling a system resource
  MemoryMap(const MemoryMap&) = delete;
  MemoryMap& operator=(const MemoryMap&) = delete;

  /// @brief move constructor
  /// @param[in] rhs the source object
  MemoryMap(MemoryMap&& rhs) noexcept;

  /// @brief move assignment operator
  /// @param[in] rhs the source object
  /// @return reference to *this
  MemoryMap& operator=(MemoryMap&& rhs) noexcept;

  /// @brief destructor, calls munmap when the underlying memory is mapped
  ~MemoryMap() noexcept;

  /// @brief returns the base address, if the object was moved it returns
  /// nullptr
  const void* getBaseAddress() const noexcept;

  /// @brief returns the base address, if the object was moved it returns
  /// nullptr
  void* getBaseAddress() noexcept;

  friend class MemoryMapBuilder;

 private:
  MemoryMap(void* const baseAddress, const uint64_t length) noexcept;
  bool destroy() noexcept;
  static MemoryMapError errnoToEnum(const int32_t errnum) noexcept;

  void* m_baseAddress{nullptr};
  uint64_t m_length{0U};
};

class MemoryMapBuilder {
  /// @brief The base address suggestion to which the memory should be mapped.
  /// But
  ///        there is no guarantee that it is really mapped at this position.
  ///        One has to verify with .getBaseAddress if the hint was accepted.
  ///        Setting it to nullptr means no suggestion
  const void* m_baseAddressHint = nullptr;

  /// @brief The length of the memory which should be mapped
  uint64_t m_length = 0U;

  /// @brief The file descriptor which should be mapped into process space
  int32_t m_fileDescriptor = 0;

  /// @brief Defines if the memory should be mapped read only or with write
  /// access.
  ///        A read only memory section will cause a segmentation fault when
  ///        written to.
  AccessMode m_accessMode = AccessMode::READ_WRITE;

  /// @brief Sets the flags defining how the mapped data should be handled
  MemoryMapFlags m_flags = MemoryMapFlags::SHARE_CHANGES;

  /// @brief Offset of the memory location
  off_t m_offset = 0;

 public:
  /// @brief creates a valid 'MemoryMap' object. If the construction failed
  /// the
  ///        expected contains an enum value describing the error.
  /// @return expected containing 'MemoryMap' on success otherwise
  /// 'MemoryMapError'
  explicit MemoryMapBuilder(const void* baseAddressHint, uint64_t len,
                            int32_t fileDescriptor, AccessMode accessMode,
                            MemoryMapFlags flags, off_t offset)
      : m_baseAddressHint(baseAddressHint),
        m_length(len),
        m_fileDescriptor(fileDescriptor),
        m_accessMode(accessMode),
        m_flags(flags),
        m_offset(offset) {}
  tl::expected<MemoryMap, MemoryMapError> create() noexcept;
};

}  // namespace details
}  // namespace shm