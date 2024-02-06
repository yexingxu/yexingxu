#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>
namespace shm {
namespace entity {

template <uint64_t CycleLength, typename ValueType = uint64_t>
class CyclicIndex {
 public:
  static_assert(std::is_unsigned<ValueType>::value,
                "ValueType must be an unsigned integral type");
  static_assert(CycleLength >= 1U, "CycleLength must be >= 1");

  using value_t = ValueType;

  static constexpr ValueType MAX_INDEX = CycleLength - 1U;
  static constexpr ValueType MAX_VALUE = std::numeric_limits<ValueType>::max();

  // assumes MAX_VALUE >= CycleLength, otherwise we could not fit in even one
  // cycle
  static constexpr ValueType MAX_CYCLE = MAX_VALUE / CycleLength;

  static constexpr ValueType INDEX_AT_MAX_VALUE = MAX_VALUE % CycleLength;
  static constexpr ValueType OVERFLOW_START_INDEX =
      (INDEX_AT_MAX_VALUE + 1U) % CycleLength;

  static_assert(CycleLength < MAX_VALUE / 2U,
                "CycleLength is too large, need at least one bit for cycle");
  static_assert(CycleLength > 0, "CycleLength must be > 0");

  explicit CyclicIndex(ValueType value = 0U) noexcept;

  CyclicIndex(ValueType index, ValueType cycle) noexcept;

  ~CyclicIndex() = default;

  CyclicIndex(const CyclicIndex&) noexcept = default;
  CyclicIndex(CyclicIndex&&) noexcept = default;
  CyclicIndex& operator=(const CyclicIndex&) noexcept = default;
  CyclicIndex& operator=(CyclicIndex&&) noexcept = default;

  ValueType getIndex() const noexcept;

  ValueType getCycle() const noexcept;

  ValueType getValue() const noexcept;

  CyclicIndex operator+(const ValueType value) const noexcept;

  CyclicIndex next() const noexcept;

  bool isOneCycleBehind(const CyclicIndex& other) const noexcept;

  /// @note The difference will be negative if lhs < rhs (lhs is this) and
  /// its absolute value fits into an int64_t, otherwise it
  /// will be positive and follow the rules of modular arithmetic of unsigned
  /// types This is intended and includes the case were rhs is "very close to 0"
  /// and and lhs is "close" to the MAX of uint64_t (MAX=2^64-1). Here close
  /// means that the real absolute difference would be larger than 2^63. This is
  /// excactly the right behaviour to deal with a (theoretically possible)
  /// overflow of lhs and can be seen as lhs being interpreted as MAX + its
  /// actual value. In this case, lhs - rhs is positive even though lhs < rhs.
  int64_t operator-(
      const CyclicIndex<CycleLength, ValueType>& rhs) const noexcept;

 private:
  ValueType m_value{0U};
};

}  // namespace entity
}  // namespace shm