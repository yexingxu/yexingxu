

#pragma once

#include "shm/file_access.hpp"
#include "shm/memory_provider.hpp"
#include "shm/shared_memory_object.hpp"

namespace shm {
namespace details {

class SharedMemoryProvider : public MemoryProvider {
  using ShmName_t = std::string;

 public:
  /// @brief Constructs a SharedMemoryProvider which can be used to request
  /// memory via MemoryBlocks
  /// @param [in] shmName is the name of the posix share memory
  /// @param [in] accessMode defines the read and write access to the memory
  /// @param [in] openMode defines the creation/open mode of the shared memory.
  SharedMemoryProvider(const ShmName_t& shmName, const AccessMode accessMode,
                       const OpenMode openMode) noexcept;
  ~SharedMemoryProvider() noexcept;

  SharedMemoryProvider(SharedMemoryProvider&&) = delete;
  SharedMemoryProvider& operator=(SharedMemoryProvider&&) = delete;

  SharedMemoryProvider(const SharedMemoryProvider&) = delete;
  SharedMemoryProvider& operator=(const SharedMemoryProvider&) = delete;

 protected:
  /// @copydoc MemoryProvider::createMemory
  /// @note This creates and maps a POSIX shared memory to the address space of
  /// the application
  tl::expected<void*, MemoryProviderError> createMemory(
      const uint64_t size, const uint64_t alignment) noexcept override;

  /// @copydoc MemoryProvider::destroyMemory
  /// @note This closes and unmaps a POSIX shared memory
  tl::expected<void, MemoryProviderError> destroyMemory() noexcept override;

 private:
  ShmName_t m_shmName;
  AccessMode m_accessMode{AccessMode::READ_ONLY};
  OpenMode m_openMode{OpenMode::OPEN_EXISTING};
  tl::optional<SharedMemoryObject> m_shmObject;

  static constexpr access_rights SHM_MEMORY_PERMISSIONS =
      perms::owner_read | perms::owner_write | perms::group_read |
      perms::group_write;
};

}  // namespace details
}  // namespace shm