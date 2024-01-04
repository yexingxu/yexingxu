/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 00:08:33
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-04 20:44:12
 */
#pragma once

#include <cstdint>
#include <iostream>

#include "macro.hpp"
#include "size_info.hpp"
#include "type_calculate.hpp"
#include "type_id.hpp"

namespace serialize {

struct serialize_buffer_size;
namespace detail {

template <typename... Args>
constexpr size_info inline calculate_payload_size(const Args &...items);

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

  // container
  template <
      typename T, typename type = remove_cvref_t<T>,
      type_id id = get_type_id<remove_cvref_t<T>>(),
      std::enable_if_t<(id == type_id::container_t || id == type_id::string_t ||
                        id == type_id::set_container_t ||
                        id == type_id::map_container_t) &&
                           trivially_copyable_container<type>,
                       int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info ret{};
    ret.size_cnt += 1;
    ret.max_size = item.size();
    using value_type = typename type::value_type;
    ret.total = item.size() * sizeof(value_type);
    return ret;
  }

  template <
      typename T, typename type = remove_cvref_t<T>,
      type_id id = get_type_id<remove_cvref_t<T>>(),
      std::enable_if_t<(id == type_id::container_t || id == type_id::string_t ||
                        id == type_id::set_container_t ||
                        id == type_id::map_container_t) &&
                           !trivially_copyable_container<type>,
                       int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info ret{};
    ret.size_cnt += 1;
    ret.max_size = item.size();
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

  // struct
  template <typename T, typename type = remove_cvref_t<T>,
            type_id id = get_type_id<remove_cvref_t<T>>(),
            std::enable_if_t<id == type_id::struct_t, int> = 0>
  constexpr size_info inline operator()(const T &item) {
    size_info ret{};
    visit_members(item, [&](auto &&...items) {
      ret += calculate_payload_size(items...);
    });
    return ret;
  }
};

template <typename... Args>
constexpr size_info inline calculate_payload_size(const Args &...items) {
  size_info info{0, 0, 0};
  static_cast<void>(
      std::initializer_list<int>{(info += calculate_one_size()(items), 0)...});
  return info;
}

template <uint64_t conf, typename... Args>
constexpr serialize_buffer_size get_serialize_runtime_info(const Args &...args);
}  // namespace detail

struct serialize_buffer_size {
  std::size_t len_;
  unsigned char metainfo_;

  constexpr serialize_buffer_size() : len_(0), metainfo_(0) {}
  constexpr std::size_t size() const { return len_; }
  constexpr unsigned char metainfo() const { return metainfo_; }
  constexpr operator std::size_t() const { return len_; }

  template <uint64_t conf, typename... Args>
  friend constexpr serialize_buffer_size detail::get_serialize_runtime_info(
      const Args &...args);
};

namespace detail {

template <bool has_container, std::enable_if_t<!has_container, int> = 0>
constexpr void has_container_helper(serialize_buffer_size &ret,
                                    size_info &sz_info) {
  ret.len_ += sz_info.total;
}

template <bool has_container, std::enable_if_t<has_container, int> = 0>
constexpr void has_container_helper(serialize_buffer_size &ret,
                                    size_info &sz_info) {
  if SER_LIKELY (sz_info.max_size < (uint64_t{1} << 8)) {
    ret.len_ += sz_info.total + sz_info.size_cnt;
  } else {
    if (sz_info.max_size < (uint64_t{1} << 16)) {
      ret.len_ += sz_info.total + sz_info.size_cnt * 2;
      ret.metainfo_ = 0b01000;
    } else if (sz_info.max_size < (uint64_t{1} << 32)) {
      ret.len_ += sz_info.total + sz_info.size_cnt * 4;
      ret.metainfo_ = 0b10000;
    } else {
      ret.len_ += sz_info.total + sz_info.size_cnt * 8;
      ret.metainfo_ = 0b11000;
    }
  }
}

template <std::uint64_t conf, typename... Args>
constexpr serialize_buffer_size get_serialize_runtime_info(
    const Args &...args) {
  using Type = get_args_type<Args...>;
  serialize_buffer_size ret;
  auto sz_info = calculate_payload_size(args...);
  constexpr bool has_container = check_if_has_container<Type>();
  has_container_helper<has_container>(ret, sz_info);
  return ret;
}

}  // namespace detail
}  // namespace serialize