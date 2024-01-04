/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 12:21:29
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-04 20:44:40
 */
#pragma once

#include <cstdint>
#include <initializer_list>
#include <iostream>

#include "detail/calculate_size.hpp"
#include "detail/endian_wrapper.hpp"
#include "detail/reflection.hpp"
#include "detail/type_calculate.hpp"
#include "ser_config.hpp"

namespace serialize {
namespace detail {

template <typename writer, typename serialize_type>
class Serializer {
 public:
  Serializer(writer &w, const serialize_buffer_size &in)
      : writer_(w), info_(in) {
    static_assert(writer_t<writer>,
                  "The writer type must satisfy requirements!");
  }
  Serializer(const Serializer &) = delete;
  Serializer &operator=(const Serializer &) = delete;

  template <std::size_t size_type, typename T, typename... Args>
  inline void serialize(const T &t, const Args &...args) {
    serialize_many<size_type>(t, args...);
  }

 private:
  template <std::size_t size_type>
  constexpr void serialize_many() {}

  template <std::size_t size_type, typename First, typename... Args>
  constexpr void serialize_many(const First &first_item, const Args &...items) {
    serialize_one<size_type>(first_item);
    if (sizeof...(items) > 0) {
      serialize_many<size_type>(items...);
    }
  }

  template <std::size_t size_type, type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<id == type_id::uint128_t || id == type_id::int128_t ||
                           std::is_fundamental<T>::value ||
                           std::is_enum<T>::value,
                       int> = 0) {
    write_wrapper<sizeof(item)>(writer_, (char *)&item);
  }
  template <std::size_t size_type, type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<id == type_id::bitset_t, int> = 0) {
    write_bytes_array(writer_, (char *)&item, sizeof(item));
  }

  template <std::size_t size_type, type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<id == type_id::array_t &&
                           (is_trivial_serializable<T>::value &&
                            is_little_endian_copyable<sizeof(item[0])>),
                       int> = 0) {
    write_bytes_array(writer_, (char *)&item,
                      sizeof(remove_cvref_t<decltype(item)>));
  }
  template <std::size_t size_type, type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<id == type_id::array_t &&
                           !(is_trivial_serializable<T>::value &&
                             is_little_endian_copyable<sizeof(item[0])>),
                       int> = 0) {
    for (const auto &i : item) {
      serialize_one<size_type>(i);
    }
  }

  template <std::size_t size_type, type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<
          (id == type_id::container_t || id == type_id::map_container_t ||
           id == type_id::set_container_t || id == type_id::string_t) &&
              (trivially_copyable_container<T> &&
               is_little_endian_copyable<sizeof(typename T::value_type)>),
          int> = 0) {
    uint8_t size = item.size();
    write_wrapper<size_type>(writer_, (char *)&size);
    using value_type = typename T::value_type;
    write_bytes_array(writer_, (char *)item.data(),
                      item.size() * sizeof(value_type));
  }

  template <std::size_t size_type, type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item,
      std::enable_if_t<
          (id == type_id::container_t || id == type_id::map_container_t ||
           id == type_id::set_container_t || id == type_id::string_t) &&
              !(trivially_copyable_container<T> &&
                is_little_endian_copyable<sizeof(typename T::value_type)>),
          int> = 0) {
    uint8_t size = item.size();
    write_wrapper<size_type>(writer_, (char *)&size);
    for (const auto &i : item) {
      serialize_one<size_type>(i);
    }
  }

  template <std::size_t size_type, type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<id == type_id::pair_t, int> = 0) {
    serialize_one<size_type>(item.first);
    serialize_one<size_type>(item.second);
  }

  template <std::size_t size_type, type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<id == type_id::optional_t, int> = 0) {
    bool has_value = item.has_value();
    write_wrapper<sizeof(bool)>(writer_, (char *)&has_value);
    if (has_value) {
      serialize_one<size_type>(*item);
    }
  }

  template <std::size_t size_type, type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<id == type_id::expected_t, int> = 0) {
    bool has_value = item.has_value();
    write_wrapper<sizeof(bool)>(writer_, (char *)&has_value);
    if (has_value) {
      serialize_one<size_type>(item.value());
    } else {
      serialize_one<size_type>(item.error());
    }
  }
  template <std::size_t size_type, type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &, std::enable_if_t<id == type_id::monostate_t, int> = 0) {
    static_assert(std::is_same<typename T::value_type, void>::value,
                  "serialize void type will do nothing");
    // do nothing
    return;
  }

  template <std::size_t size_type, type_id id, typename T>
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
  // template <std::size_t size_type, type_id id, typename T>
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
  // template <std::size_t size_type, type_id id, typename T>
  // constexpr void inline serialize_one_helper(
  //     const T &item, std::enable_if_t<(id == type_id::struct_t), int> = 0) {
  //   // TODO padding
  //   visit_members(item, [this](auto &&...items) {
  //     static_cast<void>(
  //         std::initializer_list<int>{(serialize_one<size_type>(items),
  //         0)...});
  //   });
  // }
  template <std::size_t size_type, type_id id, typename T>
  constexpr void inline serialize_one_helper(
      const T &item, std::enable_if_t<(id == type_id::struct_t), int> = 0) {
    visit_members(
        item, [this](auto &&...items) { serialize_many<size_type>(items...); });
  }

  template <std::size_t size_type, typename T>
  constexpr void inline serialize_one(const T &item) {
    using type = remove_cvref_t<decltype(item)>;
    static_assert(!std::is_pointer<type>::value, "");
    constexpr auto id = get_type_id<type>();
    serialize_one_helper<size_type, id, type>(item);
  }

  writer &writer_;
  const serialize_buffer_size &info_;
};

template <uint64_t conf = ser_config::DEFAULT, typename Writer,
          typename... Args>
void serialize_to(Writer &writer, const serialize_buffer_size &info,
                  const Args &...args) {
  static_assert(sizeof...(args) > 0, "");
  detail::Serializer<Writer, detail::get_args_type<Args...>> o(writer, info);
  switch ((info.metainfo() & 0b11000) >> 3) {
    case 0:
      o.template serialize<1>(args...);
      break;
    case 1:
      o.template serialize<2>(args...);
      break;
    case 2:
      o.template serialize<4>(args...);
      break;
    case 3:
      o.template serialize<8>(args...);
      break;
    default:
      detail::unreachable();
      break;
  };
}

}  // namespace detail
}  // namespace serialize