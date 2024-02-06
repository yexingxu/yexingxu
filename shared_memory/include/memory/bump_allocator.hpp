

#pragma once

#include <cstdint>

#include "types/expected.hpp"

namespace shm {
namespace memory {

enum class BumpAllocatorError : uint8_t {
  OUT_OF_MEMORY,
  REQUESTED_ZERO_SIZED_MEMORY
};

/// @brief A bump allocator for the memory provided in the ctor arguments
class BumpAllocator final {
 public:
  /// @brief c'tor
  /// @param[in] startAddress of the memory this allocator manages
  /// @param[in] length of the memory this allocator manages
  BumpAllocator(void* const startAddress, const uint64_t length) noexcept;

  BumpAllocator(const BumpAllocator&) = delete;
  BumpAllocator(BumpAllocator&&) noexcept = default;
  BumpAllocator& operator=(const BumpAllocator&) noexcept = delete;
  BumpAllocator& operator=(BumpAllocator&&) noexcept = default;
  ~BumpAllocator() noexcept = default;

  /// @brief allocates on the memory supplied with the ctor
  /// @param[in] size of the memory to allocate, must be greater than 0
  /// @param[in] alignment of the memory to allocate
  /// @return an expected containing a pointer to the memory if allocation was
  /// successful, otherwise BumpAllocatorError
  tl::expected<void*, BumpAllocatorError> allocate(
      const uint64_t size, const uint64_t alignment) noexcept;

  /// @brief mark the memory as unused
  void deallocate() noexcept;

 private:
  uint64_t m_startAddress{0U};
  uint64_t m_length{0U};
  uint64_t m_currentPosition{0U};
};

}  // namespace memory
}  // namespace shm