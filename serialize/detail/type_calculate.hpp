/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 06:52:31
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-28 21:19:48
 */

#pragma once

#include <type_traits>
#include <utility>

#include "reflection.hpp"
#include "type_id.hpp"

namespace serialize {
namespace detail {

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

template <typename Arg, typename... ParentArgs, std::size_t... I>
constexpr bool check_if_has_container_helper(std::index_sequence<I...>);

template <typename Arg, typename... ParentArgs,
          std::enable_if_t<check_circle<Arg, ParentArgs...>() == 0 &&
                               (std::is_fundamental<Arg>::value ||
                                std::is_floating_point<Arg>::value ||
                                std::is_enum<Arg>::value),
                           int> = 0>
constexpr bool check_if_has_container() {
  return false;
}

template <typename Arg, typename... ParentArgs,
          std::enable_if_t<is_trivial_view_v<Arg>, int> = 0>
constexpr bool check_if_has_container() {
  return check_if_has_container<typename Arg::value_type, ParentArgs...>();
}

template <typename Arg, typename... ParentArgs,
          std::enable_if_t<check_circle<Arg, ParentArgs...>() != 0, int> = 0>
constexpr bool check_if_has_container() {
  return false;
}

template <typename Arg, typename... ParentArgs,
          std::enable_if_t<check_circle<Arg, ParentArgs...>() == 0 &&
                               get_type_id<Arg>() == type_id::struct_t,
                           int> = 0>
constexpr bool check_if_has_container() {
  using Args = decltype(get_types<Arg>());
  return check_if_has_container_helper<Args, Arg, ParentArgs...>(
      std::make_index_sequence<std::tuple_size<Args>::value>());
}

template <typename Arg, typename... ParentArgs,
          std::enable_if_t<check_circle<Arg, ParentArgs...>() == 0 &&
                               get_type_id<Arg>() == type_id::array_t,
                           int> = 0>
constexpr bool check_if_has_container() {
  return check_if_has_container<
      remove_cvref_t<decltype(std::declval<Arg>()[0])>, ParentArgs...>();
}

template <typename Arg, typename... ParentArgs,
          std::enable_if_t<check_circle<Arg, ParentArgs...>() == 0 &&
                               get_type_id<Arg>() == type_id::bitset_t,
                           int> = 0>
constexpr bool check_if_has_container() {
  return false;
}

template <
    typename Arg, typename... ParentArgs,
    std::enable_if_t<check_circle<Arg, ParentArgs...>() == 0 &&
                         (get_type_id<Arg>() == type_id::container_t ||
                          get_type_id<Arg>() == type_id::string_t ||
                          get_type_id<Arg>() == type_id::set_container_t ||
                          get_type_id<Arg>() == type_id::map_container_t),
                     int> = 0>
constexpr bool check_if_has_container() {
  return true;
}

template <typename Arg, typename... ParentArgs,
          std::enable_if_t<check_circle<Arg, ParentArgs...>() == 0 &&
                               (get_type_id<Arg>() == type_id::optional_t),
                           int> = 0>
constexpr bool check_if_has_container() {
  return check_if_has_container<remove_cvref_t<typename Arg::value_type>,
                                ParentArgs...>();
}

template <typename Arg, typename... ParentArgs,
          std::enable_if_t<check_circle<Arg, ParentArgs...>() == 0 &&
                               get_type_id<Arg>() == type_id::expected_t,
                           int> = 0>
constexpr bool check_if_has_container() {
  return check_if_has_container<remove_cvref_t<typename Arg::value_type>,
                                ParentArgs...>() ||
         check_if_has_container<remove_cvref_t<typename Arg::error_type>,
                                ParentArgs...>();
}

template <typename Arg, typename... ParentArgs,
          std::enable_if_t<check_circle<Arg, ParentArgs...>() == 0 &&
                               get_type_id<Arg>() == type_id::pair_t,
                           int> = 0>
constexpr bool check_if_has_container() {
  return check_if_has_container<remove_cvref_t<typename Arg::first_type>,
                                ParentArgs...>() ||
         check_if_has_container<remove_cvref_t<typename Arg::second_type>,
                                ParentArgs...>();
}

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

}  // namespace detail
}  // namespace serialize