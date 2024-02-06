#include "memory/loffli.hpp"

namespace shm {
namespace memory {

void LoFFLi::init(not_null<Index_t*> freeIndicesMemory,
                  const uint32_t capacity) noexcept {
  EXPECTS(capacity > 0);
  constexpr uint32_t INTERNALLY_RESERVED_INDICES{1U};
  EXPECTS(capacity <
          (std::numeric_limits<Index_t>::max() - INTERNALLY_RESERVED_INDICES));
  EXPECTS(m_head.is_lock_free());

  m_nextFreeIndex = freeIndicesMemory;
  m_size = capacity;
  m_invalidIndex = m_size + 1;

  if (m_nextFreeIndex != nullptr) {
    for (uint32_t i = 0; i < m_size + 1; i++) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) upper
      // limit of index is set by m_size
      m_nextFreeIndex.get()[i] = i + 1;
    }
  }
}

bool LoFFLi::pop(Index_t& index) noexcept {
  Node oldHead = m_head.load(std::memory_order_acquire);
  Node newHead = oldHead;

  do {
    // we are empty if next points to an element with index of Size
    if (oldHead.indexToNextFreeIndex >= m_size || !m_nextFreeIndex) {
      return false;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) upper
    // limit of index set by m_size
    newHead.indexToNextFreeIndex =
        m_nextFreeIndex.get()[oldHead.indexToNextFreeIndex];
    newHead.abaCounter += 1;
  } while (!m_head.compare_exchange_weak(
      oldHead, newHead, std::memory_order_acq_rel, std::memory_order_acquire));

  /// comes from outside, is not shared and therefore no synchronization is
  /// needed
  index = oldHead.indexToNextFreeIndex;
  /// What if interrupted here an another thread guesses the index and calls
  /// push?
  /// @brief murphy case: m_nextFreeIndex does not require any synchronization
  /// since it
  ///         either is used by the same thread in push or it is given to
  ///         another thread which performs the cleanup and during this process
  ///         a synchronization is required
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  m_nextFreeIndex.get()[index] = m_invalidIndex;

  /// we need to synchronize m_nextFreeIndex with push so that we can perform a
  /// validation check right before push to avoid double free's
  std::atomic_thread_fence(std::memory_order_release);

  return true;
}

bool LoFFLi::push(const Index_t index) noexcept {
  /// we synchronize with m_nextFreeIndex in pop to perform the validity check
  std::atomic_thread_fence(std::memory_order_release);

  /// we want to avoid double free's therefore we check if the index was
  /// acquired in pop and the push argument "index" is valid
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) index is
  // limited by capacity
  if (index >= m_size || !m_nextFreeIndex ||
      m_nextFreeIndex.get()[index] != m_invalidIndex) {
    return false;
  }

  Node oldHead = m_head.load(std::memory_order_acquire);
  Node newHead = oldHead;

  do {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic) index is
    // limited by capacity
    m_nextFreeIndex.get()[index] = oldHead.indexToNextFreeIndex;
    newHead.indexToNextFreeIndex = index;
    newHead.abaCounter += 1;
  } while (!m_head.compare_exchange_weak(
      oldHead, newHead, std::memory_order_acq_rel, std::memory_order_acquire));

  return true;
}

}  // namespace memory
}  // namespace shm