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

}  // namespace entity
}  // namespace shm