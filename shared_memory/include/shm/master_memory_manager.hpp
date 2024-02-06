#pragma once

#include <cstddef>
#include <vector>

#include "types/expected.hpp"

namespace shm {
namespace details {

class MemoryProvider;

enum class MemoryManagerError {
  /// attempt to add more memory provider than the capacity allows
  MEMORY_PROVIDER_EXHAUSTED,
  /// an action was performed which requires memory provider
  NO_MEMORY_PROVIDER_PRESENT,
  /// generic error if memory creation failed
  MEMORY_CREATION_FAILED,
  /// generic error if memory destruction failed
  MEMORY_DESTRUCTION_FAILED,
};

class MemoryManagerMaster {
  static const constexpr std::size_t kMaxProviderSize = 8U;

 public:
  MemoryManagerMaster() noexcept = default;
  /// @brief The Destructor of the MemoryManagerMaster also calls destroy on the
  /// registered MemoryProvider
  virtual ~MemoryManagerMaster() noexcept;

  MemoryManagerMaster(MemoryManagerMaster&&) = delete;
  MemoryManagerMaster& operator=(MemoryManagerMaster&&) = delete;

  MemoryManagerMaster(const MemoryManagerMaster&) = delete;
  MemoryManagerMaster& operator=(const MemoryManagerMaster&) = delete;

  /// @brief This function add a MemoryProvider to the memory manager
  /// @param [in] memoryProvider is a pointer to a user defined MemoryProvider
  /// @return an MemoryManagerError::MEMORY_PROVIDER_EXHAUSTED error if no
  /// further memory provider can be added, otherwise success
  tl::expected<void, MemoryManagerError> addMemoryProvider(
      MemoryProvider* memoryProvider) noexcept;

  /// @brief The MemoryManagerMaster calls the the MemoryProvider to create the
  /// memory and announce the availability to its MemoryBlocks
  /// @return an MemoryManagerError if the MemoryProvider cannot create the
  /// memory, otherwise success
  tl::expected<void, MemoryManagerError> createAndAnnounceMemory() noexcept;

  /// @brief The MemoryManagerMaster calls the the MemoryProvider to destroy the
  /// memory, which in turn prompts the MemoryBlocks to destroy their data
  tl::expected<void, MemoryManagerError> destroyMemory() noexcept;

 private:
  std::vector<MemoryProvider*> m_memoryProvider;
};

}  // namespace details
}  // namespace shm