

#include <spdlog/spdlog.h>

#include <string>

#include "shm/master_memory_manager.hpp"
#include "shm/memory_provider.hpp"
#include "types/expected.hpp"

namespace shm {
namespace details {

MemoryManagerMaster::~MemoryManagerMaster() noexcept {
  destroyMemory().or_else([](auto) {
    spdlog::warn("Failed to cleanup MemoryManagerMaster in destructor.");
  });
}

tl::expected<void, MemoryManagerError> MemoryManagerMaster::addMemoryProvider(
    MemoryProvider* memoryProvider) noexcept {
  if (m_memoryProvider.size() >= kMaxProviderSize) {
    return tl::make_unexpected(MemoryManagerError::MEMORY_PROVIDER_EXHAUSTED);
  }

  m_memoryProvider.emplace_back(memoryProvider);
  return {};
}

tl::expected<void, MemoryManagerError>
MemoryManagerMaster::createAndAnnounceMemory() noexcept {
  if (m_memoryProvider.empty()) {
    return tl::make_unexpected(MemoryManagerError::NO_MEMORY_PROVIDER_PRESENT);
  }

  for (auto memoryProvider : m_memoryProvider) {
    auto result = memoryProvider->create();
    if (!result.has_value()) {
      spdlog::error(
          "Could not create memory: MemoryProviderError = " +
          std::string(MemoryProvider::getErrorString(result.error())));
      return tl::make_unexpected(MemoryManagerError::MEMORY_CREATION_FAILED);
    }
  }

  for (auto memoryProvider : m_memoryProvider) {
    memoryProvider->announceMemoryAvailable();
  }

  return {};
}

tl::expected<void, MemoryManagerError>
MemoryManagerMaster::destroyMemory() noexcept {
  tl::expected<void, MemoryManagerError> result{};
  for (auto memoryProvider : m_memoryProvider) {
    auto destructionResult = memoryProvider->destroy();
    if (!destructionResult.has_value() &&
        destructionResult.error() !=
            MemoryProviderError::MEMORY_NOT_AVAILABLE) {
      spdlog::error(
          "Could not destroy memory provider! Error: " +
          std::to_string(static_cast<uint64_t>(destructionResult.error())));
      /// @note do not return on first error but try to cleanup the remaining
      /// resources
      if (result.has_value()) {
        result =
            tl::make_unexpected(MemoryManagerError::MEMORY_DESTRUCTION_FAILED);
      }
    }
  }
  return result;
}

}  // namespace details
}  // namespace shm