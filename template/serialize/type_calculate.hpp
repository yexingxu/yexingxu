/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-28 15:56:49
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-29 08:04:39
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <type_traits>

#include "md5_constexpr.hpp"
#include "reflection.hpp"
#include "type_id.hpp"

#ifndef SERIALIZE_DETAILS_TYPE_CALCULATE_HPP_
#define SERIALIZE_DETAILS_TYPE_CALCULATE_HPP_

namespace serialize {
template <typename... Args>
inline constexpr decltype(auto) get_type_literal();

namespace details {

template <size_t size,
          std::enable_if_t<size<1ull * 127, int> = 0> constexpr decltype(auto)
              get_size_literal() {
  static_assert(sizeof(size_t) <= 8, "");
  return string_literal<char, 1>{{static_cast<char>(size + 129)}};
}

template <size_t size,
          std::enable_if_t<size<1ull * 127 * 127 && size >= 1ull * 127, int> =
                               0> constexpr decltype(auto) get_size_literal() {
  static_assert(sizeof(size_t) <= 8, "");
  return string_literal<char, 2>{
      {static_cast<char>(size % 127 + 1), static_cast<char>(size / 127 + 129)}};
}

template <size_t size,
          std::enable_if_t<
              size<1ull * 127 * 127 * 127 && size >= 1ull * 127 * 127, int> =
                  0> constexpr decltype(auto) get_size_literal() {
  static_assert(sizeof(size_t) <= 8, "");
  return string_literal<char, 3>{{static_cast<char>(size % 127 + 1),
                                  static_cast<char>(size / 127 % 127 + 1),
                                  static_cast<char>(size / (127 * 127) + 129)}};
}

template <size_t size, std::enable_if_t<size<1ull * 127 * 127 * 127 * 127 &&
                                                 size >= 1ull * 127 * 127 * 127,
                                             int> = 0> constexpr decltype(auto)
                           get_size_literal() {
  static_assert(sizeof(size_t) <= 8, "");
  return string_literal<char, 4>{
      {static_cast<char>(size % 127 + 1),
       static_cast<char>(size / 127 % 127 + 1),
       static_cast<char>(size / (127 * 127) % 127 + 1),
       static_cast<char>(size / (127 * 127 * 127) + 129)}};
}

template <size_t size,
          std::enable_if_t<size<1ull * 127 * 127 * 127 * 127 * 127 &&
                                    size >= 1ull * 127 * 127 * 127 * 127,
                                int> = 0> constexpr decltype(auto)
              get_size_literal() {
  static_assert(sizeof(size_t) <= 8, "");
  return string_literal<char, 5>{
      {static_cast<char>(size % 127 + 1),
       static_cast<char>(size / 127 % 127 + 1),
       static_cast<char>(size / (127 * 127) % 127 + 1),
       static_cast<char>(size / (127 * 127 * 127) % 127 + 1),
       static_cast<char>(size / (127 * 127 * 127 * 127) + 129)}};
}

template <size_t size,
          std::enable_if_t<size<1ull * 127 * 127 * 127 * 127 * 127 * 127 &&
                                    size >= 1ull * 127 * 127 * 127 * 127 * 127,
                                int> = 0> constexpr decltype(auto)
              get_size_literal() {
  static_assert(sizeof(size_t) <= 8, "");
  return string_literal<char, 6>{
      {static_cast<char>(size % 127 + 1),
       static_cast<char>(size / 127 % 127 + 1),
       static_cast<char>(size / (127 * 127) % 127 + 1),
       static_cast<char>(size / (127 * 127 * 127) % 127 + 1),
       static_cast<char>(size / (127 * 127 * 127 * 127) % 127 + 1),
       static_cast<char>(size / (127 * 127 * 127 * 127 * 127) + 129)}};
}

template <size_t size,
          std::enable_if_t<
              size<1ull * 127 * 127 * 127 * 127 * 127 * 127 * 127 &&
                       size >= 1ull * 127 * 127 * 127 * 127 * 127 * 127,
                   int> = 0> constexpr decltype(auto) get_size_literal() {
  static_assert(sizeof(size_t) <= 8, "");
  return string_literal<char, 7>{
      {static_cast<char>(size % 127 + 1),
       static_cast<char>(size / 127 % 127 + 1),
       static_cast<char>(size / (127 * 127) % 127 + 1),
       static_cast<char>(size / (127 * 127 * 127) % 127 + 1),
       static_cast<char>(size / (127 * 127 * 127 * 127) % 127 + 1),
       static_cast<char>(size / (127 * 127 * 127 * 127 * 127) % 127 + 1),
       static_cast<char>(size / (127 * 127 * 127 * 127 * 127 * 127) + 129)}};
}

template <size_t size,
          std::enable_if_t<
              size<1ull * 127 * 127 * 127 * 127 * 127 * 127 * 127 * 127 &&
                       size >= 1ull * 127 * 127 * 127 * 127 * 127 * 127 * 127,
                   int> = 0> constexpr decltype(auto) get_size_literal() {
  static_assert(sizeof(size_t) <= 8, "");
  return string_literal<char, 8>{{
      static_cast<char>(size % 127 + 1),
      static_cast<char>(size / 127 % 127 + 1),
      static_cast<char>(size / (127 * 127) % 127 + 1),
      static_cast<char>(size / (127 * 127 * 127) % 127 + 1),
      static_cast<char>(size / (127 * 127 * 127 * 127) % 127 + 1),
      static_cast<char>(size / (127 * 127 * 127 * 127 * 127) % 127 + 1),
      static_cast<char>(size / (127 * 127 * 127 * 127 * 127 * 127) % 127 + 1),
      static_cast<char>(size / (127 * 127 * 127 * 127 * 127 * 127 * 127) + 129),
  }};
}

template <size_t size, std::enable_if_t<size >= 1ull * 127 * 127 * 127 * 127 *
                                                    127 * 127 * 127 * 127,
                                        int> = 0>
constexpr decltype(auto) get_size_literal() {
  static_assert(
      size >= 1ull * 127 * 127 * 127 * 127 * 127 * 127 * 127 * 127 * 127,
      "The size is too large.");
}

template <typename Arg, typename... ParentArgs, std::size_t... I>
constexpr std::size_t check_circle_impl(std::index_sequence<I...>) {
  using types_tuple = std::tuple<ParentArgs...>;
  return (std::max)({(
      std::is_same<std::tuple_element_t<I, types_tuple>, Arg>::value ? I + 1
                                                                     : 0)...});
}

template <typename Arg, typename... ParentArgs,
          std::enable_if_t<sizeof...(ParentArgs) != 0, int> = 0>
constexpr std::size_t check_circle() {
  return check_circle_impl<Arg, ParentArgs...>(
      std::make_index_sequence<sizeof...(ParentArgs)>());
}
template <typename Arg, typename... ParentArgs,
          std::enable_if_t<sizeof...(ParentArgs) == 0, int> = 0>
constexpr std::size_t check_circle() {
  return 0;
}

template <typename T>
struct get_array_element {
  using type = typename T::value_type;
};

template <typename T, std::size_t sz>
struct get_array_element<T[sz]> {
  using type = T;
};

template <typename T, std::enable_if_t<array<T> || c_array<T>, int> = 0>
std::size_t constexpr get_array_size() {
  return sizeof(T) / sizeof(typename get_array_element<T>::type);
}

template <typename T, std::enable_if_t<bitset<T>, int> = 0>
std::size_t constexpr get_array_size() {
  return T{}.size();
}

template <typename T,
          std::enable_if_t<!(array<T> || c_array<T>)&&!bitset<T>, int> = 0>
std::size_t constexpr get_array_size() {
  return T::extent;
}

template <typename Arg, std::enable_if_t<has_user_defined_id<Arg>, int> = 0>
constexpr decltype(auto) get_type_end_flag() {
  return string_literal<char, 1>{
             {static_cast<char>(type_id::type_end_flag_with_id)}} +
         get_size_literal<Arg::struct_pack_id>();
}

template <typename Arg, std::enable_if_t<has_user_defined_id_ADL<Arg>, int> = 0>
constexpr decltype(auto) get_type_end_flag() {
  return string_literal<char, 1>{
             {static_cast<char>(type_id::type_end_flag_with_id)}} +
         get_size_literal<struct_pack_id((Arg *)nullptr)>();
}

template <typename Arg, std::enable_if_t<!has_user_defined_id<Arg> ||
                                             !has_user_defined_id_ADL<Arg>,
                                         int> = 0>
constexpr decltype(auto) get_type_end_flag() {
  return string_literal<char, 1>{{static_cast<char>(type_id::type_end_flag)}};
}

template <typename Args, typename... ParentArgs, std::size_t... I>
constexpr decltype(auto) get_type_literal(std::index_sequence<I...>);

template <typename Args, typename... ParentArgs, std::size_t... I>
constexpr decltype(auto) get_variant_literal(std::index_sequence<I...>);

template <typename Arg, typename... ParentArgs,
          std::enable_if_t<is_trivial_view_v<Arg>, int> = 0>
constexpr decltype(auto) get_type_literal() {
  return get_type_literal<typename Arg::value_type, ParentArgs...>();
}

template <typename Arg, typename... ParentArgs,
          std::enable_if_t<!is_trivial_view_v<Arg>, int> = 0>
constexpr decltype(auto) get_type_literal() {
  constexpr std::size_t has_cycle = check_circle<Arg, ParentArgs...>();
  static_assert(has_cycle >= 2, "");  // TODO

  // return get_type_literal<typename Arg::value_type, ParentArgs...>();
}

template <typename T>
constexpr decltype(auto) get_type_literal() {
  return string_literal<char, 1>{};
}

template <typename Args, typename... ParentArgs, std::size_t... I>
constexpr decltype(auto) get_type_literal(std::index_sequence<I...>) {
  string_literal<char, 0> tmp{};  // TODO
  static_cast<void>(std::initializer_list<int>{
      (tmp += get_type_literal<remove_cvref_t<std::tuple_element_t<I, Args>>,
                               ParentArgs...>(),
       0)...});

  return tmp;
}

// template <typename Args, typename... ParentArgs, std::size_t... I>
// constexpr decltype(auto) get_variant_literal(std::index_sequence<I...>) {
//   return ((get_type_literal<remove_cvref_t<std::variant_alternative_t<I,
//   Args>>,
//                             Args, ParentArgs...>()) +
//           ...);
// }

// TODO
// template <typename Parent, typename... Args,
//           std::enable_if_t<std::is_same<Parent, void>::value, int> = 0>
// constexpr decltype(auto) get_types_literal_impl() {
//   return (get_type_literal<Args>() + ...);
// }

// template <typename Parent, typename... Args,
//           std::enable_if_t<!std::is_same<Parent, void>::value, int> = 0>
// constexpr decltype(auto) get_types_literal_impl() {
//   return (get_type_literal<Args, Parent>() + ...);
// }

template <typename T, typename Tuple, std::size_t... I>
constexpr decltype(auto) get_types_literal(std::index_sequence<I...>) {
  return get_types_literal<T,
                           remove_cvref_t<std::tuple_element_t<I, Tuple>>...>();
}

template <uint64_t version, typename Args, typename... ParentArgs>
constexpr bool check_if_compatible_element_exist_impl_helper();

template <uint64_t version, typename Args, typename... ParentArgs,
          std::size_t... I>
constexpr bool check_if_compatible_element_exist_impl(
    std::index_sequence<I...>) {
  bool ret = false;
  static_cast<void>(std::initializer_list<int>{
      (ret |= check_if_compatible_element_exist_impl_helper<
           version, remove_cvref_t<std::tuple_element_t<I, Args>>,
           ParentArgs...>(),
       0)...});
  return ret;
}

// template <uint64_t version, typename Arg, typename... ParentArgs,
//           std::size_t... I>
// constexpr bool check_if_compatible_element_exist_impl_variant(
//     std::index_sequence<I...>) {
//   return (check_if_compatible_element_exist_impl_helper<
//               version, remove_cvref_t<std::variant_alternative_t<I, Arg>>,
//               ParentArgs...>() ||
//           ...);
// }

template <uint64_t version, typename Arg, typename... ParentArgs,
          std::enable_if_t<is_trivial_view_v<Arg>, int> = 0>
constexpr bool check_if_compatible_element_exist_impl_helper() {
  using T = remove_cvref_t<Arg>;
  constexpr auto id = get_type_id<T>();
  return check_if_compatible_element_exist_impl_helper<
      version, typename Arg::value_type, ParentArgs...>();
}

template <uint64_t version, typename Arg, typename... ParentArgs,
          std::enable_if_t<check_circle<Arg, ParentArgs...>() != 0, int> = 0>
constexpr bool check_if_compatible_element_exist_impl_helper() {
  return false;
}

// template <uint64_t version, typename Arg, typename... ParentArgs>
// constexpr bool check_if_compatible_element_exist_impl_helper() {
//   using T = remove_cvref_t<Arg>;
//   constexpr auto id = get_type_id<T>();
//   if constexpr (is_trivial_view_v<Arg>) {
//     return check_if_compatible_element_exist_impl_helper<
//         version, typename Arg::value_type, ParentArgs...>();
//   } else if constexpr (check_circle<Arg, ParentArgs...>() != 0) {
//     return false;
//   } else if constexpr (id == type_id::compatible_t) {
//     if constexpr (version != UINT64_MAX)
//       return T::version_number == version;
//     else
//       return true;
//   } else {
//     if constexpr (id == type_id::struct_t) {
//       using subArgs = decltype(get_types<T>());
//       return check_if_compatible_element_exist_impl<version, subArgs, T,
//                                                     ParentArgs...>(
//           std::make_index_sequence<std::tuple_size_v<subArgs>>());
//     } else if constexpr (id == type_id::optional_t) {
//       if constexpr (unique_ptr<T>) {
//         if constexpr (is_base_class<typename T::element_type>) {
//           return check_if_compatible_element_exist_impl<
//               version, derived_class_set_t<typename T::element_type>, T,
//               ParentArgs...>();
//         } else {
//           return check_if_compatible_element_exist_impl_helper<
//               version, typename T::element_type, T, ParentArgs...>();
//         }
//       } else {
//         return check_if_compatible_element_exist_impl_helper<
//             version, typename T::value_type, T, ParentArgs...>();
//       }
//     } else if constexpr (id == type_id::array_t) {
//       return check_if_compatible_element_exist_impl_helper<
//           version, remove_cvref_t<typename get_array_element<T>::type>, T,
//           ParentArgs...>();
//     } else if constexpr (id == type_id::map_container_t) {
//       return check_if_compatible_element_exist_impl_helper<
//                  version, typename T::key_type, T, ParentArgs...>() ||
//              check_if_compatible_element_exist_impl_helper<
//                  version, typename T::mapped_type, T, ParentArgs...>();
//     } else if constexpr (id == type_id::set_container_t ||
//                          id == type_id::container_t) {
//       return check_if_compatible_element_exist_impl_helper<
//           version, typename T::value_type, T, ParentArgs...>();
//     } else if constexpr (id == type_id::expected_t) {
//       return check_if_compatible_element_exist_impl_helper<
//                  version, typename T::value_type, T, ParentArgs...>() ||
//              check_if_compatible_element_exist_impl_helper<
//                  version, typename T::error_type, T, ParentArgs...>();
//     } else if constexpr (id == type_id::variant_t) {
//       return check_if_compatible_element_exist_impl_variant<version, T, T,
//                                                             ParentArgs...>(
//           std::make_index_sequence<std::variant_size_v<T>>{});
//     } else {
//       return false;
//     }
//   }
// }

template <typename T, typename... Args>
constexpr uint32_t get_types_code_impl() {
  constexpr auto str = get_types_literal<T, remove_cvref_t<Args>...>();
  return MD5::MD5Hash32Constexpr(str.data(), str.size()) & 0xFFFFFFFE;
}

template <typename T, typename Tuple, size_t... I>
constexpr uint32_t get_types_code(std::index_sequence<I...>) {
  return get_types_code_impl<T, std::tuple_element_t<I, Tuple>...>();
}

template <typename T>
constexpr uint32_t get_types_code() {
  using tuple_t = decltype(get_types<T>());
  return get_types_code<T, tuple_t>(
      std::make_index_sequence<std::tuple_size<tuple_t>::value>{});
}

template <typename Args, typename... ParentArgs>
constexpr std::size_t calculate_compatible_version_size();

template <typename Buffer, typename Args, typename... ParentArgs>
constexpr void get_compatible_version_numbers(Buffer &buffer, std::size_t &sz);

template <typename T>
constexpr auto inline get_sorted_compatible_version_numbers() {
  std::array<uint64_t, calculate_compatible_version_size<T>()> buffer{};
  std::size_t sz = 0;
  get_compatible_version_numbers<decltype(buffer), T>(buffer, sz);
  compile_time_sort(buffer);
  return buffer;
}

template <typename T>
constexpr auto inline get_sorted_and_uniqued_compatible_version_numbers() {
  constexpr auto buffer = get_sorted_compatible_version_numbers<T>();
  std::array<uint64_t, calculate_uniqued_size(buffer)> uniqued_buffer{};
  compile_time_unique(buffer, uniqued_buffer);
  return uniqued_buffer;
}

template <typename T>
constexpr auto compatible_version_number =
    get_sorted_and_uniqued_compatible_version_numbers<T>();

template <typename T, uint64_t version = UINT64_MAX>
constexpr bool check_if_compatible_element_exist() {
  using U = remove_cvref_t<T>;
  return details::check_if_compatible_element_exist_impl<version, U>(
      std::make_index_sequence<std::tuple_size<U>::value>{});
}

template <typename T, uint64_t version = UINT64_MAX>
constexpr bool exist_compatible_member =
    check_if_compatible_element_exist<decltype(get_types<T>()), version>();
// clang-format off
template <typename T, uint64_t version = UINT64_MAX>
constexpr bool unexist_compatible_member = !
exist_compatible_member<decltype(get_types<T>()), version>;
// clang-format on

template <typename Args, typename... ParentArgs, std::size_t... I>
constexpr std::size_t calculate_compatible_version_size(
    std::index_sequence<I...>) {
  std::size_t ret = 0;
  static_cast<void>(std::initializer_list<int>{
      (ret += calculate_compatible_version_size<
           remove_cvref_t<std::tuple_element_t<I, Args>>, ParentArgs...>(),
       0)...});
  return ret;
}

// TODO variant
// template <typename Arg, typename... ParentArgs, std::size_t... I>
// constexpr std::size_t calculate_variant_compatible_version_size(
//     std::index_sequence<I...>) {
//   return (calculate_compatible_version_size<
//               remove_cvref_t<std::variant_alternative<I, Arg>::type>,
//               ParentArgs...>() +
//           ...);
// }

// template <typename Arg, typename... ParentArgs>
// constexpr std::size_t calculate_compatible_version_size() {
//   using T = remove_cvref_t<Arg>;
//   constexpr auto id = get_type_id<T>();
//   std::size_t sz = 0;
//   if constexpr (is_trivial_view_v<T>) {
//     return 0;
//   }
//   if constexpr (check_circle<T, ParentArgs...>())
//     sz = 0;
//   else if constexpr (id == type_id::compatible_t) {
//     sz = 1;
//   } else {
//     if constexpr (id == type_id::struct_t) {
//       using subArgs = decltype(get_types<T>());
//       return calculate_compatible_version_size<subArgs, T, ParentArgs...>(
//           std::make_index_sequence<std::tuple_size_v<subArgs>>());
//     } else if constexpr (id == type_id::optional_t) {
//       if constexpr (unique_ptr<T>) {
//         if constexpr (is_base_class<typename T::element_type>) {
//           sz = calculate_compatible_version_size<
//               derived_class_set_t<typename T::element_type>, T,
//               ParentArgs...>();
//         } else {
//           sz = calculate_compatible_version_size<typename T::element_type, T,
//                                                  ParentArgs...>();
//         }
//       } else {
//         sz = calculate_compatible_version_size<typename T::value_type, T,
//                                                ParentArgs...>();
//       }
//     } else if constexpr (id == type_id::array_t) {
//       return calculate_compatible_version_size<
//           remove_cvref_t<typename get_array_element<T>::type>, T,
//           ParentArgs...>();
//     } else if constexpr (id == type_id::map_container_t) {
//       return calculate_compatible_version_size<typename T::key_type, T,
//                                                ParentArgs...>() +
//              calculate_compatible_version_size<typename T::mapped_type, T,
//                                                ParentArgs...>();
//     } else if constexpr (id == type_id::set_container_t ||
//                          id == type_id::container_t) {
//       return calculate_compatible_version_size<typename T::value_type, T,
//                                                ParentArgs...>();
//     } else if constexpr (id == type_id::expected_t) {
//       return calculate_compatible_version_size<typename T::value_type, T,
//                                                ParentArgs...>() +
//              calculate_compatible_version_size<typename T::error_type, T,
//                                                ParentArgs...>();
//     } else if constexpr (id == type_id::variant_t) {
//       return calculate_variant_compatible_version_size<T, T, ParentArgs...>(
//           std::make_index_sequence<std::variant_size_v<T>>{});
//     }
//   }
//   return sz;
// }

template <typename Buffer, typename Args, typename... ParentArgs,
          std::size_t... I>
constexpr void get_compatible_version_numbers(Buffer &buffer, std::size_t &sz,
                                              std::index_sequence<I...>) {
  static_cast<void>(std::initializer_list<int>{(
      get_compatible_version_numbers<
          Buffer, remove_cvref_t<std::tuple_element_t<I, Args>>, ParentArgs...>(
          buffer, sz),
      0)...});
}

// TODO variant
// template <typename Buffer, typename Arg, typename... ParentArgs,
//           std::size_t... I>
// constexpr void get_variant_compatible_version_numbers(
//     Buffer &buffer, std::size_t &sz, std::index_sequence<I...>) {
//   return (get_compatible_version_numbers<
//               Buffer, remove_cvref_t<std::variant_alternative_t<I, Arg>>,
//               ParentArgs...>(buffer, sz),
//           ...);
// }

// TODO
// template <typename Buffer, typename Arg, typename... ParentArgs>
// constexpr void get_compatible_version_numbers(Buffer &buffer, std::size_t
// &sz) {
//   using T = remove_cvref_t<Arg>;
//   constexpr auto id = get_type_id<T>();
//   if constexpr (is_trivial_view_v<T>) {
//     return;
//   } else if constexpr (check_circle<T, ParentArgs...>()) {
//     return;
//   } else if constexpr (id == type_id::compatible_t) {
//     buffer[sz++] = T::version_number;
//     return;
//   } else {
//     if constexpr (id == type_id::struct_t) {
//       using subArgs = decltype(get_types<T>());
//       get_compatible_version_numbers<Buffer, subArgs, T, ParentArgs...>(
//           buffer, sz,
//           std::make_index_sequence<std::tuple_size_v<subArgs>>());
//     } else if constexpr (id == type_id::optional_t) {
//       if constexpr (unique_ptr<T>) {
//         if constexpr (is_base_class<typename T::element_type>) {
//           sz = get_compatible_version_numbers<
//               Buffer, derived_class_set_t<typename T::element_type>, T,
//               ParentArgs...>(buffer, sz);
//         } else {
//           get_compatible_version_numbers<Buffer, typename T::element_type, T,
//                                          ParentArgs...>(buffer, sz);
//         }
//       } else {
//         get_compatible_version_numbers<Buffer, typename T::value_type, T,
//                                        ParentArgs...>(buffer, sz);
//       }
//     } else if constexpr (id == type_id::array_t) {
//       get_compatible_version_numbers<
//           Buffer, remove_cvref_t<typename get_array_element<T>::type>, T,
//           ParentArgs...>(buffer, sz);
//     } else if constexpr (id == type_id::map_container_t) {
//       get_compatible_version_numbers<Buffer, typename T::key_type, T,
//                                      ParentArgs...>(buffer, sz);
//       get_compatible_version_numbers<Buffer, typename T::mapped_type, T,
//                                      ParentArgs...>(buffer, sz);
//     } else if constexpr (id == type_id::set_container_t ||
//                          id == type_id::container_t) {
//       get_compatible_version_numbers<Buffer, typename T::value_type, T,
//                                      ParentArgs...>(buffer, sz);
//     } else if constexpr (id == type_id::expected_t) {
//       get_compatible_version_numbers<Buffer, typename T::value_type, T,
//                                      ParentArgs...>(buffer, sz);
//       get_compatible_version_numbers<Buffer, typename T::error_type, T,
//                                      ParentArgs...>(buffer, sz);
//     } else if constexpr (id == type_id::variant_t) {
//       get_variant_compatible_version_numbers<Buffer, T, T, ParentArgs...>(
//           buffer, sz, std::make_index_sequence<std::variant_size_v<T>>{});
//     }
//   }
// }

template <typename T>
struct serialize_static_config {
  static constexpr bool has_compatible = exist_compatible_member<T>;
#ifdef NDEBUG
  static constexpr bool has_type_literal = false;
#else
  static constexpr bool has_type_literal = true;
#endif
};

}  // namespace details

namespace details {

// TODO
template <uint64_t conf, typename T,
          std::enable_if_t<(conf & 0b11) == sp_config::DEFAULT, int> = 0>
constexpr bool check_if_add_type_literal() {
  return (conf & 0b11) == serialize_static_config<T>::has_type_literal;
}

template <uint64_t conf, typename T,
          std::enable_if_t<(conf & 0b11) != sp_config::DEFAULT, int> = 0>
constexpr bool check_if_add_type_literal() {
  return (conf & 0b11) == sp_config::ENABLE_TYPE_INFO;
}

template <typename Arg, typename... ParentArgs>
constexpr bool check_if_has_container();

template <typename Arg, typename... ParentArgs, std::size_t... I>
constexpr bool check_if_has_container_helper(std::index_sequence<I...>) {
  bool ret = true;
  static_cast<void>(std::initializer_list<int>{
      (ret |=
       check_if_has_container<remove_cvref_t<std::tuple_element_t<I, Arg>>,
                              ParentArgs...>(),
       0)...});

  return ret;
}

// template <typename Arg, typename... ParentArgs, std::size_t... I>
// constexpr bool check_if_has_container_variant_helper(
//     std::index_sequence<I...> idx) {
//   return ((check_if_has_container<
//               remove_cvref_t<std::variant_alternative_t<I, Arg>>, Arg,
//               ParentArgs...>()) ||
//           ...);
// }

// template <typename Arg, typename... ParentArgs>
// constexpr bool check_if_has_container() {
//   if constexpr (is_trivial_view_v<Arg>) {
//     return check_if_has_container<typename Arg::value_type, ParentArgs...>();
//   } else {
//     constexpr std::size_t has_cycle = check_circle<Arg, ParentArgs...>();
//     if constexpr (has_cycle != 0) {
//       return false;
//     } else {
//       constexpr auto id = get_type_id<Arg>();
//       if constexpr (id == type_id::struct_t) {
//         using Args = decltype(get_types<Arg>());
//         return check_if_has_container_helper<Args, Arg, ParentArgs...>(
//             std::make_index_sequence<std::tuple_size_v<Args>>());
//       } else if constexpr (id == type_id::variant_t) {
//         constexpr auto sz = std::variant_size_v<Arg>;
//         static_assert(sz > 0, "empty param of std::variant is not allowed!");
//         static_assert(sz < 256, "too many alternative type in variant!");
//         return check_if_has_container_variant_helper<Arg, ParentArgs...>(
//             std::make_index_sequence<std::variant_size_v<Arg>>());
//       } else if constexpr (id == type_id::array_t) {
//         return check_if_has_container<
//             remove_cvref_t<decltype(std::declval<Arg>()[0])>, Arg,
//             ParentArgs...>();
//       } else if constexpr (id == type_id::bitset_t) {
//         return false;
//       } else if constexpr (unique_ptr<Arg>) {
//         return check_if_has_container<
//             remove_cvref_t<typename Arg::element_type>, Arg,
//             ParentArgs...>();
//       } else if constexpr (id == type_id::container_t ||
//                            id == type_id::string_t ||
//                            id == type_id::set_container_t ||
//                            id == type_id::map_container_t) {
//         return true;
//       } else if constexpr (id == type_id::optional_t ||
//                            id == type_id::compatible_t) {
//         return check_if_has_container<remove_cvref_t<typename
//         Arg::value_type>,
//                                       Arg, ParentArgs...>();
//       } else if constexpr (id == type_id::expected_t) {
//         return check_if_has_container<remove_cvref_t<typename
//         Arg::value_type>,
//                                       Arg, ParentArgs...>() ||
//                check_if_has_container<remove_cvref_t<typename
//                Arg::error_type>,
//                                       Arg, ParentArgs...>();
//       } else {
//         return false;
//       }
//     }
//   }
// }

template <uint64_t conf, typename T,
          std::enable_if_t<(conf & 0b11) != sp_config::DISABLE_ALL_META_INFO,
                           int> = 0>
constexpr bool check_if_disable_hash_head_impl() {
  return false;
}

template <uint64_t conf, typename T,
          std::enable_if_t<(conf & 0b11) == sp_config::DISABLE_ALL_META_INFO,
                           int> = 0>
constexpr bool check_if_disable_hash_head_impl() {
  return true;
}

// template <uint64_t conf, typename T>
// constexpr bool check_if_disable_hash_head_impl() {
//   constexpr auto config = conf & 0b11;
//   if constexpr (config != sp_config::DISABLE_ALL_META_INFO) {
//     if constexpr (struct_pack::detail::user_defined_config<T>) {
//       constexpr auto config = set_sp_config((T *)nullptr) & 0b11;
//       if constexpr (config == sp_config::DISABLE_ALL_META_INFO) {
//         return true;
//       }
//     }
//     return false;
//   }
//   return true;
// }

template <uint64_t conf, typename T,
          std::enable_if_t<check_if_disable_hash_head_impl<conf, T>(), int> = 0>
constexpr bool check_if_disable_hash_head() {
  static_assert(
      !check_if_compatible_element_exist<decltype(get_types<T>())>(),
      "It's not allow add compatible member when you disable hash head");
  return true;
}
template <
    uint64_t conf, typename T,
    std::enable_if_t<!check_if_disable_hash_head_impl<conf, T>(), int> = 0>
constexpr bool check_if_disable_hash_head() {
  return false;
}

template <uint64_t conf, typename T>
constexpr bool check_has_metainfo() {
  return serialize_static_config<T>::has_compatible ||
         (!check_if_disable_hash_head<conf, T>() &&
          check_if_add_type_literal<conf, T>()) ||
         ((check_if_disable_hash_head<conf, T>() &&
           check_if_has_container<T>()));
}
template <
    typename U, typename T = remove_cvref_t<U>,
    std::enable_if_t<std::is_fundamental<T>::value || std::is_enum<T>::value ||
                         varint_t<T> || string<T> || container<T> ||
                         optional<T> || unique_ptr<T> || is_variant_v<T> ||
                         expected<T> || array<T> || c_array<T> || bitset<T>
#if (__GNUC__ || __clang__)
                         || std::is_same<__int128, T>::value ||
                         std::is_same<unsigned __int128, T>::value
#endif
                     ,
                     int> = 0>
constexpr auto get_types() {
  return declval<std::tuple<T>>();
}

template <typename U, typename T = remove_cvref_t<U>,
          std::enable_if_t<tuple<T> || is_trivial_tuple<T>, int> = 0>
constexpr auto get_types() {
  return declval<T>();
}

template <typename U, typename T = remove_cvref_t<U>,
          std::enable_if_t<pair<T>, int> = 0>
constexpr auto get_types() {
  return declval<std::tuple<typename T::first_type, typename T::second_type>>();
}

template <typename U, typename T = remove_cvref_t<U>,
          std::enable_if_t<std::is_class<T>::value, int> = 0>
constexpr auto get_types() {
  return visit_members(declval<T>(), [](auto &&...args) {
    return declval<std::tuple<remove_cvref_t<decltype(args)>...>>();
  });
}

}  // namespace details
}  // namespace serialize

#endif