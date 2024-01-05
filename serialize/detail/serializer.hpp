/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 12:21:29
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-05 10:08:14
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <type_traits>

#include "detail/calculate_size.hpp"
#include "detail/endian_wrapper.hpp"
#include "detail/reflection.hpp"
#include "detail/type_calculate.hpp"
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
    write_wrapper<sizeof(item)>(writer_, (char *)&item);
  }
  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<id == type_id::bitset_t, int> = 0) {
    write_bytes_array(writer_, (char *)&item, sizeof(item));
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<id == type_id::array_t &&
                           (is_trivial_serializable<T>::value &&
                            is_little_endian_copyable<sizeof(item[0])>),
                       int> = 0) {
    write_bytes_array(writer_, (char *)&item,
                      sizeof(remove_cvref_t<decltype(item)>));
  }
  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<id == type_id::array_t &&
                           !(is_trivial_serializable<T>::value &&
                             is_little_endian_copyable<sizeof(item[0])>),
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
    auto size = item.size() + 1;
    write_wrapper<serialize_conf::kStringLengthField>(writer_, (char *)&size);
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
               is_little_endian_copyable<sizeof(typename T::value_type)>),
          int> = 0) {
    static_assert((serialize_conf::kArrayLengthField > 0) &&
                      (serialize_conf::kArrayLengthField == 1 ||
                       serialize_conf::kArrayLengthField == 2 ||
                       serialize_conf::kArrayLengthField == 4 ||
                       serialize_conf::kArrayLengthField == 8),
                  "");
    auto size = item.size();
    write_wrapper<serialize_conf::kArrayLengthField>(writer_, (char *)&size);
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
                is_little_endian_copyable<sizeof(typename T::value_type)>),
          int> = 0) {
    static_assert((serialize_conf::kArrayLengthField > 0) &&
                      (serialize_conf::kArrayLengthField == 1 ||
                       serialize_conf::kArrayLengthField == 2 ||
                       serialize_conf::kArrayLengthField == 4 ||
                       serialize_conf::kArrayLengthField == 8),
                  "");
    auto size = item.size();
    write_wrapper<serialize_conf::kArrayLengthField>(writer_, (char *)&size);
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
      const T &item, std::enable_if_t<id == type_id::optional_t, int> = 0) {
    bool has_value = item.has_value();
    write_wrapper<sizeof(bool)>(writer_, (char *)&has_value);
    if (has_value) {
      serialize_one(*item);
    }
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<id == type_id::expected_t, int> = 0) {
    bool has_value = item.has_value();
    write_wrapper<sizeof(bool)>(writer_, (char *)&has_value);
    if (has_value) {
      serialize_one(item.value());
    } else {
      serialize_one(item.error());
    }
  }
  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &, std::enable_if_t<id == type_id::monostate_t, int> = 0) {
    static_assert(std::is_same<typename T::value_type, void>::value,
                  "serialize void type will do nothing");
    // do nothing
    return;
  }

  template <type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &,
      std::enable_if_t<id == type_id::struct_t && !user_defined_refl<T>, int> =
          0) {
    static_assert(user_struct<T>,
                  "serializer only support user defined struct when "
                  "add macro SERIALIZE_REFL(Type,field1,field2...)");
    // do nothing
    return;
  }
  // template <type_id id, typename T>
  // constexpr void inline serialize_one_helper(
  //     const T &item,
  //     std::enable_if_t<(id == type_id::struct_t) &&
  //                          (is_trivial_serializable<T>::value &&
  //                           is_little_endian_copyable<sizeof(T)>),
  //                      int> = 0) {
  //   // TODO padding
  //   std::cout << sizeof(T) << " --- " << std::endl;
  //   write_wrapper<sizeof(T)>(writer_, (char *)&item);
  // }
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
      const T &item, std::enable_if_t<(id == type_id::struct_t), int> = 0) {
    if (serialize_conf::kStructLengthField != 0) {
      auto size = calculate_payload_size<serialize_conf>(item);
      write_wrapper<serialize_conf::kStructLengthField>(writer_, (char *)&size);
    }
    visit_members(item, [this](auto &&...items) { serialize_many(items...); });
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