#pragma once

#include <atomic>
#include <cstdint>
#include <type_traits>

#include "entity/cycle_index.hpp"
#include "types/optional.hpp"

namespace shm {
namespace entity {

template <typename ElementType, uint64_t Capacity>
class LockFreeQueue;

template <typename ElementType, uint64_t Capacity>
class ResizeableLockFreeQueue;

/// @brief lockfree queue capable of storing indices 0,1,... Capacity-1
template <uint64_t Capacity, typename ValueType = uint64_t>
class IndexQueue {
 public:
  static_assert(std::is_unsigned<ValueType>::value,
                "ValueType must be an unsigned integral type");

  using value_t = ValueType;

  struct ConstructFull_t {};

  struct ConstructEmpty_t {};

  static constexpr ConstructFull_t ConstructFull{};
  static constexpr ConstructEmpty_t ConstructEmpty{};

  ~IndexQueue() noexcept = default;
  IndexQueue(const IndexQueue&) = delete;
  IndexQueue(IndexQueue&&) = delete;
  IndexQueue& operator=(const IndexQueue&) = delete;
  IndexQueue& operator=(IndexQueue&&) = delete;

  /// @brief constructs an empty IndexQueue
  explicit IndexQueue(ConstructEmpty_t = ConstructEmpty) noexcept;

  /// @brief constructs IndexQueue filled with all indices 0,1,...capacity-1
  explicit IndexQueue(ConstructFull_t) noexcept;

  /// @brief get the capacity of the IndexQueue
  /// @return capacity of the IndexQueue
  /// threadsafe, lockfree
  constexpr uint64_t capacity() const noexcept;

  /// @brief check whether the queue is empty
  /// @return true iff the queue is empty
  /// note that if the queue is used concurrently it might
  /// not be empty anymore after the call
  /// (but it was at some point during the call)
  bool empty() const noexcept;

  /// @brief push index into the queue in FIFO order
  /// @param index to be pushed
  /// note that do the way it is supposed to be used
  /// we cannot overflow (the number of indices available is bounded
  /// and the capacity is large enough to hold them all)
  void push(const ValueType index) noexcept;

  /// @brief pop an index from the queue in FIFO order if the queue not empty
  /// @return index if the queue was is empty, nullopt oterwise
  tl::optional<ValueType> pop() noexcept;

  /// @brief pop an index from the queue in FIFO order if the queue is full
  /// @return index if the queue was full, nullopt otherwise
  tl::optional<ValueType> popIfFull() noexcept;

  /// @brief pop an index from the queue in FIFO order if the queue contains
  ///        at least  a specified number number of elements
  /// @param size the number of elements needed to successfully perform the pop
  /// @return index if the queue contains size elements, nullopt otherwise
  tl::optional<ValueType> popIfSizeIsAtLeast(uint64_t size) noexcept;

 private:
  template <typename ElementType, uint64_t Cap>
  friend class LockFreeQueue;

  template <typename ElementType, uint64_t Cap>
  friend class ResizeableLockFreeQueue;

  // remark: a compile time check whether Index is actually lock free would be
  // nice note: there is a way  with is_always_lock_free in c++17 (which we
  // cannot use here)
  using Index = CyclicIndex<Capacity>;
  using Cell = std::atomic<Index>;

  /// the array entries have to be initialized explicitly in the constructor
  /// since the default atomic constructor does not call the default constructor
  /// of the underlying class. See,
  /// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0883r0.pdf
  // NOLINTJUSTIFICATION we need an initialized array here and will not use
  // std::array for now NOLINTNEXTLINE(*avoid-c-arrays)
  Cell m_cells[Capacity];

  std::atomic<Index> m_readPosition;
  std::atomic<Index> m_writePosition;

  /// @brief load the value from m_cells at a position with a given memory order
  /// @param position position to load the value from
  /// @param memoryOrder memory order to load the value with
  /// @return value at position
  Index loadvalueAt(
      const Index& position,
      std::memory_order memoryOrder = std::memory_order_relaxed) const noexcept;

  /// @brief pop an index from the queue in FIFO order if the queue not empty
  /// @param index that was obtained, undefined if false is returned
  /// @return true if an index was obtained, false otherwise
  bool pop(ValueType& index) noexcept;

  /// @brief pop an index from the queue in FIFO order if the queue contains at
  /// least minSize indices
  /// @param minSize minimum number of indices required in the queue to
  /// successfully obtain the first index
  /// @param index that was obtained, undefined if false is returned
  /// @return true if an index was obtained, false otherwise
  bool popIfSizeIsAtLeast(uint64_t minSize, ValueType& index) noexcept;

  /// @brief pop an index from the queue in FIFO order if the queue is full
  /// @param index that was obtained, undefined if false is returned
  /// @return true if an index was obtained, false otherwise
  bool popIfFull(ValueType& index) noexcept;
};

template <uint64_t Capacity, typename ValueType>
inline IndexQueue<Capacity, ValueType>::IndexQueue(ConstructEmpty_t) noexcept
    : m_readPosition(Index(Capacity)), m_writePosition(Index(Capacity)) {
  for (uint64_t i = 0U; i < Capacity; ++i) {
    m_cells[i].store(Index(0U), std::memory_order_relaxed);
  }
}

template <uint64_t Capacity, typename ValueType>
IndexQueue<Capacity, ValueType>::IndexQueue(ConstructFull_t) noexcept
    : m_readPosition(Index(0U)), m_writePosition(Index(Capacity)) {
  for (uint64_t i = 0U; i < Capacity; ++i) {
    m_cells[i].store(Index(i), std::memory_order_relaxed);
  }
}

template <uint64_t Capacity, typename ValueType>
constexpr uint64_t IndexQueue<Capacity, ValueType>::capacity() const noexcept {
  return Capacity;
}

template <uint64_t Capacity, typename ValueType>
void IndexQueue<Capacity, ValueType>::push(const ValueType index) noexcept {
  // we need the CAS loop here since we may fail due to concurrent push
  // operations note that we are always able to succeed to publish since we have
  // enough capacity for all unique indices used

  // case analyis
  // (1) loaded value is exactly one cycle behind:
  //     value is from the last cycle
  //     we can try to publish
  // (2) loaded value has the same cycle:
  //     some other push has published but not updated the write position
  //     help updating the write position
  // (3) loaded value is more than one cycle behind:
  //     this should only happen due to wrap around when push is interrupted for
  //     a long time reload write position and try again note that a complete
  //     wraparound can lead to a false detection of 1) (ABA problem) but this
  //     is very unlikely with e.g. a 64 bit value type
  // (4) loaded value is some cycle ahead:
  //     write position is outdated, there must have been other pushes
  //     concurrently reload write position and try again

  constexpr bool NotPublished = true;

  auto writePosition = m_writePosition.load(std::memory_order_relaxed);
  do {
    auto oldValue = loadvalueAt(writePosition, std::memory_order_relaxed);

    auto cellIsFree = oldValue.isOneCycleBehind(writePosition);

    if (cellIsFree) {
      // case (1)
      Index newValue(index, writePosition.getCycle());

      // if publish fails, another thread has published before us
      bool published = m_cells[writePosition.getIndex()].compare_exchange_weak(
          oldValue, newValue, std::memory_order_relaxed,
          std::memory_order_relaxed);

      if (published) {
        break;
      }
    }

    // even if we are not able to publish, we check whether some other push has
    // already updated the writePosition before trying again to publish
    auto writePositionRequiresUpdate =
        oldValue.getCycle() == writePosition.getCycle();

    if (writePositionRequiresUpdate) {
      // case (2)
      // the writePosition was not updated yet by another push but the value was
      // already written help with the update (note that we do not care if it
      // fails, then a retry or another push will handle it)

      Index newWritePosition(writePosition + 1U);
      m_writePosition.compare_exchange_strong(writePosition, newWritePosition,
                                              std::memory_order_relaxed,
                                              std::memory_order_relaxed);
    } else {
      // case (3) and (4)
      // note: we do not update with CAS here, the CAS is bound to fail anyway
      // (since our value of writePosition is not up to date so needs to be
      // loaded again)
      writePosition = m_writePosition.load(std::memory_order_relaxed);
    }

  } while (NotPublished);

  Index newWritePosition(writePosition + 1U);

  // if this compare-exchange fails it is no problem, this only delays the
  // update of m_writePosition for other pushes which are able to do them on
  // their own (if writePositionRequiresUpdate above is true) no one else except
  // popIfFull requires this update: In this case it is also ok: the push is
  // only complete once this update of m_writePosition was executed, and the
  // queue (logically) cannot be full until this happens.
  m_writePosition.compare_exchange_strong(writePosition, newWritePosition,
                                          std::memory_order_relaxed,
                                          std::memory_order_relaxed);
}

template <uint64_t Capacity, typename ValueType>
bool IndexQueue<Capacity, ValueType>::pop(ValueType& index) noexcept {
  // we need the CAS loop here since we may fail due to concurrent pop
  // operations we leave when we detect an empty queue, otherwise we retry the
  // pop operation

  // case analyis
  // (1) loaded value has the same cycle:
  //     value was not popped before
  //     try to get ownership
  // (2) loaded value is exactly one cycle behind:
  //     value is from the last cycle which means the queue is empty
  //     return false
  // (3) loaded value is more than one cycle behind:
  //     this should only happen due to wrap around when push is interrupted for
  //     a long time reload read position and try again
  // (4) loaded value is some cycle ahead:
  //     read position is outdated, there must have been pushes concurrently
  //     reload read position and try again

  bool ownershipGained = false;
  Index value;
  auto readPosition = m_readPosition.load(std::memory_order_relaxed);
  do {
    value = loadvalueAt(readPosition, std::memory_order_relaxed);

    // we only dequeue if value and readPosition are in the same cycle
    auto cellIsValidToRead = readPosition.getCycle() == value.getCycle();

    if (cellIsValidToRead) {
      // case (1)
      Index newReadPosition(readPosition + 1U);
      ownershipGained = m_readPosition.compare_exchange_weak(
          readPosition, newReadPosition, std::memory_order_relaxed,
          std::memory_order_relaxed);
    } else {
      // readPosition is ahead by one cycle, queue was empty at value load
      auto isEmpty = value.isOneCycleBehind(readPosition);

      if (isEmpty) {
        // case (2)
        return false;
      }

      // case (3) and (4) requires loading readPosition again
      readPosition = m_readPosition.load(std::memory_order_relaxed);
    }

    // readPosition is outdated, retry operation

  } while (!ownershipGained);  // we leave if we gain ownership of readPosition

  index = value.getIndex();
  return true;
}

template <uint64_t Capacity, typename ValueType>
bool IndexQueue<Capacity, ValueType>::popIfFull(ValueType& index) noexcept {
  // we do NOT need a CAS loop here since if we detect that the queue is not
  // full someone else popped an element and we do not retry to check whether it
  // was filled AGAIN concurrently (which will usually not be the case and then
  // we would return false anyway) if it is filled again we can (and will) retry
  // popIfFull from the call site

  // the queue is full if and only if write position and read position are the
  // same but read position is one cycle behind write position unfortunately it
  // seems impossible in this design to check this condition without loading
  // write posiion and read position (which causes more contention)

  const auto writePosition = m_writePosition.load(std::memory_order_relaxed);
  auto readPosition = m_readPosition.load(std::memory_order_relaxed);
  const auto value = loadvalueAt(readPosition, std::memory_order_relaxed);

  auto isFull = writePosition.getIndex() == readPosition.getIndex() &&
                readPosition.isOneCycleBehind(writePosition);

  if (isFull) {
    Index newReadPosition(readPosition + 1U);
    auto ownershipGained = m_readPosition.compare_exchange_strong(
        readPosition, newReadPosition, std::memory_order_relaxed,
        std::memory_order_relaxed);

    if (ownershipGained) {
      index = value.getIndex();
      return true;
    }
  }

  // otherwise someone else has dequeued an index and the queue was not full at
  // the start of this popIfFull
  return false;
}

template <uint64_t Capacity, typename ValueType>
bool IndexQueue<Capacity, ValueType>::popIfSizeIsAtLeast(
    const uint64_t minSize, ValueType& index) noexcept {
  if (minSize == 0) {
    return pop(index);
  }
  // which to load first should make no difference for correctness
  // but for performance it might
  // note that without sync mechanisms (such as seq_cst), reordering is possible
  const auto writePosition = m_writePosition.load(std::memory_order_relaxed);
  auto readPosition = m_readPosition.load(std::memory_order_relaxed);

  // if readPosition + n = readPosition for some n>=0, the queue contains n
  // elements at this instant (!) but slightly later may contain more or less
  // elements while the m_readPosition and m_writePosition can grow during this
  // operation, we detect this for readPosition with compare_exchange and for
  // writePosition it does not matter, the queue will contain even more elements
  // then ( > n)
  const int64_t delta = writePosition - readPosition;

  // delta < 0 can actually happen (atomic values may not be up to date, i.e.
  // detect writePosition as smaller than readPosition leading to negative
  // delta) since we cannot conclude that the queue is filled with requiredSize
  // elements in this case we just return
  //
  // note that delta is signed and we cannot compare it to requiredSize
  // (unsigned) when it is negative without getting unexpected results (it will
  // be converted to large positive numbers)
  if (delta < 0) {
    return false;
  }

  // delta is positive, therefore the conversion is fine (it surely fits into
  // uint64_t)
  if (static_cast<uint64_t>(delta) >= minSize) {
    auto value = loadvalueAt(readPosition, std::memory_order_relaxed);
    Index newReadPosition(readPosition + 1U);
    auto ownershipGained = m_readPosition.compare_exchange_strong(
        readPosition, newReadPosition, std::memory_order_relaxed,
        std::memory_order_relaxed);
    if (ownershipGained) {
      index = value.getIndex();
      return true;
    }
  }

  return false;
}

template <uint64_t Capacity, typename ValueType>
tl::optional<ValueType> IndexQueue<Capacity, ValueType>::pop() noexcept {
  ValueType value;
  if (pop(value)) {
    return value;
  }
  return tl::nullopt;
}

template <uint64_t Capacity, typename ValueType>
tl::optional<ValueType> IndexQueue<Capacity, ValueType>::popIfFull() noexcept {
  ValueType value;
  if (popIfFull(value)) {
    return value;
  }
  return tl::nullopt;
}

template <uint64_t Capacity, typename ValueType>
tl::optional<ValueType> IndexQueue<Capacity, ValueType>::popIfSizeIsAtLeast(
    const uint64_t size) noexcept {
  ValueType value;
  if (popIfSizeIsAtLeast(size, value)) {
    return value;
  }
  return tl::nullopt;
}

template <uint64_t Capacity, typename ValueType>
bool IndexQueue<Capacity, ValueType>::empty() const noexcept {
  const auto readPosition = m_readPosition.load(std::memory_order_relaxed);
  const auto value = loadvalueAt(readPosition, std::memory_order_relaxed);

  // if m_readPosition is ahead by one cycle compared to the value stored at
  // head, the queue was empty at the time of the loads above (but might not be
  // anymore!)
  return value.isOneCycleBehind(readPosition);
}

template <uint64_t Capacity, typename ValueType>
typename IndexQueue<Capacity, ValueType>::Index
IndexQueue<Capacity, ValueType>::loadvalueAt(
    const Index& position, const std::memory_order memoryOrder) const noexcept {
  return m_cells[position.getIndex()].load(memoryOrder);
}

}  // namespace entity
}  // namespace shm