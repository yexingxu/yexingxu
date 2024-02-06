

#include "shm/memory_block.hpp"

namespace shm {
namespace details {

void MemoryBlock::onMemoryAvailable(not_null<void*> memory
                                    [[maybe_unused]]) noexcept {
  // nothing to do in the default implementation
}

tl::optional<void*> MemoryBlock::memory() const noexcept {
  return m_memory ? tl::make_optional<void*>(m_memory) : tl::nullopt;
}

}  // namespace details
}  // namespace shm