#pragma once

#include <atomic>
#include <cstdint>

#include "types/optional.hpp"

namespace shm {
namespace entity {

template <typename ValueType, uint64_t Capacity>
class FiFo {
 public:
  /// @brief pushes a value into the fifo
  /// @return if the values was pushed successfully into the fifo it returns
  ///         true, otherwise false
  bool push(const ValueType& value) noexcept;

  /// @brief returns the oldest value from the fifo and removes it
  /// @return if the fifo was not empty the optional contains the value,
  ///         otherwise it contains a nullopt
  tl::optional<ValueType> pop() noexcept;

  /// @brief returns true when the fifo is empty, otherwise false
  bool empty() const noexcept;

  /// @brief returns the size of the fifo
  uint64_t size() const noexcept;

  /// @brief returns the capacity of the fifo
  static constexpr uint64_t capacity() noexcept;

 private:
  bool is_full() const noexcept;

 private:
  std::array<ValueType, Capacity> m_data;
  std::atomic<uint64_t> m_write_pos{0};
  std::atomic<uint64_t> m_read_pos{0};
};

template <class ValueType, uint64_t Capacity>
inline bool FiFo<ValueType, Capacity>::push(const ValueType& value) noexcept {
  if (is_full()) {
    return false;
  }
  auto currentWritePos = m_write_pos.load(std::memory_order_relaxed);
  m_data[currentWritePos % Capacity] = value;

  // m_write_pos must be increased after writing the new value otherwise
  // it is possible that the value is read by pop while it is written.
  // this fifo is a single producer, single consumer fifo therefore
  // store is allowed.
  m_write_pos.store(currentWritePos + 1, std::memory_order_release);
  return true;
}

template <class ValueType, uint64_t Capacity>
inline bool FiFo<ValueType, Capacity>::is_full() const noexcept {
  return m_write_pos.load(std::memory_order_relaxed) ==
         m_read_pos.load(std::memory_order_relaxed) + Capacity;
}

template <class ValueType, uint64_t Capacity>
inline uint64_t FiFo<ValueType, Capacity>::size() const noexcept {
  return m_write_pos.load(std::memory_order_relaxed) -
         m_read_pos.load(std::memory_order_relaxed);
}

template <class ValueType, uint64_t Capacity>
inline constexpr uint64_t FiFo<ValueType, Capacity>::capacity() noexcept {
  return Capacity;
}

template <class ValueType, uint64_t Capacity>
inline bool FiFo<ValueType, Capacity>::empty() const noexcept {
  return m_read_pos.load(std::memory_order_relaxed) ==
         m_write_pos.load(std::memory_order_relaxed);
}

template <class ValueType, uint64_t Capacity>
inline tl::optional<ValueType> FiFo<ValueType, Capacity>::pop() noexcept {
  auto currentReadPos = m_read_pos.load(std::memory_order_acquire);
  bool isEmpty = (currentReadPos ==
                  // we are not allowed to use the empty method since we have to
                  // sync with the producer pop - this is done here
                  m_write_pos.load(std::memory_order_acquire));
  if (isEmpty) {
    return tl::nullopt;
  }
  ValueType out = m_data[currentReadPos % Capacity];

  // m_read_pos must be increased after reading the pop'ed value otherwise
  // it is possible that the pop'ed value is overwritten by push while it is
  // read. Implementing a single consumer fifo here allows us to use store.
  m_read_pos.store(currentReadPos + 1, std::memory_order_release);
  return out;
}

}  // namespace entity
}  // namespace shm