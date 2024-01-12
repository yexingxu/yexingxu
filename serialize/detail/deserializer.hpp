/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-28 16:11:16
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-12 13:20:22
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <type_traits>

#include "append_types/apply.hpp"
#include "append_types/optional.hpp"
#include "detail/endian_wrapper.hpp"
#include "detail/macro.hpp"
#include "detail/reflection.hpp"
#include "detail/return_code.hpp"
#include "detail/type_id.hpp"
#include "ser_config.hpp"

namespace serialize {
namespace detail {

template <typename Reader, typename serialize_conf = serialize_props_default>
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
    return deserialize_many(t, args...);
  }

 private:
  constexpr serialize::return_code deserialize_many() {
    return return_code::ok;
  }
  template <typename First, typename... Args>
  constexpr serialize::return_code deserialize_many(First &&first_item,
                                                    Args &&...items) {
    auto code = deserialize_one(first_item);
    if SER_UNLIKELY (code != serialize::return_code::ok) {
      return code;
    }
    return deserialize_many(items...);
  }

  template <typename T>
  constexpr serialize::return_code inline deserialize_one(T &item) {
    using type = remove_cvref_t<decltype(item)>;
    static_assert(
        !std::is_pointer<type>::value,
        "deserializer is not support a raw pointer, use smart ptr replace.");
    constexpr auto id = get_type_id<type>();
    return deserialize_one_helper<id>(item);
  }
  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<
                   std::is_fundamental<remove_cvref_t<decltype(item)>>::value ||
                       std::is_enum<remove_cvref_t<decltype(item)>>::value ||
                       id == type_id::int128_t || id == type_id::uint128_t,
                   int> = 0) {
    if SER_UNLIKELY ((!read_wrapper<serialize_conf::kByteOrder ==
                                        byte_order::kLittleEndian,
                                    sizeof(remove_cvref_t<decltype(item)>)>(
                         reader_, (char *)&item))) {
      return return_code::no_buffer_space;
    }

    return return_code::ok;
  }
  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::bitset_t, int> = 0) {
    if SER_UNLIKELY (!read_bytes_array(
                         reader_, (char *)&item,
                         sizeof(remove_cvref_t<decltype(item)>))) {
      return return_code::no_buffer_space;
    }

    return return_code::ok;
  }
  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item,
      std::enable_if_t<
          id == type_id::array_t &&
              is_trivial_serializable<remove_cvref_t<decltype(item)>>::value &&
              is_little_endian_copyable<serialize_conf::kByteOrder ==
                                            byte_order::kLittleEndian,
                                        sizeof(item[0])>,
          int> = 0) {
    if SER_UNLIKELY (!read_bytes_array(reader_, (char *)&item, sizeof(item))) {
      return return_code::no_buffer_space;
    }
    return return_code::ok;
  }
  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item,
      std::enable_if_t<
          id == type_id::array_t &&
              !(is_trivial_serializable<
                    remove_cvref_t<decltype(item)>>::value &&
                is_little_endian_copyable<serialize_conf::kByteOrder ==
                                              byte_order::kLittleEndian,
                                          sizeof(item[0])>),
          int> = 0) {
    return_code code{};
    for (auto &i : item) {
      code = deserialize_one(i);
      if SER_UNLIKELY (code != return_code::ok) {
        return code;
      }
    }
    return return_code::ok;
  }
  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::pair_t, int> = 0) {
    return_code code{};
    code = deserialize_one(item.first);
    if SER_UNLIKELY (code != return_code::ok) {
      return code;
    }
    code = deserialize_one(item.second);
    return code;
  }
  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &, std::enable_if_t<id == type_id::monostate_t, int> = 0) {
    return return_code::ok;
  }

  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::variant_t, int> = 0) {
    uint64_t index{};
    auto code =
        deserialize_length_field<serialize_conf::kUnionLengthField>(index);
    if SER_UNLIKELY (code != return_code::ok) {
      return code;
    }
    if SER_UNLIKELY (index >= mpark::variant_size_v<remove_cvref_t<T>>) {
      return return_code::invalid_buffer;
    } else {
      template_switch<variant_construct_helper>(index, *this, item);
    }

    return return_code::ok;
  }

  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::struct_t, int> = 0) {
    static_assert((serialize_conf::kStructLengthField == 0) ||
                      (serialize_conf::kStructLengthField == 1 ||
                       serialize_conf::kStructLengthField == 2 ||
                       serialize_conf::kStructLengthField == 4 ||
                       serialize_conf::kStructLengthField == 8),
                  "");
    return_code code{};
    std::uint64_t size = 0;
    code = deserialize_length_field<serialize_conf::kStructLengthField>(size);
    if SER_UNLIKELY (code != return_code::ok) {
      return code;
    }
    visit_members(item, [&](auto &&...items) {
      bool ret = true;
      static_cast<void>(std::initializer_list<int>{
          (ret &= (deserialize_one(items) == return_code::ok), 0)...});
    });
    return code;
  }

  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::tuple_t, int> = 0) {
    return_code code{};
    tl::apply([&](auto &&...items) { code = deserialize_many(items...); },
              item);
    return code;
  }

  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item,
      std::enable_if_t<id == type_id::pointer_t && unique_ptr<T>, int> = 0) {
    return_code code{};
    using type = remove_cvref_t<decltype(item)>;
    item = std::make_unique<typename type::element_type>();
    deserialize_one(*item);
    return code;
  }
  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item,
      std::enable_if_t<id == type_id::pointer_t && shared_ptr<T>, int> = 0) {
    return_code code{};
    using type = remove_cvref_t<decltype(item)>;
    item = std::make_shared<typename type::element_type>();
    deserialize_one(*item);
    return code;
  }

  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::optional_t, int> = 0) {
    using type = remove_cvref_t<decltype(item)>;
    bool has_value{};
    if SER_UNLIKELY ((!read_wrapper<serialize_conf::kByteOrder ==
                                        byte_order::kLittleEndian,
                                    sizeof(bool)>(reader_,
                                                  (char *)&has_value))) {
      return return_code::no_buffer_space;
    }
    if SER_UNLIKELY (!has_value) {
      return return_code::ok;
    } else {
      item = type{tl::in_place_t{}};
      deserialize_one(*item);
    }
    return return_code::ok;
  }
  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::expected_t, int> = 0) {
    using type = remove_cvref_t<decltype(item)>;
    bool has_value{};
    if SER_UNLIKELY ((!read_wrapper<serialize_conf::kByteOrder ==
                                        byte_order::kLittleEndian,
                                    sizeof(bool)>(reader_,
                                                  (char *)&has_value))) {
      return return_code::no_buffer_space;
    }
    if SER_UNLIKELY (!has_value) {
      item = typename type::unexpected_type{typename type::error_type{}};
      deserialize_one(item.error());
    } else {
      // TODO check void
      deserialize_one(item.value());
    }
    return return_code::ok;
  }

  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::map_container_t, int> = 0) {
    static_assert((serialize_conf::kArrayLengthField > 0) &&
                      (serialize_conf::kArrayLengthField == 1 ||
                       serialize_conf::kArrayLengthField == 2 ||
                       serialize_conf::kArrayLengthField == 4 ||
                       serialize_conf::kArrayLengthField == 8),
                  "");
    return_code code{};
    std::uint64_t size = 0;
    code = deserialize_length_field<serialize_conf::kArrayLengthField>(size);
    if SER_UNLIKELY (code != return_code::ok) {
      return code;
    }
    using type = remove_cvref_t<decltype(item)>;
    std::pair<typename type::key_type, typename type::mapped_type> value{};

    for (std::size_t i = 0; i < size; ++i) {
      code = deserialize_one(value);
      if SER_UNLIKELY (code != return_code::ok) {
        return code;
      }
      // TODO: mapped_type can deserialize without be moved
      item.emplace(std::move(value));
    }

    return return_code::ok;
  }
  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::set_container_t, int> = 0) {
    static_assert((serialize_conf::kArrayLengthField > 0) &&
                      (serialize_conf::kArrayLengthField == 1 ||
                       serialize_conf::kArrayLengthField == 2 ||
                       serialize_conf::kArrayLengthField == 4 ||
                       serialize_conf::kArrayLengthField == 8),
                  "");
    return_code code{};
    std::uint64_t size = 0;
    code = deserialize_length_field<serialize_conf::kArrayLengthField>(size);
    if SER_UNLIKELY (code != return_code::ok) {
      return code;
    }
    using type = remove_cvref_t<decltype(item)>;
    typename type::value_type value{};

    for (std::size_t i = 0; i < size; ++i) {
      code = deserialize_one(value);
      if SER_UNLIKELY (code != return_code::ok) {
        return code;
      }
      // TODO: set_type can deserialize without be moved
      item.emplace(std::move(value));
    }

    return return_code::ok;
  }

  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item, std::enable_if_t<id == type_id::string_t, int> = 0) {
    static_assert((serialize_conf::kStringLengthField > 0) &&
                      (serialize_conf::kStringLengthField == 1 ||
                       serialize_conf::kStringLengthField == 2 ||
                       serialize_conf::kStringLengthField == 4 ||
                       serialize_conf::kStringLengthField == 8),
                  "");
    return_code code{};
    std::uint64_t size = 0;
    code = deserialize_length_field<serialize_conf::kStringLengthField>(size);
    if SER_UNLIKELY (code != return_code::ok) {
      return code;
    }

    using type = remove_cvref_t<decltype(item)>;
    using value_type = typename type::value_type;
    uint64_t mem_sz = size * sizeof(value_type);

    if SER_UNLIKELY (mem_sz >= PTRDIFF_MAX) {
      unreachable();
    } else {
      item.resize(size - 1);
      if SER_UNLIKELY (!read_bytes_array(reader_, (char *)item.data(),
                                         (size - 1) * sizeof(value_type))) {
        return return_code::no_buffer_space;
      }
    }

    return return_code::ok;
  }

  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item,
      std::enable_if_t<
          (id == type_id::container_t) &&
              (trivially_copyable_container<remove_cvref_t<decltype(item)>> &&
               is_little_endian_copyable<
                   serialize_conf::kByteOrder == byte_order::kLittleEndian,
                   sizeof(
                       typename remove_cvref_t<decltype(item)>::value_type)>),
          int> = 0) {
    static_assert((serialize_conf::kArrayLengthField > 0) &&
                      (serialize_conf::kArrayLengthField == 1 ||
                       serialize_conf::kArrayLengthField == 2 ||
                       serialize_conf::kArrayLengthField == 4 ||
                       serialize_conf::kArrayLengthField == 8),
                  "");
    return_code code{};
    std::uint64_t size = 0;
    code = deserialize_length_field<serialize_conf::kArrayLengthField>(size);
    if SER_UNLIKELY (code != return_code::ok) {
      return code;
    }

    using type = remove_cvref_t<decltype(item)>;
    using value_type = typename type::value_type;
    uint64_t mem_sz = size * sizeof(value_type);

    if SER_UNLIKELY (mem_sz >= PTRDIFF_MAX) {
      unreachable();
    } else {
      item.resize(size);
      if SER_UNLIKELY (!read_bytes_array(reader_, (char *)item.data(),
                                         size * sizeof(value_type))) {
        return return_code::no_buffer_space;
      }
    }
    return return_code::ok;
  }

  template <type_id id, typename T>
  constexpr serialize::return_code inline deserialize_one_helper(
      T &item,
      std::enable_if_t<
          (id == type_id::container_t) &&
              !(trivially_copyable_container<remove_cvref_t<decltype(item)>> &&
                is_little_endian_copyable<
                    serialize_conf::kByteOrder == byte_order::kLittleEndian,
                    sizeof(
                        typename remove_cvref_t<decltype(item)>::value_type)>),
          int> = 0) {
    static_assert((serialize_conf::kArrayLengthField > 0) &&
                      (serialize_conf::kArrayLengthField == 1 ||
                       serialize_conf::kArrayLengthField == 2 ||
                       serialize_conf::kArrayLengthField == 4 ||
                       serialize_conf::kArrayLengthField == 8),
                  "");
    return_code code{};
    std::uint64_t size = 0;
    code = deserialize_length_field<serialize_conf::kArrayLengthField>(size);
    if SER_UNLIKELY (code != return_code::ok) {
      return code;
    }

    item.resize(size);
    for (auto &i : item) {
      code = deserialize_one(i);
      if SER_UNLIKELY (code != return_code::ok) {
        return code;
      }
    }

    return return_code::ok;
  }

  template <std::uint8_t length_field>
  constexpr serialize::return_code inline deserialize_length_field(
      std::uint64_t &size64) {
    std::uint8_t size8 = 0;
    std::uint16_t size16 = 0;
    std::uint32_t size32 = 0;
    constexpr bool is_little_endian =
        serialize_conf::kByteOrder == byte_order::kLittleEndian;
    switch (length_field) {
      case 0:
        break;
      case 1:
        if SER_UNLIKELY ((!read_wrapper<is_little_endian, 1>(reader_,
                                                             (char *)&size8))) {
          return return_code::no_buffer_space;
        }
        size64 = size8;
        break;
      case 2:
        if SER_UNLIKELY ((!read_wrapper<is_little_endian, 2>(
                             reader_, (char *)&size16))) {
          return return_code::no_buffer_space;
        }
        size64 = size16;
        break;
      case 4:
        if SER_UNLIKELY ((!read_wrapper<is_little_endian, 4>(
                             reader_, (char *)&size32))) {
          return return_code::no_buffer_space;
        }
        size64 = size32;
        break;
      case 8:
        if SER_UNLIKELY ((!read_wrapper<is_little_endian, 8>(
                             reader_, (char *)&size64))) {
          return return_code::no_buffer_space;
        }
        break;
      default:
        unreachable();
    }
    if SER_UNLIKELY (length_field != 0 && size64 == 0) {
      return return_code::length_error;
    }
    return return_code::ok;
  }

 private:
  struct variant_construct_helper {
    template <size_t index, typename deserializer, typename variant_t,
              std::enable_if_t<index<mpark::variant_size_v<variant_t>, int> =
                                   0> static inline constexpr void
                  run(deserializer &des, variant_t &v) {
      v = variant_t{mpark::in_place_index_t<index>{}};
      des.template deserialize_one(mpark::get<index>(v));
    }

    template <
        size_t index, typename deserializer, typename variant_t,
        std::enable_if_t<index >= mpark::variant_size_v<variant_t>, int> = 0>
    static inline constexpr void run(deserializer &, variant_t &) {
      unreachable();
      return;
    }
  };

 private:
  Reader &reader_;
};

}  // namespace detail
}  // namespace serialize