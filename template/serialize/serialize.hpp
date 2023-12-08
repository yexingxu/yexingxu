/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-07 14:53:03
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-08 12:13:49
 */

#pragma once

#include <cstdint>
#include <type_traits>

#include "calculate_size.hpp"
#include "memory_writer.hpp"
#include "reflection.hpp"
#include "serializer.hpp"
#include "type_calculate.hpp"
#include "type_traits.hpp"
#include "utils.hpp"

namespace serialize {

template <
    typename... Args,
    std::enable_if_t<sizeof...(Args) == 1 && std::is_abstract<Args...>::value,
                     int> = 0>
constexpr std::uint32_t get_type_code() {
  static_assert(sizeof...(Args) > 0, "");
  std::uint32_t ret = 0;
  details::unreachable();
  ret = ret - ret % 2;
  return ret;
}

template <
    typename... Args,
    std::enable_if_t<sizeof...(Args) == 1 && !std::is_abstract<Args...>::value,
                     int> = 0>
constexpr std::uint32_t get_type_code() {
  static_assert(sizeof...(Args) > 0, "");
  std::uint32_t ret = 0;
  ret = details::get_types_code<Args...>();
  ret = ret - ret % 2;
  return ret;
}

template <typename... Args, std::enable_if_t<sizeof...(Args) != 1, int> = 0>
constexpr std::uint32_t get_type_code() {
  static_assert(sizeof...(Args) > 0, "");
  std::uint32_t ret = 0;
  ret = details::get_types_code<std::tuple<details::remove_cvref_t<Args>...>>();
  ret = ret - ret % 2;
  return ret;
}

template <typename... Args, std::enable_if_t<sizeof...(Args) == 1, int> = 0>
constexpr decltype(auto) get_type_literal() {
  static_assert(sizeof...(Args) > 0, "");
  using Types = decltype(details::get_types<Args...>());
  return details::get_types_literal<Args..., Types>(
      std::make_index_sequence<std::tuple_size<Types>::value>());
}

template <typename... Args, std::enable_if_t<sizeof...(Args) != 1, int> = 0>
constexpr decltype(auto) get_type_literal() {
  static_assert(sizeof...(Args) > 0, "");
  return details::get_types_literal<
      std::tuple<details::remove_cvref_t<Args>...>,
      details::remove_cvref_t<Args>...>();
}

template <uint64_t conf = sp_config::DEFAULT, typename Writer, typename... Args,
          std::enable_if_t<details::writer_t<Writer>, int> = 0>
void serialize_to(Writer &writer, const Args &...args) {
  static_assert(sizeof...(args) > 0, "");
  auto info = details::get_serialize_runtime_info<conf>(args...);
  details::serialize_to<conf>(writer, info, args...);
}

template <uint64_t conf = sp_config::DEFAULT, typename Writer, typename... Args,
          std::enable_if_t<details::serialize_buffer<Writer>, int> = 0>
void serialize_to(Writer &writer, const Args &...args) {
  static_assert(sizeof...(args) > 0, "");
  auto data_offset = writer.size();
  auto info = details::get_serialize_runtime_info<conf>(args...);
  auto total = data_offset + info.size();
  writer.resize(total);
  //   details::resize(writer, total);
  auto real_writer =
      details::memory_writer{(char *)writer.data() + data_offset};
  details::serialize_to<conf>(real_writer, info, args...);
}

template <uint64_t conf = sp_config::DEFAULT, typename Writer, typename... Args,
          std::enable_if_t<!details::serialize_buffer<Writer> &&
                               !details::writer_t<Writer>,
                           int> = 0>
void serialize_to(Writer &, const Args &...) {
  static_assert(!sizeof(Writer),
                "The Writer is not satisfied serialize::writer_t or "
                "serialize_buffer requirement!");
}

template <typename Buffer = std::vector<char>, typename... Args>
Buffer serialize(const Args &...args) {
  static_assert(details::serialize_buffer<Buffer>,
                "The buffer is not satisfied serialize_buffer requirement!");
  static_assert(sizeof...(args) > 0, "");
  Buffer buffer;
  serialize_to(buffer, args...);
  return buffer;
}

template <uint64_t conf, typename Buffer = std::vector<char>, typename... Args>
Buffer serialize(const Args &...args) {
  static_assert(details::serialize_buffer<Buffer>,
                "The buffer is not satisfied serialize_buffer requirement!");
  static_assert(sizeof...(args) > 0, "");
  Buffer buffer;
  serialize_to<conf>(buffer, args...);
  return buffer;
}

}  // namespace serialize