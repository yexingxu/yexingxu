/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-29 01:55:59
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-29 02:24:53
 */

#include <type_traits>
#include <vector>

#include "serializer.hpp"
#include "type_traits.hpp"

template <uint64_t conf = serialize::sp_config::DEFAULT, typename Writer,
          typename... Args,
          std::enable_if_t<serialize::details::writer_t<Writer>, int> = 0>
void serialize_to(Writer &writer, const Args &...args) {
  static_assert(sizeof...(args) > 0, "");
  auto info = serialize::details::get_serialize_runtime_info<conf>(args...);
  serialize::details::serialize_to<conf>(writer, info, args...);
}

template <
    uint64_t conf = serialize::sp_config::DEFAULT, typename Writer,
    typename... Args,
    std::enable_if_t<serialize::details::serialize_buffer<Writer>, int> = 0>
void serialize_to(Writer &writer, const Args &...args) {
  static_assert(sizeof...(args) > 0, "");
  static_assert(sizeof...(args) > 0);
  auto data_offset = writer.size();
  auto info = serialize::details::get_serialize_runtime_info<conf>(args...);
  auto total = data_offset + info.size();
  serialize::details::resize(writer, total);
  auto real_writer =
      serialize::details::memory_writer{(char *)writer.data() + data_offset};
  serialize::details::serialize_to<conf>(real_writer, info, args...);
}

template <uint64_t conf = serialize::sp_config::DEFAULT, typename Writer,
          typename... Args,
          std::enable_if_t<!(serialize::details::serialize_buffer<Writer> ||
                             serialize::details::writer_t<Writer>),
                           int> = 0>
void serialize_to(Writer &, const Args &...) {
  static_assert(!sizeof(Writer),
                "The Writer is not satisfied serialize::writer_t or "
                "buffer requirement!");
}

template <uint64_t conf = serialize::sp_config::DEFAULT, typename... Args>
void serialize_to(char *buffer, serialize::details::serialize_buffer_size info,
                  const Args &...args) noexcept {
  static_assert(sizeof...(args) > 0);
  auto writer = serialize::details::memory_writer{(char *)buffer};
  serialize::details::serialize_to<conf>(writer, info, args...);
}

template <typename Buffer = std::vector<char>, typename... Args>
Buffer Serialize(const Args &...args) {
  static_assert(serialize::details::serialize_buffer<Buffer>,
                "The buffer is not satisfied serialize_buffer requirement!");
  static_assert(sizeof...(args) > 0, "No args to serialize");
  Buffer buffer;
  serialize_to(buffer, args...);
  return buffer;
}

template <uint64_t conf, typename Buffer = std::vector<char>, typename... Args>
Buffer Serialize(const Args &...args) {
  static_assert(serialize::details::serialize_buffer<Buffer>,
                "The buffer is not satisfied serialize_buffer requirement!");
  static_assert(sizeof...(args) > 0, "No args to serialize");
  Buffer buffer;
  serialize_to<conf>(buffer, args...);
  return buffer;
}