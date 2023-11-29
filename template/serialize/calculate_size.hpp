/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-29 02:28:26
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-29 11:27:47
 */

#ifndef SERIALIZE_CALCULATE_SIZE_HPP_
#define SERIALIZE_CALCULATE_SIZE_HPP_

#include <cstdint>
#include <type_traits>

#include "if_constexpr.hpp"
#include "reflection.hpp"
#include "size_info.hpp"
#include "type_calculate.hpp"

namespace serialize {

struct serialize_buffer_size;

namespace details {

template <typename... Args>
constexpr size_info inline calculate_payload_size(const Args &...items);

template <typename T>
constexpr size_info inline calculate_one_size(const T &item) {
  constexpr auto id = get_type_id<remove_cvref_t<T>>();
  static_assert(id != details::type_id::type_end_flag, "");
  using type = remove_cvref_t<decltype(item)>;
  static_assert(!std::is_pointer<type>::value, "Not support pointer");
  size_info ret{};

  utils::if_<id == type_id::monostate_t>(
      [] {  // do nothing
      },
      utils::else_if_ < std::is_fundamental<type>::value ||
          std::is_enum<type>::value || id == type_id::int128_t ||
          id == type_id::uint128_t ||
          id == type_id::bitset_t >
                    ([&ret] { ret.total = sizeof(type); },
                     utils::else_if_<is_trivial_view_v<type>>(
                         [&item] { return calculate_one_size(item.get()); },
                         utils::else_if_<varint_t<type>>(
                             [&ret, &item] {
                               ret.total = calculate_varint_size(item);
                             },
                             utils::else_if_<id == type_id::array_t>(
                                 [&ret, &item] {
                                   utils::if_<
                                       is_trivial_serializable<type>::value>(
                                       [&ret] { ret.total = sizeof(type); },
                                       utils::else_([&item, &ret] {
                                         for (auto &i : item) {
                                           ret += calculate_one_size(i);
                                         }
                                       }));
                                 },
                                 utils::else_if_<container<type>>(
                                     [&ret, &item] {
                                       ret.size_cnt += 1;
                                       ret.max_size = item.size();
                                       utils::if_<
                                           trivially_copyable_container<type>>(
                                           [&ret, &item] {
                                             using value_type =
                                                 typename type::value_type;
                                             ret.total = item.size() *
                                                         sizeof(value_type);
                                           },
                                           utils::else_([&ret, &item] {
                                             for (auto &&i : item) {
                                               ret += calculate_one_size(i);
                                             }
                                           }));
                                     },
                                     utils::else_if_<optional<type>>(
                                         [&ret, &item] {
                                           ret.total = sizeof(char);
                                           if (item) {
                                             ret += calculate_one_size(*item);
                                           }
                                         },
                                         utils::else_([] {
                                           static_assert(
                                               !sizeof(type),
                                               "the type is not supported yet");
                                         }))))))));

  return ret;
}

template <typename... Args>
constexpr size_info inline calculate_payload_size(const Args &...items) {
  size_info info;
  static_cast<void>(
      std::initializer_list<int>{(info += calculate_one_size(items), 0)...});
  return info;
}

template <uint64_t conf, typename... Args>
constexpr serialize_buffer_size get_serialize_runtime_info(const Args &...args);

}  // namespace details

struct serialize_buffer_size {
 private:
  std::size_t len_;
  unsigned char metainfo_;

 public:
  constexpr serialize_buffer_size() : len_(0), metainfo_(0) {}
  constexpr std::size_t size() const { return len_; }
  constexpr unsigned char metainfo() const { return metainfo_; }
  constexpr operator std::size_t() const { return len_; }

  template <uint64_t conf, typename... Args>
  friend constexpr serialize_buffer_size details::get_serialize_runtime_info(
      const Args &...args);
};

namespace details {

template <std::uint64_t conf, typename... Args>
constexpr serialize_buffer_size get_serialize_runtime_info(
    const Args &...args) {
  using Type = get_args_type<Args...>;
  constexpr bool has_compatible = serialize_static_config<Type>::has_compatible;
  constexpr bool has_type_literal = check_if_add_type_literal<conf, Type>();
  constexpr bool disable_hash_head = check_if_disable_hash_head<conf, Type>();
  constexpr bool has_container = check_if_has_container<Type>();
  constexpr bool has_compile_time_determined_meta_info =
      check_has_metainfo<conf, Type>();
  serialize_buffer_size ret;
  auto sz_info = calculate_payload_size(args...);

  utils::if_<has_compile_time_determined_meta_info>(
      [&ret]() { ret.len_ = sizeof(unsigned char); });

  utils::if_<!has_container>(
      [&ret]() { ret.len_ += sz_info.total; }, utils::else_([&ret]() {
        if (sz_info.max_size < (int64_t{1} << 8)) {
          ret.len_ += sz_info.total + sz_info.size_cnt;
        } else {
          if (sz_info.max_size < (int64_t{1} << 16)) {
            ret.len_ += sz_info.total + sz_info.size_cnt * 2;
            ret.metainfo_ = 0b01000;
          } else if (sz_info.max_size < (int64_t{1} << 32)) {
            ret.len_ += sz_info.total + sz_info.size_cnt * 4;
            ret.metainfo_ = 0b01000;
          } else {
            ret.len_ += sz_info.total + sz_info.size_cnt * 8;
            ret.metainfo_ = 0b01000;
          }
          utils::if_<!has_compile_time_determined_meta_info>(
              [&ret]() { ret.len_ += sizeof(unsigned char); });
        }
      }));

  utils::if_<!disable_hash_head>([&ret]() {
    ret.len_ += sizeof(uint32_t);

    utils::if_<has_type_literal>([&ret]() {
      constexpr auto type_literal = get_type_literal<Args...>();
      ret.len_ += type_literal.size() + 1;
      ret.metainfo_ |= 0b100;
    });

    utils::if_<has_compatible>([&ret] {
      if (ret.len_ + 2 < (int64_t{1} << 16)) {
        ret.len_ += 2;
        ret.metainfo_ |= 0b01;
      } else if (ret.len_ + 4 < (int64_t{1} << 32)) {
        ret.len_ += 4;
        ret.metainfo_ |= 0b10;
      } else {
        ret.len_ += 8;
        ret.metainfo_ |= 0b11;
      }
    });
  });

  return ret;
}

}  // namespace details
}  // namespace serialize

#endif