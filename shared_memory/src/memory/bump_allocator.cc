

#include "memory/bump_allocator.hpp"

#include <spdlog/spdlog.h>

#include <string>

#include "shm/memory.hpp"
#include "types/expected.hpp"

namespace shm {
namespace memory {

BumpAllocator::BumpAllocator(void* const startAddress,
                             const uint64_t length) noexcept
    // AXIVION Next Construct AutosarC++19_03-A5.2.4, AutosarC++19_03-M5.2.9 :
    // required for low level memory management
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    : m_startAddress(reinterpret_cast<uint64_t>(startAddress)),
      m_length(length) {}

// NOLINTJUSTIFICATION allocation interface requires size and alignment as
// integral types NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
tl::expected<void*, BumpAllocatorError> BumpAllocator::allocate(
    const uint64_t size, const uint64_t alignment) noexcept {
  if (size == 0) {
    spdlog::warn("Cannot allocate memory of size 0.");
    return tl::make_unexpected(BumpAllocatorError::REQUESTED_ZERO_SIZED_MEMORY);
  }

  const uint64_t currentAddress{m_startAddress + m_currentPosition};
  uint64_t alignedPosition{details::align(currentAddress, alignment)};

  alignedPosition -= m_startAddress;

  void* allocation{nullptr};

  const uint64_t nextPosition{alignedPosition + size};
  if (m_length >= nextPosition) {
    // AXIVION Next Construct AutosarC++19_03-A5.2.4 : required for low level
    // memory management
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,
    // performance-no-int-to-ptr)
    allocation = reinterpret_cast<void*>(m_startAddress + alignedPosition);
    m_currentPosition = nextPosition;
  } else {
    spdlog::warn("Trying to allocate additional " + std::to_string(size) +
                 " bytes in the memory of capacity " +
                 std::to_string(m_length) + " when there are already " +
                 std::to_string(alignedPosition) +
                 " aligned bytes in use.\n Only " +
                 std::to_string(m_length - alignedPosition) + " bytes left.");
    return tl::make_unexpected(BumpAllocatorError::OUT_OF_MEMORY);
  }

  return allocation;
}

void BumpAllocator::deallocate() noexcept { m_currentPosition = 0; }

}  // namespace memory
}  // namespace shm