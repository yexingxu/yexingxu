/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 12:21:29
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-11 14:37:37
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <type_traits>

#include "append_types/apply.hpp"
#include "detail/calculate_size.hpp"
#include "detail/endian_wrapper.hpp"
#include "detail/reflection.hpp"
#include "ser_config.hpp"

namespace serialize {
namespace detail {

template <typename writer, typename serialize_conf>
class Serializer {
 public:
  Serializer(writer &w, const std::size_t &in) : writer_(w), info_(in) {
    static_assert(writer_t<writer>,
                  "The writer type must satisfy requirements!");
  }
  Serializer(const Serializer &) = delete;
  Serializer &operator=(const Serializer &) = delete;

  template <typename T, typename... Args>
  inline void serialize(const T &t, const Args &...args) {
    serialize_many(t, args...);
  }

 private:
  constexpr void serialize_many() {}

  template <typename First, typename... Args>
  constexpr void serialize_many(const First &first_item, const Args &...items) {
    serialize_one(first_item);
    if (sizeof...(items) > 0) {
      serialize_many(items...);
    }
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<id == type_id::uint128_t || id == type_id::int128_t ||
                           std::is_fundamental<T>::value ||
                           std::is_enum<T>::value,
                       int> = 0) {
    write_wrapper<serialize_conf::kByteOrder == byte_order::kLittleEndian,
                  sizeof(item)>(writer_, (char *)&item);
  }
  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<id == type_id::bitset_t, int> = 0) {
    write_bytes_array(writer_, (char *)&item, sizeof(item));
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<
          id == type_id::array_t &&
              (is_trivial_serializable<T>::value &&
               is_little_endian_copyable<serialize_conf::kByteOrder ==
                                             byte_order::kLittleEndian,
                                         sizeof(item[0])>),
          int> = 0) {
    write_bytes_array(writer_, (char *)&item,
                      sizeof(remove_cvref_t<decltype(item)>));
  }
  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<
          id == type_id::array_t &&
              !(is_trivial_serializable<T>::value &&
                is_little_endian_copyable<serialize_conf::kByteOrder ==
                                              byte_order::kLittleEndian,
                                          sizeof(item[0])>),
          int> = 0) {
    for (const auto &i : item) {
      serialize_one(i);
    }
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<id == type_id::string_t, int> = 0) {
    static_assert((serialize_conf::kStringLengthField > 0) &&
                      (serialize_conf::kStringLengthField == 1 ||
                       serialize_conf::kStringLengthField == 2 ||
                       serialize_conf::kStringLengthField == 4 ||
                       serialize_conf::kStringLengthField == 8),
                  "");
    uint64_t size = item.size() + 1;
    serialize_length_field<serialize_conf::kStringLengthField>(size);
    using value_type = typename T::value_type;
    write_bytes_array(writer_, (char *)item.data(),
                      item.size() * sizeof(value_type));
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<
          (id == type_id::container_t || id == type_id::map_container_t ||
           id == type_id::set_container_t) &&
              (trivially_copyable_container<T> &&
               is_little_endian_copyable<serialize_conf::kByteOrder ==
                                             byte_order::kLittleEndian,
                                         sizeof(typename T::value_type)>),
          int> = 0) {
    static_assert((serialize_conf::kArrayLengthField > 0) &&
                      (serialize_conf::kArrayLengthField == 1 ||
                       serialize_conf::kArrayLengthField == 2 ||
                       serialize_conf::kArrayLengthField == 4 ||
                       serialize_conf::kArrayLengthField == 8),
                  "");
    uint64_t size = item.size();
    serialize_length_field<serialize_conf::kArrayLengthField>(size);
    using value_type = typename T::value_type;
    write_bytes_array(writer_, (char *)item.data(),
                      item.size() * sizeof(value_type));
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<
          (id == type_id::container_t || id == type_id::map_container_t ||
           id == type_id::set_container_t) &&
              !(trivially_copyable_container<T> &&
                is_little_endian_copyable<serialize_conf::kByteOrder ==
                                              byte_order::kLittleEndian,
                                          sizeof(typename T::value_type)>),
          int> = 0) {
    static_assert((serialize_conf::kArrayLengthField > 0) &&
                      (serialize_conf::kArrayLengthField == 1 ||
                       serialize_conf::kArrayLengthField == 2 ||
                       serialize_conf::kArrayLengthField == 4 ||
                       serialize_conf::kArrayLengthField == 8),
                  "");
    uint64_t size = item.size();
    serialize_length_field<serialize_conf::kArrayLengthField>(size);
    for (const auto &i : item) {
      serialize_one(i);
    }
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<id == type_id::pair_t, int> = 0) {
    serialize_one(item.first);
    serialize_one(item.second);
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<id == type_id::tuple_t, int> = 0) {
    tl::apply([&](auto &&...items) { serialize_many(items...); }, item);
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<id == type_id::variant_t, int> = 0) {
    uint64_t index = item.index();
    serialize_length_field<serialize_conf::kUnionLengthField>(index);
    mpark::visit([this](auto &&e) { this->serialize_one(e); }, item);
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<id == type_id::optional_t, int> = 0) {
    bool has_value = item.has_value();
    write_wrapper<serialize_conf::kByteOrder == byte_order::kLittleEndian,
                  sizeof(bool)>(writer_, (char *)&has_value);
    if (has_value) {
      serialize_one(*item);
    }
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<id == type_id::expected_t, int> = 0) {
    bool has_value = item.has_value();
    write_wrapper<serialize_conf::kByteOrder == byte_order::kLittleEndian,
                  sizeof(bool)>(writer_, (char *)&has_value);
    if (has_value) {
      serialize_one(item.value());
    } else {
      serialize_one(item.error());
    }
  }
  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &, std::enable_if_t<id == type_id::monostate_t, int> = 0) {
    // do nothing
    return;
  }

  // template <type_id id, typename T>
  // constexpr void inline serialize_one_helper(
  //     const T &item, std::enable_if_t<(id == type_id::struct_t), int> = 0) {
  //   // TODO padding
  //   visit_members(item, [this](auto &&...items) {
  //     static_cast<void>(
  //         std::initializer_list<int>{(serialize_one(items),
  //         0)...});
  //   });
  // }
  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<(id == type_id::struct_t) &&
                           serialize_conf::kStructLengthField != 0,
                       int> = 0) {
    auto size_info = calculate_payload_size<serialize_conf>(item);
    uint64_t size = size_info.total + size_info.length_field;
    serialize_length_field<serialize_conf::kStructLengthField>(size);
    visit_members(item, [this](auto &&...items) { serialize_many(items...); });
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<(id == type_id::struct_t) &&
                           serialize_conf::kStructLengthField == 0,
                       int> = 0) {
    visit_members(item, [this](auto &&...items) { serialize_many(items...); });
  }

  template <std::uint8_t length_field>
  constexpr auto inline serialize_length_field(std::uint64_t &size64) {
    std::uint8_t size8 = 0;
    std::uint16_t size16 = 0;
    std::uint32_t size32 = 0;
    switch (length_field) {
      case 0:
        break;
      case 1:
        size8 = static_cast<uint8_t>(size64);
        write_wrapper<serialize_conf::kByteOrder == byte_order::kLittleEndian,
                      1>(writer_, (char *)&size8);
        break;
      case 2:
        size16 = static_cast<uint16_t>(size64);
        write_wrapper<serialize_conf::kByteOrder == byte_order::kLittleEndian,
                      2>(writer_, (char *)&size16);
        break;
      case 4:
        size32 = static_cast<uint32_t>(size64);
        write_wrapper<serialize_conf::kByteOrder == byte_order::kLittleEndian,
                      4>(writer_, (char *)&size32);
        break;
      case 8:
        write_wrapper<serialize_conf::kByteOrder == byte_order::kLittleEndian,
                      8>(writer_, (char *)&size64);
        break;
      default:
        unreachable();
    }
  }

  template <typename T>
  constexpr void inline serialize_one(const T &item) {
    using type = remove_cvref_t<decltype(item)>;
    static_assert(!std::is_pointer<type>::value, "");
    constexpr auto id = get_type_id<type>();
    serialize_one_helper<id, type>(item);
  }

 private:
  writer &writer_;
  const std::size_t &info_;
};

template <typename conf, typename Writer, typename... Args>
void serialize_to(Writer &writer, const std::size_t &info,
                  const Args &...args) {
  static_assert(writer_t<Writer>, "The writer type must satisfy requirements!");
  detail::Serializer<Writer, conf> o(writer, info);
  o.template serialize(args...);
}

}  // namespace detail
}  // namespace serialize