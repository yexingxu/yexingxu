

#include "shm/memory.hpp"

#include <cassert>
#include <cstdlib>

namespace shm {
namespace details {

void* alignedAlloc(const uint64_t alignment, const uint64_t size) noexcept {
  // -1 == since the max alignment addition is alignment - 1 otherwise the
  // memory is already aligned and we have to do nothing
  // low-level memory management, no other approach then to use malloc to
  // acquire heap memory
  // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-pro-type-reinterpret-cast,hicpp-no-malloc,cppcoreguidelines-no-malloc)
  auto memory = reinterpret_cast<uint64_t>(
      std::malloc(size + alignment + sizeof(void*) - 1));
  if (memory == 0) {
    return nullptr;
  }
  uint64_t alignedMemory = align(memory + sizeof(void*), alignment);
  assert(alignedMemory >= memory + 1);
  // low-level memory management, we have to store the actual start of the
  // memory a position before the returned aligned address to be able to release
  // the actual memory address again with free when we only get the aligned
  // address
  // NOLINTNEXTLINE(performance-no-int-to-ptr,cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-pointer-arithmetic)
  reinterpret_cast<void**>(alignedMemory)[-1] = reinterpret_cast<void*>(memory);

  // we have to return a void pointer to the aligned memory address
  // NOLINTNEXTLINE(performance-no-int-to-ptr,cppcoreguidelines-pro-type-reinterpret-cast)
  return reinterpret_cast<void*>(alignedMemory);
}

void alignedFree(void* const memory) noexcept {
  if (memory != nullptr) {
    // low-level memory management
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,
    // cppcoreguidelines-no-malloc,
    // cppcoreguidelines-pro-bounds-pointer-arithmetic)
    // NOLINTNEXTLINE
    std::free(reinterpret_cast<void**>(memory)[-1]);
  }
}

}  // namespace details
}  // namespace shm