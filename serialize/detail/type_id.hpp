

#pragma once

#include <climits>
#include <cstdint>
#include <limits>
#include <type_traits>

#include "reflection.hpp"

namespace serialize {
namespace detail {

enum class type_id {
  // compatible template type
  unsupported_t = 0,
  // fundamental integral type
  int32_t = 1,
  uint32_t,
  int64_t,
  uint64_t,
  int8_t,
  uint8_t,
  int16_t,
  uint16_t,
  int128_t,  // Tips: We only support 128-bit integer on gcc clang
  uint128_t,
  bool_t,
  char_8_t,
  char_16_t,
  char_32_t,
  w_char_t,  // Note: this type is not portable!Enable it with marco
             // STRUCT_PACK_ENABLE_UNPORTABLE_TYPE
  // fundamental float type
  float16_t,  // TODO: wait for C++23 standard float type
  float32_t,
  float64_t,
  float128_t,
  v_int32_t,   // variable size int
  v_int64_t,   // variable size int
  v_uint32_t,  // variable size unsigned int
  v_uint64_t,  // variable size unsigned int
  // template type
  string_t = 128,
  array_t,
  vector_t,
  map_container_t,
  set_container_t,
  container_t,
  pair_t,
  tuple_t,
  optional_t,
  variant_t,
  expected_t,
  bitset_t,
  polymorphic_unique_ptr_t,
  // monostate, or void
  monostate_t = 250,
  // circle_flag
  circle_flag = 251,
  // end helper with user defined type ID
  type_end_flag_with_id = 252,
  // class type
  struct_t = 253,
  // end helper
  type_end_flag = 255,
};

template <typename T,
          std::enable_if_t<std::is_same<int32_t, T>::value, int> = 0>
constexpr type_id get_integral_type() {
  return type_id::int32_t;
}

template <typename T,
          std::enable_if_t<std::is_same<uint32_t, T>::value, int> = 0>
constexpr type_id get_integral_type() {
  return type_id::uint32_t;
}

template <typename T, std::enable_if_t<std::is_same<int64_t, T>::value ||
                                           (sizeof(long long) == 8 &&
                                            std::is_same<T, long long>::value),
                                       int> = 0>
constexpr type_id get_integral_type() {
  return type_id::int64_t;
}

template <typename T,
          std::enable_if_t<std::is_same<uint64_t, T>::value ||
                               (sizeof(long long) == 8 &&
                                std::is_same<T, unsigned long long>::value),
                           int> = 0>
constexpr type_id get_integral_type() {
  return type_id::uint64_t;
}

template <typename T, std::enable_if_t<std::is_same<int8_t, T>::value ||
                                           std::is_same<signed char, T>::value,
                                       int> = 0>
constexpr type_id get_integral_type() {
  return type_id::int8_t;
}

template <typename T,
          std::enable_if_t<std::is_same<uint8_t, T>::value ||
                               std::is_same<unsigned char, T>::value,
                           int> = 0>
constexpr type_id get_integral_type() {
  return type_id::uint8_t;
}

template <typename T,
          std::enable_if_t<std::is_same<int16_t, T>::value, int> = 0>
constexpr type_id get_integral_type() {
  return type_id::int16_t;
}

template <typename T,
          std::enable_if_t<std::is_same<uint16_t, T>::value, int> = 0>
constexpr type_id get_integral_type() {
  return type_id::uint16_t;
}

template <typename T, std::enable_if_t<std::is_same<char, T>::value
#ifdef __cpp_lib_char8_t
                                           || std::is_same<char8_t, T>::value
#endif
                                       ,
                                       int> = 0>
constexpr type_id get_integral_type() {
  return type_id::char_8_t;
}

template <typename T,
          std::enable_if_t<std::is_same<wchar_t, T>::value, int> = 0>
constexpr type_id get_integral_type() {
  return type_id::w_char_t;
}

template <typename T, std::enable_if_t<std::is_same<char16_t, T>::value &&
                                           sizeof(char16_t) == 2,
                                       int> = 0>
constexpr type_id get_integral_type() {
  static_assert(sizeof(char16_t) == 2,
                "sizeof(char16_t) != 2, which is not supported.");
  return type_id::char_16_t;
}
template <typename T, std::enable_if_t<std::is_same<char32_t, T>::value &&
                                           sizeof(char32_t) == 4,
                                       int> = 0>
constexpr type_id get_integral_type() {
  static_assert(sizeof(char32_t) == 4,
                "sizeof(char32_t) != 4, which is not supported.");
  return type_id::char_32_t;
}

template <
    typename T,
    std::enable_if_t<std::is_same<bool, T>::value && sizeof(bool), int> = 0>
constexpr type_id get_integral_type() {
  static_assert(sizeof(bool) == 1,
                "sizeof(bool) != 1, which is not supported.");
  return type_id::bool_t;
}

#if (__GNUC__ || __clang__)
template <typename T,
          std::enable_if_t<std::is_same<__int128, T>::value, int> = 0>
constexpr type_id get_integral_type() {
  return type_id::int128_t;
}

template <typename T,
          std::enable_if_t<std::is_same<unsigned __int128, T>::value, int> = 0>
constexpr type_id get_integral_type() {
  return type_id::uint128_t;
}
#endif

template <typename T,
          std::enable_if_t<std::is_same<float, T>::value &&
                               (!std::numeric_limits<float>::is_iec559 ||
                                sizeof(float) != 4),
                           int> = 0>
constexpr type_id get_floating_point_type() {
  static_assert(!sizeof(T),
                "The float type in this machine is not standard IEEE 754 "
                "32bits float point!");
  return type_id::type_end_flag;
}

template <typename T,
          std::enable_if_t<std::is_same<float, T>::value &&
                               !(!std::numeric_limits<float>::is_iec559 ||
                                 sizeof(float) != 4),
                           int> = 0>
constexpr type_id get_floating_point_type() {
  return type_id::float32_t;
}

template <typename T,
          std::enable_if_t<std::is_same<double, T>::value &&
                               !(!std::numeric_limits<double>::is_iec559 ||
                                 sizeof(double) != 8),
                           int> = 0>
constexpr type_id get_floating_point_type() {
  return type_id::float64_t;
}

template <typename T,
          std::enable_if_t<std::is_same<double, T>::value &&
                               (!std::numeric_limits<double>::is_iec559 ||
                                sizeof(double) != 8),
                           int> = 0>
constexpr type_id get_floating_point_type() {
  static_assert(!sizeof(T),
                "The double type in this machine is not standard IEEE 754 "
                "64bits float point!");
  return type_id::type_end_flag;
}

template <typename T,
          std::enable_if_t<std::is_same<long double, T>::value &&
                               !(!std::numeric_limits<long double>::is_iec559 ||
                                 sizeof(long double) != 16),
                           int> = 0>
constexpr type_id get_floating_point_type() {
  return type_id::float128_t;
}

template <typename T,
          std::enable_if_t<std::is_same<long double, T>::value &&
                               (!std::numeric_limits<long double>::is_iec559 ||
                                sizeof(long double) != 16),
                           int> = 0>
constexpr type_id get_floating_point_type() {
  static_assert(!sizeof(T),
                "The long double type in this machine is not standard IEEE 754 "
                "128bits float point!");
  return type_id::type_end_flag;
}

template <typename T, std::enable_if_t<std::is_enum<T>::value, int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return get_integral_type<std::underlying_type_t<T>>();
}

template <typename T,
          std::enable_if_t<std::is_integral<T>::value
#if (__GNUC__ || __clang__)
                               || std::is_same<__int128, T>::value ||
                               std::is_same<unsigned __int128, T>::value
#endif
                           ,
                           int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return get_integral_type<T>();
}

template <typename T,
          std::enable_if_t<std::is_floating_point<T>::value, int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return get_floating_point_type<T>();
}

template <typename T, std::enable_if_t<std::is_same<void, T>::value ||
                                           std::is_abstract<T>::value,
                                       int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return type_id::monostate_t;
}

template <typename T, std::enable_if_t<bitset<T>, int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return type_id::bitset_t;
}

template <typename T, std::enable_if_t<string<T>, int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return type_id::string_t;
}
template <typename T,
          std::enable_if_t<array<T> || c_array<T> || static_span<T>, int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return type_id::array_t;
}

template <typename T, std::enable_if_t<map_container<T>, int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return type_id::map_container_t;
}

template <typename T, std::enable_if_t<set_container<T>, int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return type_id::set_container_t;
}

// TODO
template <typename T,
          std::enable_if_t<container<T> && !string<T> && !map_container<T> &&
                               !set_container<T> &&
                               !(array<T> || c_array<T> || static_span<T>),
                           int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return type_id::container_t;
}

template <typename T, std::enable_if_t<optional<T>, int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return type_id::optional_t;
}

template <typename T, std::enable_if_t<expected<T>, int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return type_id::expected_t;
}

// TODO tuple  会判定为 struct
template <typename T, std::enable_if_t<user_struct<T>, int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return type_id::struct_t;
}

template <typename T, std::enable_if_t<pair<T>, int> = 0>
constexpr type_id get_type_id() {
  static_assert(CHAR_BIT == 8, "");
  return type_id::pair_t;
}

// TODO unsupported

}  // namespace detail
}  // namespace serialize