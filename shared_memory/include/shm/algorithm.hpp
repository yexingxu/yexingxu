#pragma once

#include <spdlog/spdlog.h>

#include <cstdlib>
#include <string>

namespace shm {
namespace details {

void Require(const bool condition, const char* file, const int line,
             const char* function, const char* conditionString) noexcept;

#define EXPECTS(condition)                                                  \
  shm::details::Require(condition, __FILE__, __LINE__, __PRETTY_FUNCTION__, \
                        #condition)

#define ENSURES(condition)                                                  \
  shm::details::Require(condition, __FILE__, __LINE__, __PRETTY_FUNCTION__, \
                        #condition)

template <typename T, T Minimum>
struct greater_or_equal {
 public:
  // AXIVION Next Construct AutosarC++19_03-A12.1.4: this class should behave
  // like a T but which never can be less than Minimum. Adding explicit would
  // defeat the purpose. NOLINTNEXTLINE(hicpp-explicit-conversions)
  greater_or_equal(T t) noexcept : m_value(t) { EXPECTS(t >= Minimum); }

  // AXIVION Next Construct AutosarC++19_03-A13.5.2,AutosarC++19_03-A13.5.3:this
  // class should behave like a T but which never can be less than Minimum.
  // Adding explicit would defeat the purpose.
  // NOLINTNEXTLINE(hicpp-explicit-conversions)
  constexpr operator T() const noexcept { return m_value; }

 private:
  T m_value;
};

template <typename T, T Minimum, T Maximum>
struct range {
 public:
  // AXIVION Next Construct AutosarC++19_03-A12.1.4: this class should behave
  // like a T but with values only in range [Minimum, Maximum] Adding explicit
  // would defeat the purpose. NOLINTNEXTLINE(hicpp-explicit-conversions)
  range(T t) noexcept : m_value(t) {
    // AXIVION Next Construct AutosarC++19_03-A1.4.3 : False positive! 't >=
    // Minimum' depends on input parameter
    EXPECTS((t >= Minimum) && (t <= Maximum));
  }

  // AXIVION Next Construct AutosarC++19_03-A13.5.2, AutosarC++19_03-A13.5.3:
  // this class should behave like a T but with values only in range [Minimum,
  // Maximum]. Adding explicit would defeat the purpose.
  // NOLINTNEXTLINE(hicpp-explicit-conversions)
  constexpr operator T() const noexcept { return m_value; }

 private:
  T m_value;
};

template <typename T,
          typename =
              typename std::enable_if<std::is_pointer<T>::value, void>::type>
struct not_null {
 public:
  // this class should behave like a pointer which never can be nullptr, adding
  // explicit would defeat the purpose
  // NOLINTNEXTLINE(hicpp-explicit-conversions)
  not_null(T t) noexcept : m_value(t) { EXPECTS(t != nullptr); }

  // AXIVION Next Construct AutosarC++19_03-A13.5.2,AutosarC++19_03-A13.5.3:this
  // should behave like a pointer which never can be nullptr, adding explicit
  // would defeat the purpose NOLINTNEXTLINE(hicpp-explicit-conversions)
  constexpr operator T() const noexcept { return m_value; }

 private:
  T m_value;
};

/// @brief Returns true if T is equal to CompareType, otherwise false
/// @param T type to compare to
/// @param CompareType the type to which T is compared
/// @return true if the types T and CompareType are equal, otherwise false
template <typename T, typename CompareType>
constexpr bool doesContainType() noexcept;

/// @brief Returns true if T is contained the provided type list
/// @param T type to compare to
/// @param CompareType, Next, Remainder the type list in which T should be
/// contained
/// @return true if the T is contained in the type list, otherwise false
template <typename T, typename CompareType, typename Next,
          typename... Remainder>
constexpr bool doesContainType() noexcept;

template <typename T, typename CompareType>
inline constexpr bool doesContainType() noexcept {
  return std::is_same<T, CompareType>::value;
}

template <typename T, typename CompareType, typename Next,
          typename... Remainder>
inline constexpr bool doesContainType() noexcept {
  return doesContainType<T, CompareType>() ||
         doesContainType<T, Next, Remainder...>();
}

template <typename T>
inline constexpr bool doesContainValue(const T) noexcept {
  return false;
}

template <typename T1, typename T2, typename... ValueList>
inline constexpr bool doesContainValue(
    const T1 value, const T2 firstValueListEntry,
    const ValueList... remainingValueListEntries) noexcept {
  // AXIVION Next Line AutosarC++19_03-M6.2.2 : intentional check for exact
  // equality
  return (value == firstValueListEntry)
             ? true
             : doesContainValue(value, remainingValueListEntries...);
}

/// @brief Checks if an unsigned integer is a power of two
/// @return true if power of two, otherwise false
template <typename T>
constexpr bool isPowerOfTwo(const T n) noexcept {
  static_assert(std::is_unsigned<T>::value && !std::is_same<T, bool>::value,
                "Only unsigned integer are allowed!");
  // AXIVION Next Construct AutosarC++19_03-M0.1.2, AutosarC++19_03-M0.1.9,
  // FaultDetection-DeadBranches : False positive! 'n' can be zero.
  return (n > 0) && ((n & (n - 1U)) == 0U);
}

template <typename T>
inline constexpr T maxVal(const T& left) noexcept {
  return left;
}

template <typename T>
inline constexpr T maxVal(const T& left, const T& right) noexcept {
  return (right < left) ? left : right;
}

template <typename T, typename... Targs>
inline constexpr T maxVal(const T& left, const T& right,
                          const Targs&... args) noexcept {
  return maxVal(maxVal(left, right), args...);
}

template <typename T>
inline constexpr T minVal(const T& left) noexcept {
  return left;
}

template <typename T>
inline constexpr T minVal(const T& left, const T& right) noexcept {
  return (left < right) ? left : right;
}

template <typename T, typename... Targs>
inline constexpr T minVal(const T& left, const T& right,
                          const Targs&... args) noexcept {
  return minVal(minVal(left, right), args...);
}

}  // namespace details
}  // namespace shm
