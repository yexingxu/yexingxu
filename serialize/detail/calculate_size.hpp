/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 00:08:33
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-12 12:49:01
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "append_types/apply.hpp"
#include "macro.hpp"
#include "size_info.hpp"
#include "type_id.hpp"

namespace serialize {

namespace detail {

template <typename conf, typename... Args>
constexpr size_info inline calculate_payload_size(const Args &...items);

template <typename conf>
struct calculate_one_size {
  template <typename T, typename type = remove_cvref_t<T>,
            type_id id = get_type_id<remove_cvref_t<T>>(),
            std::enable_if_t<
                std::is_fundamental<type>::value || std::is_enum<type>::value ||
                    id == type_id::int128_t || id == type_id::uint128_t ||
                    id == type_id::bitset_t,
                int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info info{};
    info.total = sizeof(remove_cvref_t<decltype(item)>);
    return info;
  }
  template <typename T, typename type = remove_cvref_t<T>,
            type_id id = get_type_id<remove_cvref_t<T>>(),
            std::enable_if_t<is_trivial_view_v<type>, int> = 0>
  constexpr size_info inline operator()(const T &item) {
    return calculate_one_size()(item.get());
  }

  // string
  template <typename T, typename type = remove_cvref_t<T>,
            type_id id = get_type_id<remove_cvref_t<T>>(),
            std::enable_if_t<id == type_id::string_t, int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info ret{};
    ret.length_field += conf::kStringLengthField;
    ret.total = item.size() + 1;
    return ret;
  }

  // container
  template <typename T, typename type = remove_cvref_t<T>,
            type_id id = get_type_id<remove_cvref_t<T>>(),
            std::enable_if_t<(id == type_id::container_t ||
                              id == type_id::set_container_t ||
                              id == type_id::map_container_t) &&
                                 trivially_copyable_container<type>,
                             int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info ret{};
    ret.length_field += conf::kArrayLengthField;
    using value_type = typename type::value_type;
    ret.total = item.size() * sizeof(value_type);
    return ret;
  }

  template <typename T, typename type = remove_cvref_t<T>,
            type_id id = get_type_id<remove_cvref_t<T>>(),
            std::enable_if_t<(id == type_id::container_t ||
                              id == type_id::set_container_t ||
                              id == type_id::map_container_t) &&
                                 !trivially_copyable_container<type>,
                             int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info ret{};
    ret.length_field += conf::kArrayLengthField;
    for (auto &&i : item) {
      ret += calculate_one_size()(i);
    }
    return ret;
  }
  // array
  template <typename T, typename type = remove_cvref_t<T>,
            type_id id = get_type_id<type>(),
            std::enable_if_t<(id == type_id::array_t &&
                              is_trivial_serializable<type>::value),
                             int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info info{};
    info.total = sizeof(remove_cvref_t<decltype(item)>);
    return info;
  }

  template <typename T, typename type = remove_cvref_t<T>,
            type_id id = get_type_id<type>(),
            std::enable_if_t<id == type_id::array_t &&
                                 !is_trivial_serializable<type>::value,
                             int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info info{};
    for (auto &i : item) {
      info += calculate_one_size()(i);
    }
    return info;
  }

  // pair
  template <typename T, typename type = remove_cvref_t<T>,
            type_id id = get_type_id<remove_cvref_t<T>>(),
            std::enable_if_t<id == type_id::pair_t, int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info ret{};
    ret += calculate_one_size()(item.first);
    ret += calculate_one_size()(item.second);
    return ret;
  }

  // tuple
  template <typename T, typename type = remove_cvref_t<T>,
            type_id id = get_type_id<remove_cvref_t<T>>(),
            std::enable_if_t<id == type_id::tuple_t, int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info ret{};
    tl::apply(
        [&](auto &&...items) { ret += calculate_payload_size<conf>(items...); },
        item);
    return ret;
  }

  // variant
  template <typename T, typename type = remove_cvref_t<T>,
            type_id id = get_type_id<remove_cvref_t<T>>(),
            std::enable_if_t<id == type_id::variant_t, int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info ret{};
    ret.length_field += conf::kUnionLengthField;
    ret += mpark::visit([](const auto &e) { return calculate_one_size()(e); },
                        item);
    return ret;
  }

  // struct
  template <typename T, typename type = remove_cvref_t<T>,
            type_id id = get_type_id<remove_cvref_t<T>>(),
            std::enable_if_t<id == type_id::struct_t, int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info ret{};
    ret.length_field += conf::kStructLengthField;
    visit_members(item, [&](auto &&...items) {
      ret += calculate_payload_size<conf>(items...);
    });
    return ret;
  }

  // struct
  template <typename T, typename type = remove_cvref_t<T>,
            type_id id = get_type_id<remove_cvref_t<T>>(),
            std::enable_if_t<id == type_id::pointer_t, int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info ret{};
    ret += calculate_one_size<conf>()(*item);
    return ret;
  }
};

template <typename conf, typename... Args>
constexpr size_info inline calculate_payload_size(const Args &...items) {
  size_info info{0, 0};
  static_cast<void>(std::initializer_list<int>{
      (info += calculate_one_size<conf>()(items), 0)...});
  return info;
}

template <typename conf, typename... Args>
constexpr std::size_t get_serialize_runtime_info(const Args &...args);
}  // namespace detail

namespace detail {

template <typename conf, typename... Args>
constexpr std::size_t get_serialize_runtime_info(const Args &...args) {
  std::size_t ret = 0;
  auto sz_info = calculate_payload_size<conf>(args...);
  ret = sz_info.total + sz_info.length_field;
  return ret;
}

}  // namespace detail
}  // namespace serialize