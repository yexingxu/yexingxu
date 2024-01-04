/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 16:11:16
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-04 22:14:51
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <type_traits>

#include "append_types/optional.hpp"
#include "detail/endian_wrapper.hpp"
#include "detail/macro.hpp"
#include "detail/reflection.hpp"
#include "detail/return_code.hpp"
#include "detail/type_id.hpp"
#include "ser_config.hpp"

namespace serialize {
namespace detail {

template <typename Reader, uint64_t conf = ser_config::DEFAULT>
class Deserializer {
 public:
  Deserializer() = delete;
  Deserializer(const Deserializer &) = delete;
  Deserializer &operator=(const Deserializer &) = delete;

  Deserializer(Reader &reader) : reader_(reader) {
    static_assert(reader_t<Reader>,
                  "The reader type must satisfy requirements!");
  }

  template <typename T, typename... Args>
  serialize::return_code deserialize(T &t, Args &...args) {
    serialize::return_code ret{};
    // TODO 将容器长度域作为配置项
    length_field_ = 0;
    switch (length_field_) {
      case 0:
        ret = deserialize_many<1>(t, args...);
        break;
      case 1:
        ret = deserialize_many<2>(t, args...);
        break;
      case 2:
        ret = deserialize_many<4>(t, args...);
        break;
      case 3:
        ret = deserialize_many<8>(t, args...);
        break;
      default:
        unreachable();
    }
    return ret;
  }

 private:
  template <size_t size_type>
  constexpr serialize::return_code deserialize_many() {
    return {};
  }
  template <size_t size_type, typename First, typename... Args>
  constexpr serialize::return_code deserialize_many(First &&first_item,
                                                    Args &&...items) {
    auto code = deserialize_one<size_type>(first_item);
    if SER_UNLIKELY (code != serialize::return_code{}) {
      return code;
    }
    return deserialize_many<size_type>(items...);
  }

  template <size_t size_type, typename T>
  constexpr serialize::return_code inline deserialize_one(T &item) {
    using type = remove_cvref_t<decltype(item)>;
    static_assert(!std::is_pointer<type>::value, "");
    constexpr auto id = get_type_id<type>();
    return deserialize_one_helper<size_type, id>(item);
  }
  template <size_t size_type, type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<
                   std::is_fundamental<remove_cvref_t<decltype(item)>>::value ||
                       std::is_enum<remove_cvref_t<decltype(item)>>::value ||
                       id == type_id::int128_t || id == type_id::uint128_t,
                   int> = 0) {
    if SER_UNLIKELY (!read_wrapper<sizeof(remove_cvref_t<decltype(item)>)>(
                         reader_, (char *)&item)) {
      return return_code::no_buffer_space;
    }

    return return_code::ok;
  }
  template <size_t size_type, type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::bitset_t, int> = 0) {
    if SER_UNLIKELY (!read_bytes_array(
                         reader_, (char *)&item,
                         sizeof(remove_cvref_t<decltype(item)>))) {
      return return_code::no_buffer_space;
    }

    return return_code::ok;
  }
  template <size_t size_type, type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item,
      std::enable_if_t<
          id == type_id::array_t &&
              is_trivial_serializable<remove_cvref_t<decltype(item)>>::value &&
              is_little_endian_copyable<sizeof(item[0])>,
          int> = 0) {
    if SER_UNLIKELY (!read_bytes_array(reader_, (char *)&item, sizeof(item))) {
      return return_code::no_buffer_space;
    }
    return return_code::ok;
  }
  template <size_t size_type, type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item,
      std::enable_if_t<id == type_id::array_t &&
                           !(is_trivial_serializable<
                                 remove_cvref_t<decltype(item)>>::value &&
                             is_little_endian_copyable<sizeof(item[0])>),
                       int> = 0) {
    return_code code{};
    for (auto &i : item) {
      code = deserialize_one<size_type>(i);
      if SER_UNLIKELY (code != return_code::ok) {
        return code;
      }
    }
    return return_code::ok;
  }
  template <size_t size_type, type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::pair_t, int> = 0) {
    return_code code{};
    code = deserialize_one<size_type>(item.first);
    if SER_UNLIKELY (code != return_code::ok) {
      return code;
    }
    code = deserialize_one<size_type>(item.second);
    return code;
  }
  template <size_t size_type, type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::monostate_t, int> = 0) {
    static_assert(std::is_same<void, remove_cvref_t<decltype(item)>>::value,
                  "deserialize void type will do nothing.");
    return return_code::ok;
  }

  template <size_t size_type, type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::struct_t, int> = 0) {
    return_code code{};
    visit_members(item, [&](auto &&...items) {
      //   int i = 1;
      //   auto f = [&](auto &&item) {
      //     code = deserialize_one<size_type>(item);
      //     // if SER_LIKELY (code == return_code::ok) {
      //     //   //   code = ignore_padding(align::padding_size<type>[i++]);
      //     // }
      //     return code == return_code::ok;
      //   };
      bool ret = true;
      static_cast<void>(std::initializer_list<int>{
          (ret &= (deserialize_one<size_type>(items) == return_code::ok),
           0)...});
    });
    return code;
  }

  template <size_t size_type, type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::optional_t, int> = 0) {
    using type = remove_cvref_t<decltype(item)>;
    bool has_value{};
    if SER_UNLIKELY (!read_wrapper<sizeof(bool)>(reader_, (char *)&has_value)) {
      return return_code::no_buffer_space;
    }
    if SER_UNLIKELY (!has_value) {
      return return_code::ok;
    } else {
      item = type{tl::in_place_t{}};
      deserialize_one<size_type>(*item);
    }
    return return_code::ok;
  }
  template <size_t size_type, type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::expected_t, int> = 0) {
    using type = remove_cvref_t<decltype(item)>;
    bool has_value{};
    if SER_UNLIKELY (!read_wrapper<sizeof(bool)>(reader_, (char *)&has_value)) {
      return return_code::no_buffer_space;
    }
    if SER_UNLIKELY (!has_value) {
      item = typename type::unexpected_type{typename type::error_type{}};
      deserialize_one<size_type>(item.error());
    } else {
      // TODO check void
      deserialize_one<size_type>(item.value());
    }
    return return_code::ok;
  }

  template <size_t size_type, type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::map_container_t, int> = 0) {
    return_code code{};
    std::uint64_t size = 0;
    code = deserialize_length_field<size_type>(size);
    if SER_UNLIKELY (code != return_code::ok) {
      return code;
    }
    using type = remove_cvref_t<decltype(item)>;
    std::pair<typename type::key_type, typename type::mapped_type> value{};

    for (std::size_t i = 0; i < size; ++i) {
      code = deserialize_one<size_type>(value);
      if SER_UNLIKELY (code != return_code::ok) {
        return code;
      }
      // TODO: mapped_type can deserialize without be moved
      item.emplace(std::move(value));
    }

    return return_code::ok;
  }
  template <size_t size_type, type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::set_container_t, int> = 0) {
    return_code code{};
    std::uint64_t size = 0;
    code = deserialize_length_field<size_type>(size);
    if SER_UNLIKELY (code != return_code::ok) {
      return code;
    }
    using type = remove_cvref_t<decltype(item)>;
    typename type::value_type value{};

    for (std::size_t i = 0; i < size; ++i) {
      code = deserialize_one<size_type>(value);
      if SER_UNLIKELY (code != return_code::ok) {
        return code;
      }
      // TODO: set_type can deserialize without be moved
      item.emplace(std::move(value));
    }

    return return_code::ok;
  }

  template <size_t size_type, type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item,
      std::enable_if_t<
          (id == type_id::container_t || id == type_id::string_t) &&
              (trivially_copyable_container<remove_cvref_t<decltype(item)>> &&
               is_little_endian_copyable<sizeof(
                   typename remove_cvref_t<decltype(item)>::value_type)>),
          int> = 0) {
    return_code code{};
    std::uint64_t size = 0;
    code = deserialize_length_field<size_type>(size);
    if SER_UNLIKELY (code != return_code::ok) {
      return code;
    }

    using type = remove_cvref_t<decltype(item)>;
    using value_type = typename type::value_type;
    uint64_t mem_sz = size * sizeof(value_type);

    if SER_UNLIKELY (mem_sz >= PTRDIFF_MAX)
      unreachable();
    else {
      item.resize(size);
      if SER_UNLIKELY (!read_bytes_array(reader_, (char *)item.data(),
                                         size * sizeof(value_type))) {
        return return_code::no_buffer_space;
      }
    }
    return return_code::ok;
  }

  template <size_t size_type, type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item,
      std::enable_if_t<
          (id == type_id::container_t || id == type_id::string_t) &&
              !(trivially_copyable_container<remove_cvref_t<decltype(item)>> &&
                is_little_endian_copyable<sizeof(
                    typename remove_cvref_t<decltype(item)>::value_type)>),
          int> = 0) {
    return_code code{};
    std::uint64_t size = 0;
    code = deserialize_length_field<size_type>(size);
    if SER_UNLIKELY (code != return_code::ok) {
      return code;
    }

    item.resize(size);
    for (auto &i : item) {
      code = deserialize_one<size_type>(i);
      if SER_UNLIKELY (code != return_code::ok) {
        return code;
      }
    }
    return return_code::ok;
  }

  template <size_t size_type>
  constexpr serialize::return_code inline deserialize_length_field(
      std::uint64_t &size64) {
    std::uint8_t size8 = 0;
    std::uint16_t size16 = 0;
    std::uint32_t size32 = 0;
    switch (size_type) {
      case 1:
        if SER_UNLIKELY (!read_wrapper<1>(reader_, (char *)&size8)) {
          return return_code::no_buffer_space;
        }
        size64 = size8;
        break;
      case 2:
        if SER_UNLIKELY (!read_wrapper<2>(reader_, (char *)&size16)) {
          return return_code::no_buffer_space;
        }
        size64 = size16;
        break;
      case 4:
        if SER_UNLIKELY (!read_wrapper<4>(reader_, (char *)&size32)) {
          return return_code::no_buffer_space;
        }
        size64 = size32;
        break;
      case 8:
        if SER_UNLIKELY (!read_wrapper<8>(reader_, (char *)&size64)) {
          return return_code::no_buffer_space;
        }
        break;
      default:
        unreachable();
    }
    if SER_UNLIKELY (size64 == 0) {
      return return_code::length_error;
    }
    return return_code::ok;
  }

 private:
  std::uint8_t length_field_;
  std::size_t data_len_;

  Reader &reader_;
};

}  // namespace detail
}  // namespace serialize