/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-14 17:49:05
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-15 15:19:39
 */
#pragma once

#include <cstdint>

#include "shm/memory_map.hpp"
#include "shm/shared_memory.hpp"
#include "types/expected.hpp"
#include "types/optional.hpp"

namespace shm {
namespace details {

enum class SharedMemoryObjectError {
  SHARED_MEMORY_CREATION_FAILED,
  MAPPING_SHARED_MEMORY_FAILED,
  UNABLE_TO_VERIFY_MEMORY_SIZE,
  REQUESTED_SIZE_EXCEEDS_ACTUAL_SIZE,
  INTERNAL_LOGIC_FAILURE,
};

enum class SharedMemoryAllocationError {
  REQUESTED_MEMORY_AFTER_FINALIZED_ALLOCATION,
  NOT_ENOUGH_MEMORY,
  REQUESTED_ZERO_SIZED_MEMORY

};

class SharedMemoryObjectBuilder;

class SharedMemoryObject : public FileManagementInterface<SharedMemoryObject> {
 public:
  using Builder = SharedMemoryObjectBuilder;

  static constexpr const void* const NO_ADDRESS_HINT = nullptr;
  SharedMemoryObject(const SharedMemoryObject&) = delete;
  SharedMemoryObject& operator=(const SharedMemoryObject&) = delete;
  SharedMemoryObject(SharedMemoryObject&&) noexcept = default;
  SharedMemoryObject& operator=(SharedMemoryObject&&) noexcept = default;
  ~SharedMemoryObject() noexcept = default;

  /// @brief Returns start- or base-address of the shared memory.
  const void* getBaseAddress() const noexcept;

  /// @brief Returns start- or base-address of the shared memory.
  void* getBaseAddress() noexcept;

  /// @brief Returns the underlying file handle of the shared memory
  shm_handle_t getFileHandle() const noexcept;

  /// @brief True if the shared memory has the ownership. False if an already
  ///        existing shared memory was opened.
  bool hasOwnership() const noexcept;

  inline tl::expected<uint64_t, FileStatError> get_size() const noexcept {
    auto result = details::get_file_status(get_file_handle());
    if (!result.has_value()) {
      return tl::make_unexpected(result.error());
    }

    return static_cast<uint64_t>(result->st_size);
  }
  friend class SharedMemoryObjectBuilder;

 private:
  SharedMemoryObject(details::SharedMemory&& sharedMemory,
                     details::MemoryMap&& memoryMap) noexcept;

  friend struct FileManagementInterface<SharedMemoryObject>;

  shm_handle_t get_file_handle() const noexcept;

 private:
  details::SharedMemory m_sharedMemory;
  details::MemoryMap m_memoryMap;
};

class SharedMemoryObjectBuilder {
  /// @brief A valid file name for the shared memory with the restriction that
  ///        no leading dot is allowed since it is not compatible with every
  ///        file system
  SharedMemory::Name_t m_name = "";

  /// @brief Defines the size of the shared memory
  uint64_t m_memorySizeInBytes = 0U;

  /// @brief Defines if the memory should be mapped read only or with write
  /// access.
  ///        A read only memory section will cause a segmentation fault when
  ///        written to.
  AccessMode m_accessMode = AccessMode::READ_ONLY;

  /// @brief Defines how the shared memory is acquired
  OpenMode m_openMode = OpenMode::OPEN_EXISTING;

  /// @brief If this is set to a non null address create will try to map the
  /// shared
  ///        memory to the provided address. Since it is a hint, this mapping
  ///        can fail. The .getBaseAddress() method of the SharedMemoryObject
  ///        returns the actual mapped base address.
  tl::optional<const void*> m_baseAddressHint = tl::nullopt;

  /// @brief Defines the access permissions of the shared memory
  access_rights m_permissions = perms::none;

 public:
  explicit SharedMemoryObjectBuilder(const SharedMemory::Name_t& name,
                                     const uint64_t memorySize,
                                     const AccessMode& am, const OpenMode& om,
                                     const access_rights& rights)
      : m_name(name),
        m_memorySizeInBytes(memorySize),
        m_accessMode(am),
        m_openMode(om),
        m_permissions(rights) {}

  tl::expected<SharedMemoryObject, SharedMemoryObjectError> create() noexcept;
};

}  // namespace details
}  // namespace shm