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

}  // namespace entity
}  // namespace shm