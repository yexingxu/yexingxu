/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-07 14:53:03
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-28 23:52:33
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <type_traits>

#include "append_types/expected.hpp"
#include "detail/calculate_size.hpp"
#include "detail/deserializer.hpp"
#include "detail/memory_reader.hpp"
#include "detail/memory_writer.hpp"
#include "detail/reflection.hpp"
#include "detail/return_code.hpp"
#include "detail/serializer.hpp"
#include "detail/type_calculate.hpp"
#include "detail/utils.hpp"
#include "ser_config.hpp"

namespace serialize {

template <class T, class E>
using expected = tl::expected<T, E>;

template <class T>
using unexpected = tl::unexpected<T>;

using unexpect_t = tl::unexpect_t;

template <uint64_t conf = ser_config::DEFAULT, typename Writer,
          typename... Args, std::enable_if_t<detail::writer_t<Writer>, int> = 0>
void serialize_to(Writer &writer, const Args &...args) {
  static_assert(sizeof...(args) > 0, "");
  auto info = detail::get_serialize_runtime_info<conf>(args...);
  detail::serialize_to<conf>(writer, info, args...);
}

template <uint64_t conf = ser_config::DEFAULT, typename Writer,
          typename... Args,
          std::enable_if_t<detail::serialize_buffer<Writer>, int> = 0>
void serialize_to(Writer &writer, const Args &...args) {
  static_assert(sizeof...(args) > 0, "");

  auto data_offset = writer.size();
  auto info = detail::get_serialize_runtime_info<conf>(args...);
  auto total = data_offset + info.size();
  writer.resize(total);
  //   detail::resize(writer, total);
  auto real_writer = detail::memory_writer{(char *)writer.data() + data_offset};
  detail::serialize_to<conf>(real_writer, info, args...);
}

template <uint64_t conf = ser_config::DEFAULT, typename Writer,
          typename... Args,
          std::enable_if_t<!detail::serialize_buffer<Writer> &&
                               !detail::writer_t<Writer>,
                           int> = 0>
void serialize_to(Writer &, const Args &...) {
  static_assert(!sizeof(Writer),
                "The Writer is not satisfied serialize::writer_t or "
                "serialize_buffer requirement!");
}

template <typename Buffer = std::vector<char>, typename... Args>
Buffer serialize(const Args &...args) {
  static_assert(detail::serialize_buffer<Buffer>,
                "The buffer is not satisfied serialize_buffer requirement!");
  static_assert(sizeof...(args) > 0, "");
  Buffer buffer;
  serialize_to(buffer, args...);
  return buffer;
}

template <uint64_t conf, typename Buffer = std::vector<char>, typename... Args>
Buffer serialize(const Args &...args) {
  static_assert(detail::serialize_buffer<Buffer>,
                "The buffer is not satisfied serialize_buffer requirement!");
  static_assert(sizeof...(args) > 0, "");
  Buffer buffer;
  serialize_to<conf>(buffer, args...);
  return buffer;
}

//___________________________________________//
template <typename Reader,
          std::enable_if_t<detail::seek_reader_t<Reader>, int> = 0>
void seek_reader_helper(Reader &reader, std::size_t old_pos,
                        serialize::return_code &code) {
  if SER_UNLIKELY (!reader.seekg(old_pos)) {
    code = serialize::return_code::seek_failed;
  }
}
template <typename Reader,
          std::enable_if_t<!detail::seek_reader_t<Reader>, int> = 0>
void seek_reader_helper(Reader &, std::size_t, serialize::return_code &) {}

template <uint64_t conf = ser_config::DEFAULT, typename T, typename... Args,
          typename View,
          typename = std::enable_if_t<detail::deserialize_view<View>>>
serialize::return_code deserialize_to(T &t, const View &v, Args &...args) {
  detail::memory_reader reader{(const char *)v.data(),
                               (const char *)v.data() + v.size()};
  detail::Deserializer<detail::memory_reader, conf> in(reader);
  return in.deserialize(t, args...);
}

template <uint64_t conf = ser_config::DEFAULT, typename T, typename... Args>
serialize::return_code deserialize_to(T &t, const char *data, size_t size,
                                      Args &...args) {
  detail::memory_reader reader{data, data + size};
  detail::Deserializer<detail::memory_reader, conf> in(reader);
  return in.deserialize(t, args...);
}

template <uint64_t conf = ser_config::DEFAULT, typename T, typename... Args,
          typename Reader,
          typename = std::enable_if_t<detail::reader_t<Reader>>>
serialize::return_code deserialize_to(T &t, Reader &reader, Args &...args) {
  detail::Deserializer<Reader, conf> in(reader);
  std::size_t consume_len;
  auto old_pos = reader.tellg();
  auto ret = in.deserialize_with_len(consume_len, t, args...);
  std::size_t delta = reader.tellg() - old_pos;
  if SER_LIKELY (ret == return_code{}) {
    if SER_LIKELY (consume_len > 0) {
      if SER_UNLIKELY (delta > consume_len) {
        ret = serialize::return_code::invalid_buffer;
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
          typename = std::enable_if_t<detail::deserialize_view<View>>>
auto deserialize(const View &v) {
  expected<detail::get_args_type<Args...>, serialize::return_code> ret;
  auto errc = deserialize_to(ret.value(), v);
  if SER_UNLIKELY (errc != serialize::return_code::ok) {
    ret = unexpected<serialize::return_code>{errc};
  }
  return ret;
}

template <typename... Args>
auto deserialize(const char *data, size_t size) {
  expected<detail::get_args_type<Args...>, serialize::return_code> ret;
  serialize::return_code errc{};

  errc = deserialize_to(ret.value(), data, size);

  if (errc != serialize::return_code{}) {
    ret = unexpected<serialize::return_code>{errc};
  }

  return ret;
}

template <typename... Args, typename Reader,
          typename = std::enable_if_t<detail::reader_t<Reader>>>
auto deserialize(Reader &v) {
  expected<detail::get_args_type<Args...>, serialize::return_code> ret;
  auto errc = deserialize_to(ret.value(), v);
  if SER_UNLIKELY (errc != serialize::return_code{}) {
    ret = unexpected<serialize::return_code>{errc};
  }
  return ret;
}

template <uint64_t conf, typename... Args, typename View,
          typename = std::enable_if_t<detail::deserialize_view<View>>>
auto deserialize(const View &v) {
  expected<detail::get_args_type<Args...>, serialize::return_code> ret;

  auto errc = deserialize_to<conf>(ret.value(), v);
  if SER_UNLIKELY (errc != serialize::return_code{}) {
    ret = unexpected<serialize::return_code>{errc};
  }
  return ret;
}

template <uint64_t conf, typename... Args>
auto deserialize(const char *data, size_t size) {
  expected<detail::get_args_type<Args...>, serialize::return_code> ret;
  serialize::return_code errc{};

  errc = deserialize_to<conf>(ret.value(), data, size);
  if (errc != serialize::return_code{}) {
    ret = unexpected<serialize::return_code>{errc};
  }

  return ret;
}

template <uint64_t conf, typename... Args, typename Reader,
          typename = std::enable_if_t<detail::reader_t<Reader>>>
auto deserialize(Reader &v) {
  expected<detail::get_args_type<Args...>, serialize::return_code> ret;
  auto errc = deserialize_to<conf>(ret.value(), v);
  if SER_UNLIKELY (errc != serialize::return_code{}) {
    ret = unexpected<serialize::return_code>{errc};
  }
  return ret;
}

}  // namespace serialize