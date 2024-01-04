/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-29 02:28:26
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-28 02:31:41
 */

#ifndef SERIALIZE_CALCULATE_SIZE_HPP_
#define SERIALIZE_CALCULATE_SIZE_HPP_

#include <cstddef>
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

template <typename T, typename type = remove_cvref_t<T>,
          type_id id = get_type_id<remove_cvref_t<T>>()>
constexpr size_info inline calculate_one_size(
    const T &item,
    std::enable_if_t<std::is_fundamental<type>::value ||
                         std::is_enum<type>::value ||
                         (id == type_id::int128_t || id == type_id::uint128_t ||
                          id == type_id::bitset_t),
                     int> = 0) {
  size_info info{};
  info.total = sizeof(remove_cvref_t<decltype(item)>);
  return info;
}

template <typename T, typename type = remove_cvref_t<T>,
          type_id id = get_type_id<remove_cvref_t<T>>()>
constexpr size_info inline calculate_one_size(
    const T &item, std::enable_if_t<is_trivial_view_v<type>, int> = 0) {
  return calculate_one_size(item.get());
  ;
}

template <typename T, typename type = remove_cvref_t<T>,
          type_id id = get_type_id<remove_cvref_t<T>>()>
constexpr size_info inline calculate_one_size(
    const T &item, std::enable_if_t<id == type_id::array_t &&
                                        is_trivial_serializable<type>::value,
                                    int> = 0) {
  size_info info{};
  info.total = sizeof(remove_cvref_t<decltype(item)>);
  return info;
}

template <typename T, typename type = remove_cvref_t<T>,
          type_id id = get_type_id<remove_cvref_t<T>>()>
constexpr size_info inline calculate_one_size(
    const T &item, std::enable_if_t<id == type_id::array_t &&
                                        !is_trivial_serializable<type>::value,
                                    int> = 0) {
  size_info info{};
  for (auto &i : item) {
    info += calculate_one_size(i);
  }
  return info;
}

template <typename T, typename type = remove_cvref_t<T>,
          type_id id = get_type_id<remove_cvref_t<T>>()>
constexpr size_info inline calculate_one_size(
    const T &item,
    std::enable_if_t<(container<type> &&
                      !array<T>)&&trivially_copyable_container<type>,
                     int> = 0) {
  size_info info{};
  info.size_cnt += 1;
  info.max_size = item.size();
  using value_type = typename type::value_type;
  info.total = item.size() * sizeof(value_type);
  return info;
}

template <typename T, typename type = remove_cvref_t<T>,
          type_id id = get_type_id<remove_cvref_t<T>>()>
constexpr size_info inline calculate_one_size(
    const T &item,
    std::enable_if_t<(container<type> &&
                      !array<T>)&&!trivially_copyable_container<type>,
                     int> = 0) {
  size_info info{};
  info.size_cnt += 1;
  info.max_size = item.size();
  for (auto &&i : item) {
    info += calculate_one_size(i);
  }
  return info;
}

// template <typename T, typename type = remove_cvref_t<T>,
//           type_id id = get_type_id<remove_cvref_t<T>>()>
// constexpr size_info inline calculate_one_size(
//     const T &,
//     std::enable_if_t<pair<T> && is_trivial_serializable<type>::value, int> =
//         0) {
//   size_info info{};
//   info.total = sizeof(type);
//   return info;
// }

// template <typename T, typename type = remove_cvref_t<T>,
//           type_id id = get_type_id<remove_cvref_t<T>>()>
// constexpr size_info inline calculate_one_size(
//     const T &item,
//     std::enable_if_t<pair<T> && is_trivial_serializable<type, true>::value,
//                      int> = 0) {
//   size_info info{};
//   visit_members(item, [&](auto &&...items) {
//     info += calculate_payload_size(items...);
//     info.total += align::total_padding_size<type>;
//   });
//   return info;
// }

// template <typename T, typename type = remove_cvref_t<T>,
//           type_id id = get_type_id<remove_cvref_t<T>>()>
// constexpr size_info inline calculate_one_size(
//     const T &item,
//     std::enable_if_t<pair<T> && !is_trivial_serializable<type, true>::value
//     &&
//                          !is_trivial_serializable<type>::value,
//                      int> = 0) {
//   size_info info{};
//   visit_members(
//       item, [&](auto &&...items) { info += calculate_payload_size(items...);
//       });

//   return info;
// }

// template <typename T, typename type = remove_cvref_t<T>,
//           type_id id = get_type_id<remove_cvref_t<T>>()>
// constexpr size_info inline calculate_one_size(
//     const T &item, std::enable_if_t<pair<T>, int> = 0) {
//   size_info info{};
//   visit_members(
//       item, [&](auto &&...items) { info += calculate_payload_size(items...);
//       });

//   return info;
// }

template <typename... Args>
constexpr size_info inline calculate_payload_size(const Args &...items) {
  size_info info{0, 0, 0};
  static_cast<void>(
      std::initializer_list<int>{(info += calculate_one_size(items), 0)...});
  return info;
}

template <uint64_t conf, typename... Args>
constexpr serialize_buffer_size get_serialize_runtime_info(const Args &...args);

}  // namespace details

struct serialize_buffer_size {
 public:
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

template <bool has_compile_time_determined_meta_info>
constexpr void has_compile_time_determined_meta_info_helper(
    std::size_t &len,
    std::enable_if_t<has_compile_time_determined_meta_info, int> = 0) {
  len += sizeof(unsigned char);
}
template <bool has_compile_time_determined_meta_info>
constexpr void has_compile_time_determined_meta_info_helper(
    std::size_t &,
    std::enable_if_t<!has_compile_time_determined_meta_info, int> = 0) {}

template <bool has_container, bool has_compile_time_determined_meta_info>
constexpr void has_container_helper(serialize_buffer_size &ret,
                                    size_info &sz_info,
                                    std::enable_if_t<!has_container, int> = 0) {
  ret.len_ += sz_info.total;
}

template <bool has_container, bool has_compile_time_determined_meta_info>
constexpr void has_container_helper(serialize_buffer_size &ret,
                                    size_info &sz_info,
                                    std::enable_if_t<has_container, int> = 0) {
  if (sz_info.max_size < (int64_t{1} << 8)) {
    ret.len_ += sz_info.total + sz_info.size_cnt;
  } else {
    if (sz_info.max_size < (int64_t{1} << 16)) {
      ret.len_ += sz_info.total + sz_info.size_cnt * 2;
      ret.metainfo_ = 0b01000;
    } else if (sz_info.max_size < (int64_t{1} << 32)) {
      ret.len_ += sz_info.total + sz_info.size_cnt * 4;
      ret.metainfo_ = 0b10000;
    } else {
      ret.len_ += sz_info.total + sz_info.size_cnt * 8;
      ret.metainfo_ = 0b11000;
    }
    has_compile_time_determined_meta_info_helper<
        has_compile_time_determined_meta_info>(ret.len_);
  }
}

template <std::uint64_t conf, typename... Args>
constexpr serialize_buffer_size get_serialize_runtime_info(
    const Args &...args) {
  using Type = get_args_type<Args...>;
  constexpr bool has_compatible = serialize_static_config<Type>::has_compatible;
  // constexpr bool has_compatible = false;
  constexpr bool has_type_literal = check_if_add_type_literal<conf, Type>();
  constexpr bool disable_hash_head = check_if_disable_hash_head<conf, Type>();
  constexpr bool has_container = check_if_has_container<Type>();
  constexpr bool has_compile_time_determined_meta_info =
      check_has_metainfo<conf, Type>();
  std::cout << has_compatible << has_type_literal << disable_hash_head
            << has_container << has_compile_time_determined_meta_info
            << std::endl;
  serialize_buffer_size ret;
  auto sz_info = calculate_payload_size(args...);

  has_compile_time_determined_meta_info_helper<
      has_compile_time_determined_meta_info>(ret.len_);

  has_container_helper<has_container, has_compile_time_determined_meta_info>(
      ret, sz_info);

  if (!disable_hash_head) {
    ret.len_ += sizeof(uint32_t);  // for record hash code
    if (has_type_literal) {
      constexpr auto type_literal = serialize::get_type_literal<Args...>();
      // struct_pack::get_type_literal<Args...>().size() crash in clang13.
      // Bug?
      ret.len_ += type_literal.size() + 1;
      ret.metainfo_ |= 0b100;
    }
    if (has_compatible) {  // calculate bytes count of serialize
                           // length
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
    }
  }

  return ret;
}

}  // namespace details
}  // namespace serialize

#endif