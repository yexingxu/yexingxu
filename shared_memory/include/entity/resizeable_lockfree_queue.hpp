#pragma once
#include <vector>

#include "entity/lockfree_queue.hpp"
#include "utils/type_traits.hpp"

namespace shm {
namespace entity {

template <typename ElementType, uint64_t MaxCapacity>
class ResizeableLockFreeQueue
    : protected LockFreeQueue<ElementType, MaxCapacity> {
 private:
  using Base = LockFreeQueue<ElementType, MaxCapacity>;

 public:
  using element_t = ElementType;
  static constexpr uint64_t MAX_CAPACITY = MaxCapacity;

  ResizeableLockFreeQueue() noexcept = default;
  ~ResizeableLockFreeQueue() noexcept = default;

  // deleted for now, can be implemented later if needed
  // note: concurrent copying or moving in lockfree fashion is nontrivial
  ResizeableLockFreeQueue(const ResizeableLockFreeQueue&) = delete;
  ResizeableLockFreeQueue(ResizeableLockFreeQueue&&) = delete;
  ResizeableLockFreeQueue& operator=(const ResizeableLockFreeQueue&) = delete;
  ResizeableLockFreeQueue& operator=(ResizeableLockFreeQueue&&) = delete;

  explicit ResizeableLockFreeQueue(const uint64_t initialCapacity) noexcept;

  /// @brief returns the maximum capacity of the queue
  /// @return the maximum capacity
  static constexpr uint64_t maxCapacity() noexcept;

  using Base::empty;
  using Base::pop;
  using Base::size;
  using Base::tryPush;

  /// @brief returns the current capacity of the queue
  /// @return the current capacity
  /// @note threadsafe, lockfree
  uint64_t capacity() const noexcept;

  /// @brief inserts value in FIFO order, always succeeds by removing the oldest
  /// value when the queue is detected to be full (overflow)
  /// @param[in] value to be inserted is copied into the queue
  /// @return removed value if an overflow occured, empty optional otherwise
  /// @note threadsafe, lockfree
  tl::optional<ElementType> push(const ElementType& value) noexcept;

  /// @brief inserts value in FIFO order, always succeeds by removing the oldest
  /// value when the queue is detected to be full (overflow)
  /// @param[in] value to be inserted is moved into the queue if possible
  /// @return removed value if an overflow occured, empty optional otherwise
  /// @note threadsafe, lockfree
  tl::optional<ElementType> push(ElementType&& value) noexcept;

  // overloads to set the capacity
  // 1) The most general one allows providing a removeHandler to specify remove
  // behavior.
  //    This could e.g. be to store them in a container.
  // 2) The second overload discards removed elements.

  /// @note setCapacity is lockfree, but if an application crashes during
  /// setCapacity it
  ///       currently may prevent other applications from setting the capacity
  ///       (they will not block though). This is not a problem if for example
  ///       there is only one application calling setCapacity or setCapacity is
  ///       only called from vital applications (which if they crash will lead
  ///       to system shutdown) and there is only one (non-vital, i.e. allowed
  ///       to crash) application reading the data via pop. The reader
  ///       application may also call setCapacity, since if it crashes there is
  ///       no one reading the data and the capacity can be considered
  ///       meaningless.

  /// @brief      Set the capacity to some value.
  /// @param[in]  newCapacity capacity to be set
  /// @param[in]  removeHandler is a function taking an element which specifies
  ///             what to do with removed elements should the need for removal
  ///             arise.
  /// @return     true if the capacity was successfully set, false otherwise
  template <typename Function, typename = typename std::enable_if<is_invocable<
                                   Function, ElementType>::value>::type>
  bool setCapacity(const uint64_t newCapacity,
                   Function&& removeHandler) noexcept;

  /// @brief Set the capacity to a new capacity between 0 and MaxCapacity, if
  /// the capacity is reduced it may be necessary to remove the least recent
  /// elements which are then discarded.
  /// @param[in] newCapacity new capacity to be set, if it is larger than
  /// MaxCapacity the call fails
  /// @return true setting if the new capacity was successful, false otherwise
  /// (newCapacity > MaxCapacity)
  /// @note threadsafe, lockfree but multiple concurrent calls may have no
  /// effect
  bool setCapacity(const uint64_t newCapacity) noexcept;

 private:
  using BufferIndex = uint64_t;
  std::atomic<uint64_t> m_capacity{MaxCapacity};
  // must be operator= otherwise it is undefined, see
  // https://en.cppreference.com/w/cpp/atomic/ATOMIC_FLAG_INIT
  std::atomic_flag m_resizeInProgress = ATOMIC_FLAG_INIT;
  std::vector<BufferIndex> m_unusedIndices;

  /// @brief      Increase the capacity by some value.
  /// @param[in]  toIncrease value by which the capacity is to be increased
  /// @return     value by which the capacity was actually increased
  /// @note       If incrementing cannot be carried out (because the MaxCapacity
  /// was reached),
  ///             this value will be smaller than toIncrease.
  uint64_t increaseCapacity(const uint64_t toIncrease) noexcept;

  /// @brief      Decrease the capacity by some value.
  /// @param[in]  toDecrease value by which the capacity is to be decreased
  /// @param[in]  removedHandler is a function  which specifies what to do with
  /// removed elements
  ///             (e.g. store in a container or discard it).
  /// @return     value by which the capacity was actually decreased.
  /// @note       If decrementing cannot be carried out (because the capacity is
  /// already 0),
  ///             this value will be smaller than toDecrease.
  template <typename Function>
  uint64_t decreaseCapacity(const uint64_t toDecrease,
                            Function&& removeHandler) noexcept;

  /// @brief       Try to get a used index if available.
  /// @param[out]  index index obtained in the successful case
  /// @return      true if an index was obtained, false otherwise
  bool tryGetUsedIndex(BufferIndex& index) noexcept;

  template <typename T>
  tl::optional<ElementType> pushImpl(T&& value) noexcept;
};

template <typename ElementType, uint64_t MaxCapacity>
ResizeableLockFreeQueue<ElementType, MaxCapacity>::ResizeableLockFreeQueue(
    const uint64_t initialCapacity) noexcept {
  setCapacity(initialCapacity);
}

template <typename ElementType, uint64_t MaxCapacity>
constexpr uint64_t
ResizeableLockFreeQueue<ElementType, MaxCapacity>::maxCapacity() noexcept {
  return MAX_CAPACITY;
}

template <typename ElementType, uint64_t MaxCapacity>
uint64_t ResizeableLockFreeQueue<ElementType, MaxCapacity>::capacity()
    const noexcept {
  return m_capacity.load(std::memory_order_relaxed);
}

template <typename ElementType, uint64_t MaxCapacity>
bool ResizeableLockFreeQueue<ElementType, MaxCapacity>::setCapacity(
    const uint64_t newCapacity) noexcept {
  auto removeHandler = [](const ElementType&) {};
  return setCapacity(newCapacity, removeHandler);
}

template <typename ElementType, uint64_t MaxCapacity>
template <typename Function, typename>
bool ResizeableLockFreeQueue<ElementType, MaxCapacity>::setCapacity(
    const uint64_t newCapacity, Function&& removeHandler) noexcept {
  if (newCapacity > MAX_CAPACITY) {
    return false;
  }

  /// @note The vector m_unusedIndices is protected by the atomic flag, but this
  /// also means dying during a resize will prevent further resizes. This is not
  /// a problem for the use case were only the dying receiver itself requests
  /// the resize. I.e. resize is lockfree, but it assumes that a concurrent
  /// resize will always eventually complete (which is true when the application
  /// does not die and the relevant thread is scheduled eventually. The latter
  /// is the case for any OS and mandatory for a Realtime OS.

  if (m_resizeInProgress.test_and_set(std::memory_order_acquire)) {
    // at most one resize can be in progress at any time
    return false;
  }

  auto cap = capacity();

  while (cap != newCapacity) {
    if (cap < newCapacity) {
      auto toIncrease = newCapacity - cap;
      increaseCapacity(toIncrease);  // return value does not matter, we check
                                     // the capacity later
    } else {
      auto toDecrease = cap - newCapacity;
      decreaseCapacity(toDecrease,
                       removeHandler);  // return value does not matter, we
                                        // check the capacity later
    }

    cap = capacity();
  }

  // sync everything related to capacity change, e.g. the new capacity stored in
  // m_capacity
  m_resizeInProgress.clear(std::memory_order_release);
  return true;
}

template <typename ElementType, uint64_t MaxCapacity>
uint64_t ResizeableLockFreeQueue<ElementType, MaxCapacity>::increaseCapacity(
    const uint64_t toIncrease) noexcept {
  // we can be sure this is not called concurrently due to the
  // m_resizeInProgress flag
  //(this must be ensured as the vector is modified)
  uint64_t increased = 0U;
  while (increased < toIncrease) {
    if (m_unusedIndices.empty()) {
      // no indices left to increase capacity
      return increased;
    }
    ++increased;
    m_capacity.fetch_add(1U);
    Base::m_freeIndices.push(m_unusedIndices.back());
    m_unusedIndices.pop_back();
  }

  return increased;
}

template <typename ElementType, uint64_t MaxCapacity>
template <typename Function>
uint64_t ResizeableLockFreeQueue<ElementType, MaxCapacity>::decreaseCapacity(
    const uint64_t toDecrease, Function&& removeHandler) noexcept {
  uint64_t decreased = 0U;
  while (decreased < toDecrease) {
    BufferIndex index{0};
    while (decreased < toDecrease) {
      if (!Base::m_freeIndices.pop(index)) {
        break;
      }

      m_unusedIndices.push_back(index);
      ++decreased;
      if (m_capacity.fetch_sub(1U) == 1U) {
        // we reached capacity 0 and cannot further decrease it
        return decreased;
      }
    }

    // no free indices, try the used ones
    while (decreased < toDecrease) {
      // remark: just calling pop to create free space is not sufficent in a
      // concurrent scenario we want to make sure no one else gets the index
      // once we have it
      if (!tryGetUsedIndex(index)) {
        // try the free ones again
        break;
      }

      auto result = Base::readBufferAt(index);
      removeHandler(result.value());
      m_unusedIndices.push_back(index);

      ++decreased;
      if (m_capacity.fetch_sub(1U) == 1U) {
        // we reached capacity 0 and cannot further decrease it
        return decreased;
      }
    }
  }
  return decreased;
}

template <typename ElementType, uint64_t MaxCapacity>
bool ResizeableLockFreeQueue<ElementType, MaxCapacity>::tryGetUsedIndex(
    BufferIndex& index) noexcept {
  /// @note: we have a problem here if we lose an index entirely, since the
  /// queue can then never be full again (or, more generally contain capacity
  /// indices) to lessen this problem, we could use a regular pop if we fail too
  /// often here instead of a variation of popIfFull (which will never work
  /// then)

  return Base::m_usedIndices.popIfSizeIsAtLeast(capacity(), index);
}

template <typename ElementType, uint64_t MaxCapacity>
tl::optional<ElementType>
ResizeableLockFreeQueue<ElementType, MaxCapacity>::push(
    const ElementType& value) noexcept {
  return pushImpl(std::forward<const ElementType>(value));
}

template <typename ElementType, uint64_t MaxCapacity>
tl::optional<ElementType>
ResizeableLockFreeQueue<ElementType, MaxCapacity>::push(
    ElementType&& value) noexcept {
  return pushImpl(std::forward<ElementType>(value));
}

template <typename ElementType, uint64_t MaxCapacity>
template <typename T>
tl::optional<ElementType>
ResizeableLockFreeQueue<ElementType, MaxCapacity>::pushImpl(
    T&& value) noexcept {
  tl::optional<ElementType> evictedValue;

  BufferIndex index{0};

  while (!Base::m_freeIndices.pop(index)) {
    if (tryGetUsedIndex(index)) {
      evictedValue = Base::readBufferAt(index);
      break;
    }
    // if m_usedIndices was not full we try again (m_freeIndices should contain
    // an index in this case) note that it is theoretically possible to be
    // unsuccessful indefinitely (and thus we would have an infinite loop) but
    // this requires a timing of concurrent pushes and pops which is
    // exceptionally unlikely in practice
  }

  // if we removed from a full queue via popIfFull it might not be full anymore
  // when a concurrent pop occurs

  Base::writeBufferAt(index, std::forward<T>(value));

  Base::m_usedIndices.push(index);

  return evictedValue;  // value was moved into the queue, if a value was
                        // evicted to do so return it
}

}  // namespace entity
}  // namespace shm