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

template <uint64_t CycleLength, typename ValueType>
CyclicIndex<CycleLength, ValueType>::CyclicIndex(ValueType value) noexcept
    : m_value(value) {}

template <uint64_t CycleLength, typename ValueType>
CyclicIndex<CycleLength, ValueType>::CyclicIndex(ValueType index,
                                                 ValueType cycle) noexcept
    : CyclicIndex(index + cycle * CycleLength) {}

template <uint64_t CycleLength, typename ValueType>
ValueType CyclicIndex<CycleLength, ValueType>::getIndex() const noexcept {
  return m_value % CycleLength;
}

template <uint64_t CycleLength, typename ValueType>
ValueType CyclicIndex<CycleLength, ValueType>::getCycle() const noexcept {
  return m_value / CycleLength;
}

template <uint64_t CycleLength, typename ValueType>
ValueType CyclicIndex<CycleLength, ValueType>::getValue() const noexcept {
  return m_value;
}

template <uint64_t CycleLength, typename ValueType>
CyclicIndex<CycleLength, ValueType>
CyclicIndex<CycleLength, ValueType>::operator+(
    const ValueType value) const noexcept {
  // if we were at this value, we would have no overflow, i.e. when m_value is
  // larger there is an overflow
  auto delta = MAX_VALUE - value;
  if (delta < m_value) {
    // overflow, rare case (overflow by m_value - delta)
    // we need to compute the correct index and cycle we are in after overflow
    // note that we could also limit the max value to always start at
    // OVERFLOW_START_INDEX = 0, but this has other drawbacks (and the overflow
    // will not occur often if at all with 64 bit)
    delta = m_value - delta - 1;
    return CyclicIndex(OVERFLOW_START_INDEX + delta);
  }

  // no overflow, regular case
  return CyclicIndex(m_value + value);
}

template <uint64_t CycleLength, typename ValueType>
CyclicIndex<CycleLength, ValueType> CyclicIndex<CycleLength, ValueType>::next()
    const noexcept {
  if (m_value == MAX_VALUE) {
    return CyclicIndex(OVERFLOW_START_INDEX);
  }
  return CyclicIndex(m_value + 1);
}

template <uint64_t CycleLength, typename ValueType>
bool CyclicIndex<CycleLength, ValueType>::isOneCycleBehind(
    const CyclicIndex& other) const noexcept {
  auto thisCycle = this->getCycle();
  auto otherCycle = other.getCycle();

  if (thisCycle == MAX_CYCLE) {
    return otherCycle == 0;
  }
  return (thisCycle + 1 == otherCycle);
}

template <uint64_t CycleLength, typename ValueType>
int64_t CyclicIndex<CycleLength, ValueType>::operator-(
    const CyclicIndex<CycleLength, ValueType>& rhs) const noexcept {
  return static_cast<int64_t>(m_value - rhs.m_value);
}

}  // namespace entity
}  // namespace shm