#pragma once

#include <atomic>
#include <cstdint>

#include "entity/index_queue.hpp"
#include "types/optional.hpp"
namespace shm {
namespace entity {

/// @brief implements a lock free queue (i.e. container with FIFO order) of
/// elements of type T with a fixed Capacity
template <typename ElementType, uint64_t Capacity>
class LockFreeQueue {
 public:
  using element_t = ElementType;

  /// @brief creates and initalizes an empty LockFreeQueue
  LockFreeQueue() noexcept;

  ~LockFreeQueue() noexcept = default;

  // remark: a thread-safe and lockfree implementation of copy seems impossible
  // but unsafe copying (i.e. where synchronization is up to the user) would be
  // possible can be implemented when it is needed
  LockFreeQueue(const LockFreeQueue&) = delete;
  LockFreeQueue(LockFreeQueue&&) = delete;
  LockFreeQueue& operator=(const LockFreeQueue&) = delete;
  LockFreeQueue& operator=(LockFreeQueue&&) = delete;

  /// @brief returns the capacity of the queue
  /// @note threadsafe, lockfree
  constexpr uint64_t capacity() const noexcept;

  /// @brief tries to insert value in FIFO order, moves the value internally
  /// @param value to be inserted
  /// @return true if insertion was successful (i.e. queue was not full during
  /// push), false otherwise
  /// @note threadsafe, lockfree
  bool tryPush(ElementType&& value) noexcept;

  /// @brief tries to insert value in FIFO order, copies the value internally
  /// @param value to be inserted
  /// @return true if insertion was successful (i.e. queue was not full during
  /// push), false otherwise
  /// @note threadsafe, lockfree
  bool tryPush(const ElementType& value) noexcept;

  /// @brief inserts value in FIFO order, always succeeds by removing the oldest
  /// value when the queue is detected to be full (overflow)
  /// @param value to be inserted is copied into the queue
  /// @return removed value if an overflow occured, empty optional otherwise
  /// @note threadsafe, lockfree
  tl::optional<ElementType> push(const ElementType& value) noexcept;

  /// @brief inserts value in FIFO order, always succeeds by removing the oldest
  /// value when the queue is detected to be full (overflow)
  /// @param value to be inserted is moved into the queue if possible
  /// @return removed value if an overflow occured, empty optional otherwise
  /// @note threadsafe, lockfree
  tl::optional<ElementType> push(ElementType&& value) noexcept;

  /// @brief tries to remove value in FIFO order
  /// @return value if removal was successful, empty optional otherwise
  /// @note threadsafe, lockfree
  tl::optional<ElementType> pop() noexcept;

  /// @brief check whether the queue is empty
  /// @return true iff the queue is empty
  /// @note that if the queue is used concurrently it might
  /// not be empty anymore after the call
  ///  (but it was at some point during the call)
  /// @note threadsafe, lockfree
  bool empty() const noexcept;

  /// @brief get the number of stored elements in the queue
  /// @return number of stored elements in the queue
  /// @note that this will not be perfectly in sync with the actual number of
  /// contained elements during concurrent operation but will always be at most
  /// capacity
  /// @note threadsafe, lockfree
  uint64_t size() const noexcept;

 protected:
  using Queue = IndexQueue<Capacity>;

  // remark: actually m_freeIndices do not have to be in a queue, it could be
  // another multi-push multi-pop capable lockfree container (e.g. a stack or a
  // list)
  Queue m_freeIndices;

  // required to be a queue for LockFreeQueue to exhibit FIFO behaviour
  Queue m_usedIndices;

  std::array<ElementType, Capacity> m_buffer;

  std::atomic<uint64_t> m_size{0U};

  // template is needed to distinguish between lvalue and rvalue T references
  // (universal reference type deduction)
  template <typename T>
  void writeBufferAt(const uint64_t& index, T&& value) noexcept;

  // needed to avoid code duplication (via universal reference type deduction)
  template <typename T>
  tl::optional<ElementType> pushImpl(T&& value) noexcept;

  tl::optional<ElementType> readBufferAt(const uint64_t& index) noexcept;
};

template <typename ElementType, uint64_t Capacity>
LockFreeQueue<ElementType, Capacity>::LockFreeQueue() noexcept
    : m_freeIndices(IndexQueue<Capacity>::ConstructFull),
      m_usedIndices(IndexQueue<Capacity>::ConstructEmpty) {}

template <typename ElementType, uint64_t Capacity>
constexpr uint64_t LockFreeQueue<ElementType, Capacity>::capacity()
    const noexcept {
  return Capacity;
}

template <typename ElementType, uint64_t Capacity>
bool LockFreeQueue<ElementType, Capacity>::tryPush(
    const ElementType& value) noexcept {
  uint64_t index{0};

  if (!m_freeIndices.pop(index)) {
    return false;  // detected full queue, value is unchanged (as demanded by
                   // const)
  }

  writeBufferAt(index, value);  // const& version is called

  m_usedIndices.push(index);

  return true;  // value was copied into the queue and is unchanged
}

template <typename ElementType, uint64_t Capacity>
bool LockFreeQueue<ElementType, Capacity>::tryPush(
    ElementType&& value) noexcept {
  uint64_t index{0};

  if (!m_freeIndices.pop(index)) {
    return false;  // detected full queue
  }

  writeBufferAt(index,
                std::forward<ElementType>(value));  //&& version is called

  m_usedIndices.push(index);

  return true;
}

template <typename ElementType, uint64_t Capacity>
template <typename T>
tl::optional<ElementType> LockFreeQueue<ElementType, Capacity>::pushImpl(
    T&& value) noexcept {
  tl::optional<ElementType> evictedValue;

  uint64_t index{0};

  while (!m_freeIndices.pop(index)) {
    // only pop the index if the queue is still full
    // note, this leads to issues if an index is lost
    // (only possible due to an application crash)
    // then the queue can never be full and we may never leave if no one calls a
    // concurrent pop A quick remedy is not to use a conditional pop such as
    // popIfFull here, but a normal one. However, then it can happen that due to
    // a concurrent pop it was not really necessary to evict a value (i.e. we
    // may needlessly lose values in rare cases) Whether there is another
    // acceptable solution needs to be explored.
    if (m_usedIndices.popIfFull(index)) {
      evictedValue = readBufferAt(index);
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

  writeBufferAt(index, std::forward<T>(value));

  m_usedIndices.push(index);

  return evictedValue;  // value was moved into the queue, if a value was
                        // evicted to do so return it
}

template <typename ElementType, uint64_t Capacity>
tl::optional<ElementType> LockFreeQueue<ElementType, Capacity>::push(
    const ElementType& value) noexcept {
  return pushImpl(std::forward<const ElementType>(value));
}

template <typename ElementType, uint64_t Capacity>
tl::optional<ElementType> LockFreeQueue<ElementType, Capacity>::push(
    ElementType&& value) noexcept {
  return pushImpl(std::forward<ElementType>(value));
}

template <typename ElementType, uint64_t Capacity>
tl::optional<ElementType> LockFreeQueue<ElementType, Capacity>::pop() noexcept {
  uint64_t index{0};

  if (!m_usedIndices.pop(index)) {
    return tl::nullopt;  // detected empty queue
  }

  auto result = readBufferAt(index);

  m_freeIndices.push(index);

  return result;
}

template <typename ElementType, uint64_t Capacity>
bool LockFreeQueue<ElementType, Capacity>::empty() const noexcept {
  return m_usedIndices.empty();
}

template <typename ElementType, uint64_t Capacity>
uint64_t LockFreeQueue<ElementType, Capacity>::size() const noexcept {
  return m_size.load(std::memory_order_relaxed);
}

template <typename ElementType, uint64_t Capacity>
tl::optional<ElementType> LockFreeQueue<ElementType, Capacity>::readBufferAt(
    const uint64_t& index) noexcept {
  // also used for buffer synchronization
  m_size.fetch_sub(1U, std::memory_order_acquire);

  auto& element = m_buffer[index];
  tl::optional<ElementType> result(std::move(element));
  element.~ElementType();
  return result;
}

template <typename ElementType, uint64_t Capacity>
template <typename T>
void LockFreeQueue<ElementType, Capacity>::writeBufferAt(const uint64_t& index,
                                                         T&& value) noexcept {
  auto elementPtr = &m_buffer[index];
  new (elementPtr) ElementType(std::forward<T>(
      value));  // move ctor invoked when available, copy ctor otherwise

  // also used for buffer synchronization
  m_size.fetch_add(1U, std::memory_order_release);
}

}  // namespace entity
}  // namespace shm