/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-07 14:53:03
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-13 02:41:06
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "calculate_size.hpp"
#include "deserializer.hpp"
#include "expected.hpp"
#include "memory_writer.hpp"
#include "reflection.hpp"
#include "serializer.hpp"
#include "type_calculate.hpp"
#include "type_traits.hpp"
#include "utils.hpp"

namespace serialize {

template <class T, class E>
using expected = tl::expected<T, E>;

template <class T>
using unexpected = tl::unexpected<T>;

using unexpect_t = tl::unexpect_t;

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

//___________________________________________//
template <typename Reader,
          std::enable_if_t<details::seek_reader_t<Reader>, int> = 0>
void seek_reader_helper(Reader &reader, std::size_t old_pos,
                        serialize::errc &code) {
  if SER_UNLIKELY (!reader.seekg(old_pos)) {
    code = serialize::errc::seek_failed;
  }
}
template <typename Reader,
          std::enable_if_t<!details::seek_reader_t<Reader>, int> = 0>
void seek_reader_helper(Reader &, std::size_t, serialize::errc &) {}

template <
    uint64_t conf = sp_config::DEFAULT, typename T, typename... Args,
    typename View,
    typename = std::enable_if_t<serialize::details::deserialize_view<View>>>
serialize::errc deserialize_to(T &t, const View &v, Args &...args) {
  details::memory_reader reader{(const char *)v.data(),
                                (const char *)v.data() + v.size()};
  details::Deserializer<details::memory_reader, conf> in(reader);
  return in.deserialize(t, args...);
}

template <uint64_t conf = sp_config::DEFAULT, typename T, typename... Args>
serialize::errc deserialize_to(T &t, const char *data, size_t size,
                               Args &...args) {
  details::memory_reader reader{data, data + size};
  details::Deserializer<details::memory_reader, conf> in(reader);
  return in.deserialize(t, args...);
}

template <uint64_t conf = sp_config::DEFAULT, typename T, typename... Args,
          typename Reader,
          typename = std::enable_if_t<details::reader_t<Reader>>>
serialize::errc deserialize_to(T &t, Reader &reader, Args &...args) {
  details::Deserializer<Reader, conf> in(reader);
  std::size_t consume_len;
  auto old_pos = reader.tellg();
  auto ret = in.deserialize_with_len(consume_len, t, args...);
  std::size_t delta = reader.tellg() - old_pos;
  if SER_LIKELY (ret == errc{}) {
    if SER_LIKELY (consume_len > 0) {
      if SER_UNLIKELY (delta > consume_len) {
        ret = serialize::errc::invalid_buffer;
        seek_reader_helper<Reader>(reader, old_pos, ret);
      } else {
        reader.ignore(consume_len - delta);
      }
    }
  } else {
    seek_reader_helper<Reader>(reader, old_pos, ret);
  }
  return ret;
}

template <typename... Args, typename View,
          typename = std::enable_if_t<details::deserialize_view<View>>>
auto deserialize(const View &v) {
  expected<details::get_args_type<Args...>, serialize::errc> ret;

  auto errc = deserialize_to(ret.value(), v);

  if SER_UNLIKELY (errc != serialize::errc{}) {
    ret = unexpected<serialize::errc>{errc};
  }
  return ret;
}

template <typename... Args>
auto deserialize(const char *data, size_t size) {
  expected<details::get_args_type<Args...>, serialize::errc> ret;
  serialize::errc errc{};

  errc = deserialize_to(ret.value(), data, size);

  if (errc != serialize::errc{}) {
    ret = unexpected<serialize::errc>{errc};
  }

  return ret;
}

template <typename... Args, typename Reader,
          typename = std::enable_if_t<details::reader_t<Reader>>>
auto deserialize(Reader &v) {
  expected<details::get_args_type<Args...>, serialize::errc> ret;
  auto errc = deserialize_to(ret.value(), v);
  if SER_UNLIKELY (errc != serialize::errc{}) {
    ret = unexpected<serialize::errc>{errc};
  }
  return ret;
}

template <uint64_t conf, typename... Args, typename View,
          typename = std::enable_if_t<details::deserialize_view<View>>>
auto deserialize(const View &v) {
  expected<details::get_args_type<Args...>, serialize::errc> ret;

  auto errc = deserialize_to<conf>(ret.value(), v);

  if SER_UNLIKELY (errc != serialize::errc{}) {
    ret = unexpected<serialize::errc>{errc};
  }
  return ret;
}

template <uint64_t conf, typename... Args>
auto deserialize(const char *data, size_t size) {
  expected<details::get_args_type<Args...>, serialize::errc> ret;
  serialize::errc errc{};

  errc = deserialize_to<conf>(ret.value(), data, size);

  if (errc != serialize::errc{}) {
    ret = unexpected<serialize::errc>{errc};
  }

  return ret;
}

template <uint64_t conf, typename... Args, typename Reader,
          typename = std::enable_if_t<details::reader_t<Reader>>>
auto deserialize(Reader &v) {
  expected<details::get_args_type<Args...>, serialize::errc> ret;
  auto errc = deserialize_to<conf>(ret.value(), v);
  if SER_UNLIKELY (errc != serialize::errc{}) {
    ret = unexpected<serialize::errc>{errc};
  }
  return ret;
}

}  // namespace serialize