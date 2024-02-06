#include "shm/shared_memory_provider.hpp"

#include <spdlog/spdlog.h>

#include "shm/system_call.hpp"

namespace shm {
namespace details {

constexpr access_rights SharedMemoryProvider::SHM_MEMORY_PERMISSIONS;

SharedMemoryProvider::SharedMemoryProvider(const ShmName_t& shmName,
                                           const AccessMode accessMode,
                                           const OpenMode openMode) noexcept
    : m_shmName(shmName), m_accessMode(accessMode), m_openMode(openMode) {}

SharedMemoryProvider::~SharedMemoryProvider() noexcept {
  if (isAvailable()) {
    destroy().or_else([](auto) {
      spdlog::warn("failed to cleanup POSIX shared memory provider resources");
    });
  }
}

tl::expected<void*, MemoryProviderError> SharedMemoryProvider::createMemory(
    const uint64_t size, const uint64_t alignment) noexcept {
  // TODO
  if (alignment > pageSize()) {
    return tl::make_unexpected(
        MemoryProviderError::MEMORY_ALIGNMENT_EXCEEDS_PAGE_SIZE);
  }

  auto res = SharedMemoryObjectBuilder(m_shmName, size, m_accessMode,
                                       m_openMode, SHM_MEMORY_PERMISSIONS)
                 .create();

  if (res.has_value()) {
    m_shmObject.emplace(std::move(res.value()));
  } else {
    return tl::make_unexpected(MemoryProviderError::MEMORY_CREATION_FAILED);
  }

  auto baseAddress = m_shmObject->getBaseAddress();
  if (baseAddress == nullptr) {
    return tl::make_unexpected(MemoryProviderError::MEMORY_CREATION_FAILED);
  }

  return baseAddress;
}

tl::expected<void, MemoryProviderError>
SharedMemoryProvider::destroyMemory() noexcept {
  m_shmObject.reset();
  return {};
}

}  // namespace details
}  // namespace shm