/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-12 17:07:21
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-13 02:32:09
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <type_traits>

#include "alignment.hpp"
#include "calculate_size.hpp"
#include "endian_wrapper.hpp"
#include "error_code.hpp"
#include "md5_constexpr.hpp"
#include "memory_reader.hpp"
// #include "packer.hpp"
#include "macro.hpp"
#include "reflection.hpp"
#include "trivial_view.hpp"
#include "type_calculate.hpp"
#include "type_id.hpp"
#include "type_traits.hpp"
// #include "varint.hpp"
namespace serialize {
namespace details {

template <typename Reader, uint64_t conf = sp_config::DEFAULT>
class Deserializer {
 public:
  Deserializer() = delete;
  Deserializer(const Deserializer &) = delete;
  Deserializer &operator=(const Deserializer &) = delete;

  template <typename DerivedClasses, typename size_type, typename version,
            typename NotSkip>
  friend struct deserialize_one_derived_class_helper;

  Deserializer(Reader &reader) : reader_(reader) {
    static_assert(reader_t<Reader>,
                  "The reader type must satisfy requirements!");
  }

  template <
      typename T, typename... Args, typename Type = get_args_type<T, Args...>,
      std::enable_if_t<
          check_if_compatible_element_exist<decltype(get_types<Type>())>(),
          int> = 0>
  serialize::errc deserialize(T &t, Args &...args) {
    data_len_ = reader_.tellg();

    auto &&metainfo = deserialize_metainfo<Type>();
    auto &err_code = metainfo.first;
    auto &buffer_len = metainfo.second;
    if (err_code != serialize::errc{}) {
      return err_code;
    }
    data_len_ += buffer_len;
    switch (size_type_) {
      case 0:
        err_code = deserialize_many<1, UINT64_MAX, true>(t, args...);
        break;
      case 1:
        err_code = deserialize_many<2, UINT64_MAX, true>(t, args...);
        break;
      case 2:
        err_code = deserialize_many<4, UINT64_MAX, true>(t, args...);
        break;
      case 3:
        err_code = deserialize_many<8, UINT64_MAX, true>(t, args...);
        break;
      default:
        unreachable();
    }

    if (err_code != errc::ok) {
      return err_code;
    }
    constexpr std::size_t sz = compatible_version_number<Type>.size();
    err_code = deserialize_compatibles<T, Args...>(
        t, std::make_index_sequence<sz>{}, args...);
    return err_code;
  }

  template <
      typename T, typename... Args, typename Type = get_args_type<T, Args...>,
      std::enable_if_t<
          !check_if_compatible_element_exist<decltype(get_types<Type>())>(),
          int> = 0>
  serialize::errc deserialize(T &t, Args &...args) {
    auto &&metainfo = deserialize_metainfo<Type>();
    auto &err_code = metainfo.first;
    // auto &buffer_len = metainfo.second;
    if (err_code != serialize::errc{}) {
      return err_code;
    }

    switch (size_type_) {
      case 0:
        err_code = deserialize_many<1, UINT64_MAX, true>(t, args...);
        break;
      case 1:
        err_code = deserialize_many<2, UINT64_MAX, true>(t, args...);
        break;
      case 2:
        err_code = deserialize_many<4, UINT64_MAX, true>(t, args...);
        break;
      case 3:
        err_code = deserialize_many<8, UINT64_MAX, true>(t, args...);
        break;
      default:
        unreachable();
    }

    return err_code;
  }

  template <
      typename T, typename... Args, typename Type = get_args_type<T, Args...>,
      std::enable_if_t<
          check_if_compatible_element_exist<decltype(get_types<Type>())>(),
          int> = 0>
  serialize::errc deserialize_with_len(std::size_t &len, T &t, Args &...args) {
    data_len_ = reader_.tellg();
    auto &&metainfo = deserialize_metainfo<Type>();
    auto &err_code = metainfo.first;
    auto &buffer_len = metainfo.second;
    len = buffer_len;
    if (err_code != serialize::errc{}) {
      return err_code;
    }

    data_len_ += buffer_len;

    switch (size_type_) {
      case 0:
        err_code = deserialize_many<1, UINT64_MAX, true>(t, args...);
        break;
      case 1:
        err_code = deserialize_many<2, UINT64_MAX, true>(t, args...);
        break;
      case 2:
        err_code = deserialize_many<4, UINT64_MAX, true>(t, args...);
        break;
      case 3:
        err_code = deserialize_many<8, UINT64_MAX, true>(t, args...);
        break;
      default:
        unreachable();
    }

    if (err_code != errc::ok) {
      return err_code;
    }
    constexpr std::size_t sz = compatible_version_number<Type>.size();
    err_code = deserialize_compatibles<T, Args...>(
        t, std::make_index_sequence<sz>{}, args...);

    return err_code;
  }

  template <
      typename T, typename... Args, typename Type = get_args_type<T, Args...>,
      std::enable_if_t<
          !check_if_compatible_element_exist<decltype(get_types<Type>())>(),
          int> = 0>
  serialize::errc deserialize_with_len(std::size_t &len, T &t, Args &...args) {
    auto &&metainfo = deserialize_metainfo<Type>();
    auto &err_code = metainfo.first;
    auto &buffer_len = metainfo.second;
    len = buffer_len;
    if (err_code != serialize::errc{}) {
      return err_code;
    }

    switch (size_type_) {
      case 0:
        err_code = deserialize_many<1, UINT64_MAX, true>(t, args...);
        break;
      case 1:
        err_code = deserialize_many<2, UINT64_MAX, true>(t, args...);
        break;
      case 2:
        err_code = deserialize_many<4, UINT64_MAX, true>(t, args...);
        break;
      case 3:
        err_code = deserialize_many<8, UINT64_MAX, true>(t, args...);
        break;

      default:
        unreachable();
    }

    return err_code;
  }

  //   template <typename U, size_t I>
  //   serialize::errc get_field(
  //       std::tuple_element_t<I, decltype(get_types<U>())> &field) {
  //     using T = remove_cvref_t<U>;
  //     using Type = get_args_type<T>;

  //     constexpr bool has_compatible =
  //         check_if_compatible_element_exist<decltype(get_types<Type>())>();
  //     if constexpr (has_compatible) {
  //       data_len_ = reader_.tellg();
  //     }

  //     auto &&[err_code, buffer_len] = deserialize_metainfo<T>();
  //     if (err_code != serialize::errc{}) {
  //       return err_code;
  //     }
  //     if constexpr (has_compatible) {
  //       data_len_ += buffer_len;
  //     }
  //     switch (size_type_) {
  //       case 0:
  //         err_code = get_field_impl<1, UINT64_MAX, U, I>(field);
  //         break;
  // #ifdef STRUCT_PACK_OPTIMIZE
  //       case 1:
  //         err_code = get_field_impl<2, UINT64_MAX, U, I>(field);
  //         break;
  //       case 2:
  //         err_code = get_field_impl<4, UINT64_MAX, U, I>(field);
  //         break;
  //       case 3:
  //         err_code = get_field_impl<8, UINT64_MAX, U, I>(field);
  //         break;
  // #else
  //       case 1:
  //       case 2:
  //       case 3:
  //         err_code = get_field_impl<2, UINT64_MAX, U, I>(field);
  //         break;
  // #endif
  //       default:
  //         unreachable();
  //     }
  //     if constexpr (has_compatible) {
  //       if (err_code != errc::ok) {
  //         return err_code;
  //       }
  //       constexpr std::size_t sz = compatible_version_number<Type>.size();
  //       err_code = deserialize_compatible_fields<U, I>(
  //           field, std::make_index_sequence<sz>{});
  //     }
  //     return err_code;
  //   }

 private:
  template <size_t size_type, size_t I>
  serialize::errc deserialize_compatibles_helper() {
    return serialize::errc::ok;
  }
  template <size_t size_type, size_t I, typename T, typename... Args>
  serialize::errc deserialize_compatibles_helper(T &t, Args &...args) {
    using Type = get_args_type<T, Args...>;
    auto err_code =
        deserialize_many<size_type, compatible_version_number<Type>[I], true>(
            t, args...);
    if (err_code != serialize::errc::ok) {
      return err_code;
    }
    return deserialize_compatibles_helper<size_type, I + 1>(args...);
  }
  template <typename T, typename... Args, size_t... I>
  serialize::errc deserialize_compatibles(T &t, std::index_sequence<I...>,
                                          Args &...args) {
    serialize::errc err_code;
    switch (size_type_) {
      case 0:
        err_code = deserialize_compatibles_helper<1, I...>(t, args...);
        break;
      case 1:
        err_code = deserialize_compatibles_helper<2, I...>(t, args...);
        break;
      case 2:
        err_code = deserialize_compatibles_helper<4, I...>(t, args...);
        break;
      case 3:
        err_code = deserialize_compatibles_helper<8, I...>(t, args...);
        break;
      default:
        unreachable();
    }
    if (size_type_ ==
        UCHAR_MAX) {  // reuse size_type_ as a tag that the buffer miss some
                      // compatible field, whic is legal.
      err_code = {};
    }
    return err_code;
  }

  std::pair<serialize::errc, std::uint64_t> deserialize_compatible(
      unsigned compatible_sz_len) {
    // constexpr std::size_t sz[] = {0, 2, 4, 8};
    // auto len_sz = sz[compatible_sz_len];
    uint64_t data_len64;
    uint32_t data_len32;
    uint16_t data_len16;
    // bool result;
    switch (compatible_sz_len) {
      case 1:
        if SER_LIKELY (read_wrapper<2>(reader_, (char *)&data_len16)) {
          return {errc{}, data_len16};
        }
        break;
      case 2:
        if SER_LIKELY (read_wrapper<4>(reader_, (char *)&data_len32)) {
          return {errc{}, data_len32};
        }
        break;
      case 3:
        if SER_LIKELY (read_wrapper<8>(reader_, (char *)&data_len64)) {
          return {errc{}, data_len64};
        }
        break;
      default:
        unreachable();
    }
    return {errc::no_buffer_space, 0};
  }

  template <typename T, typename R = Reader,
            std::enable_if_t<view_reader_t<R>, int> = 0>
  serialize::errc deserialize_type_literal() {
    constexpr auto literal = serialize::get_type_literal<T>();
    const char *buffer = reader_.read_view(literal.size() + 1);
    if SER_UNLIKELY (!buffer) {
      return serialize::errc::no_buffer_space;
    }
    if SER_UNLIKELY (memcmp(buffer, literal.data(), literal.size() + 1)) {
      return serialize::errc::hash_conflict;
    }
    return serialize::errc{};
  }
  template <typename T, typename R = Reader,
            std::enable_if_t<!view_reader_t<R>, int> = 0>
  serialize::errc deserialize_type_literal() {
    constexpr auto literal = serialize::get_type_literal<T>();
    char buffer[literal.size() + 1];
    if SER_UNLIKELY (!read_bytes_array(reader_, buffer, literal.size() + 1)) {
      return serialize::errc::no_buffer_space;
    }
    if SER_UNLIKELY (memcmp(buffer, literal.data(), literal.size() + 1)) {
      return serialize::errc::hash_conflict;
    }
    return serialize::errc{};
  }

  template <typename T>
  std::pair<serialize::errc, std::uint64_t> deserialize_metainfo_helper(
      unsigned char &metainfo) {
    std::pair<serialize::errc, std::uint64_t> ret;
    auto compatible_sz_len = metainfo & 0b11;
    if (compatible_sz_len) {
      ret = deserialize_compatible(compatible_sz_len);
      if SER_UNLIKELY (ret.first != errc{}) {
        return ret;
      }
    }
    auto has_type_literal = metainfo & 0b100;
    if (has_type_literal) {
      auto ec = deserialize_type_literal<T>();
      if SER_UNLIKELY (ec != errc{}) {
        return {ec, 0};
      }
    }
    size_type_ = (metainfo & 0b11000) >> 3;
    return ret;
  }

  template <class T, std::enable_if_t<check_if_disable_hash_head<conf, T>() &&
                                          check_if_has_container<T>(),
                                      int> = 0>
  std::pair<serialize::errc, std::uint64_t> deserialize_metainfo() {
    unsigned char metainfo;
    if SER_UNLIKELY (!read_wrapper<sizeof(unsigned char)>(reader_,
                                                          (char *)&metainfo)) {
      return {serialize::errc::no_buffer_space, 0};
    }
    size_type_ = (metainfo & 0b11000) >> 3;
    return {};
  }
  template <class T, std::enable_if_t<check_if_disable_hash_head<conf, T>() &&
                                          !check_if_has_container<T>(),
                                      int> = 0>
  std::pair<serialize::errc, std::uint64_t> deserialize_metainfo() {
    size_type_ = 0;
    return {};
  }
  template <class T, std::enable_if_t<!check_if_disable_hash_head<conf, T>() &&
                                          is_MD5_reader_wrapper<Reader>,
                                      int> = 0>
  std::pair<serialize::errc, std::uint64_t> deserialize_metainfo() {
    uint32_t current_types_code;

    reader_.read_head((char *)&current_types_code);
    if SER_LIKELY (current_types_code % 2 == 0)  // unexist metainfo
    {
      size_type_ = 0;
      return {};
    }
    unsigned char metainfo;
    return deserialize_metainfo_helper<T>(metainfo);
  }
  template <class T, std::enable_if_t<!check_if_disable_hash_head<conf, T>() &&
                                          !is_MD5_reader_wrapper<Reader>,
                                      int> = 0>
  std::pair<serialize::errc, std::uint64_t> deserialize_metainfo() {
    uint32_t current_types_code;

    if SER_UNLIKELY (!read_wrapper<sizeof(uint32_t)>(
                         reader_, (char *)&current_types_code)) {
      return {serialize::errc::no_buffer_space, 0};
    }
    constexpr uint32_t types_code = get_types_code<T>();
    if SER_UNLIKELY ((current_types_code / 2) != (types_code / 2)) {
      std::cout << __FILE__ << ": " << __LINE__ << ": " << current_types_code
                << "  " << types_code << std::endl;
      return {serialize::errc::invalid_buffer, 0};
    }
    if SER_LIKELY (current_types_code % 2 == 0)  // unexist metainfo
    {
      size_type_ = 0;
      return {};
    }
    unsigned char metainfo;
    return deserialize_metainfo_helper<T>(metainfo);
  }

  template <size_t size_type, uint64_t version, bool NotSkip>
  constexpr serialize::errc deserialize_many() {
    return {};
  }
  template <size_t size_type, uint64_t version, bool NotSkip, typename First,
            typename... Args>
  constexpr serialize::errc deserialize_many(First &&first_item,
                                             Args &&...items) {
    auto code = deserialize_one<size_type, version, NotSkip>(first_item);
    if SER_UNLIKELY (code != serialize::errc{}) {
      return code;
    }
    return deserialize_many<size_type, version, NotSkip>(items...);
  }

  constexpr serialize::errc ignore_padding(std::size_t sz) {
    if (sz > 0) {
      return reader_.ignore(sz) ? errc{} : errc::no_buffer_space;
    } else {
      return errc{};
    }
  }

  template <size_t size_type, uint64_t version, bool NotSkip, type_id id,
            typename T>
  constexpr serialize::errc inline deserialize_one_helper(
      T &item,
      std::enable_if_t<
          version == UINT64_MAX &&
              (id == type_id::compatible_t || id == type_id::monostate_t ||
               std::is_same<void, remove_cvref_t<decltype(item)>>::value),
          int> = 0) {
    // do nothing
    return serialize::errc{};
  }

  template <size_t size_type, uint64_t version, bool NotSkip, type_id id,
            typename T>
  constexpr serialize::errc inline deserialize_one_helper(
      T &item,
      std::enable_if_t<
          version == UINT64_MAX &&
              (std::is_fundamental<remove_cvref_t<decltype(item)>>::value ||
               std::is_enum<remove_cvref_t<decltype(item)>>::value ||
               id == type_id::int128_t || id == type_id::uint128_t),
          int> = 0) {
    // do nothing
    return serialize::errc{};
  }

  template <size_t size_type, uint64_t version, bool NotSkip, typename T>
  constexpr serialize::errc inline deserialize_one(T &item) {
    using type = remove_cvref_t<decltype(item)>;
    static_assert(!std::is_pointer<type>::value, "");
    constexpr auto id = get_type_id<type>();
    return deserialize_one_helper<size_type, version, NotSkip, id>(item);
  }

  //   template <size_t size_type, uint64_t version, bool NotSkip, typename T,
  //             type_id id = get_type_id<remove_cvref_t<T>>()>
  //   constexpr struct_pack::errc inline deserialize_one(T &item) {}

  //   template <size_t size_type, uint64_t version, bool NotSkip, typename T>
  //   constexpr struct_pack::errc inline deserialize_one(T &item) {
  //     struct_pack::errc code{};
  //     using type = remove_cvref_t<decltype(item)>;
  //     static_assert(!std::is_pointer_v<type>);
  //     constexpr auto id = get_type_id<type>();
  //     if constexpr (is_trivial_view_v<type>) {
  //       static_assert(view_reader_t<Reader>,
  //                     "The Reader isn't a view_reader, can't deserialize "
  //                     "a trivial_view<T>");
  //       static_assert(
  //           is_little_endian_copyable<sizeof(typename type::value_type)>,
  //           "get a trivial view with byte width > 1 in big-endian system is "
  //           "not allowed.");
  //       const char *view = reader_.read_view(sizeof(typename T::value_type));
  //       if SP_LIKELY (view != nullptr) {
  //         item = *reinterpret_cast<const typename T::value_type *>(view);
  //         code = errc::ok;
  //       } else {
  //         code = errc::no_buffer_space;
  //       }
  //     } else if constexpr (version == UINT64_MAX) {
  //       if constexpr (id == type_id::compatible_t) {
  //         // do nothing
  //       } else if constexpr (std::is_same_v<type, std::monostate>) {
  //         // do nothing
  //       } else if constexpr (std::is_fundamental_v<type> ||
  //                            std::is_enum_v<type> || id == type_id::int128_t
  //                            || id == type_id::uint128_t) {
  //         if constexpr (NotSkip) {
  //           if SP_UNLIKELY (!read_wrapper<sizeof(type)>(reader_, (char
  //           *)&item)) {
  //             return struct_pack::errc::no_buffer_space;
  //           }
  //         } else {
  //           return reader_.ignore(sizeof(type)) ? errc{} :
  //           errc::no_buffer_space;
  //         }
  //       } else if constexpr (id == type_id::bitset_t) {
  //         if constexpr (NotSkip) {
  //           if SP_UNLIKELY (!read_bytes_array(reader_, (char *)&item,
  //                                             sizeof(type))) {
  //             return struct_pack::errc::no_buffer_space;
  //           }
  //         } else {
  //           return reader_.ignore(sizeof(type)) ? errc{} :
  //           errc::no_buffer_space;
  //         }
  //       } else if constexpr (unique_ptr<type>) {
  //         bool has_value{};
  //         if SP_UNLIKELY (!read_wrapper<sizeof(bool)>(reader_,
  //                                                     (char *)&has_value)) {
  //           return struct_pack::errc::no_buffer_space;
  //         }
  //         if (!has_value) {
  //           return {};
  //         }
  //         if constexpr (is_base_class<typename type::element_type>) {
  //           uint32_t id;
  //           read_wrapper<sizeof(id)>(reader_, (char *)&id);
  //           bool ok;
  //           auto index = search_type_by_md5<typename type::element_type>(id,
  //           ok); if SP_UNLIKELY (!ok) {
  //             return errc::invalid_buffer;
  //           } else {
  //             return template_switch<deserialize_one_derived_class_helper<
  //                 derived_class_set_t<typename type::element_type>,
  //                 std::integral_constant<std::size_t, size_type>,
  //                 std::integral_constant<std::uint64_t, version>,
  //                 std::integral_constant<std::uint64_t, NotSkip>>>(index,
  //                 this,
  //                                                                  item);
  //           }
  //         } else {
  //           item = std::make_unique<typename type::element_type>();
  //           deserialize_one<size_type, version, NotSkip>(*item);
  //         }
  //       } else if constexpr (detail::varint_t<type>) {
  //         code = detail::deserialize_varint<NotSkip>(reader_, item);
  //       } else if constexpr (id == type_id::array_t) {
  //         if constexpr (is_trivial_serializable<type>::value &&
  //                       is_little_endian_copyable<sizeof(item[0])>) {
  //           if constexpr (NotSkip) {
  //             if SP_UNLIKELY (!read_bytes_array(reader_, (char *)&item,
  //                                               sizeof(item))) {
  //               return struct_pack::errc::no_buffer_space;
  //             }
  //           } else {
  //             return reader_.ignore(sizeof(type)) ? errc{}
  //                                                 : errc::no_buffer_space;
  //           }
  //         } else {
  //           for (auto &i : item) {
  //             code = deserialize_one<size_type, version, NotSkip>(i);
  //             if SP_UNLIKELY (code != struct_pack::errc{}) {
  //               return code;
  //             }
  //           }
  //         }
  //       } else if constexpr (container<type>) {
  //         uint16_t size16;
  //         uint32_t size32;
  //         uint64_t size64;
  //         bool result;
  //         if constexpr (size_type == 1) {
  //           uint8_t size8;
  //           if SP_UNLIKELY (!read_wrapper<size_type>(reader_, (char
  //           *)&size8)) {
  //             return struct_pack::errc::no_buffer_space;
  //           }
  //           size64 = size8;
  //         }
  // #ifdef STRUCT_PACK_OPTIMIZE
  //         else if constexpr (size_type == 2) {
  //           if SP_UNLIKELY (!read_wrapper<size_type>(reader_, (char
  //           *)&size16)) {
  //             return struct_pack::errc::no_buffer_space;
  //           }
  //           size64 = size16;
  //         } else if constexpr (size_type == 4) {
  //           if SP_UNLIKELY (!read_wrapper<size_type>(reader_, (char
  //           *)&size32)) {
  //             return struct_pack::errc::no_buffer_space;
  //           }
  //           size64 = size32;
  //         } else if constexpr (size_type == 8) {
  //           if SP_UNLIKELY (!read_wrapper<size_type>(reader_, (char
  //           *)&size64)) {
  //             return struct_pack::errc::no_buffer_space;
  //           }
  //         } else {
  //           static_assert(!sizeof(T), "illegal size_type");
  //         }
  // #else
  //         else {
  //           switch (size_type_) {
  //             case 1:
  //               if SP_UNLIKELY (!read_wrapper<2>(reader_, (char *)&size16)) {
  //                 return struct_pack::errc::no_buffer_space;
  //               }
  //               size64 = size16;
  //               break;
  //             case 2:
  //               if SP_UNLIKELY (!read_wrapper<4>(reader_, (char *)&size32)) {
  //                 return struct_pack::errc::no_buffer_space;
  //               }
  //               size64 = size32;
  //               break;
  //             case 3:
  //               if SP_UNLIKELY (!read_wrapper<8>(reader_, (char *)&size64)) {
  //                 return struct_pack::errc::no_buffer_space;
  //               }
  //               break;
  //             default:
  //               unreachable();
  //           }
  //         }
  // #endif
  //         if SP_UNLIKELY (size64 == 0) {
  //           return {};
  //         }
  //         if constexpr (map_container<type>) {
  //           std::pair<typename type::key_type, typename type::mapped_type>
  //               value{};
  //           if constexpr (is_trivial_serializable<decltype(value)>::value &&
  //                         !NotSkip) {
  //             return reader_.ignore(size64 * sizeof(value))
  //                        ? errc{}
  //                        : errc::no_buffer_space;
  //           } else {
  //             for (uint64_t i = 0; i < size64; ++i) {
  //               code = deserialize_one<size_type, version, NotSkip>(value);
  //               if SP_UNLIKELY (code != struct_pack::errc{}) {
  //                 return code;
  //               }
  //               if constexpr (NotSkip) {
  //                 item.emplace(std::move(value));
  //                 // TODO: mapped_type can deserialize without be moved
  //               }
  //             }
  //           }
  //         } else if constexpr (set_container<type>) {
  //           typename type::value_type value{};
  //           if constexpr (is_trivial_serializable<decltype(value)>::value &&
  //                         !NotSkip) {
  //             return reader_.ignore(size64 * sizeof(value))
  //                        ? errc{}
  //                        : errc::no_buffer_space;
  //           } else {
  //             for (uint64_t i = 0; i < size64; ++i) {
  //               code = deserialize_one<size_type, version, NotSkip>(value);
  //               if SP_UNLIKELY (code != struct_pack::errc{}) {
  //                 return code;
  //               }
  //               if constexpr (NotSkip) {
  //                 item.emplace(std::move(value));
  //                 // TODO: mapped_type can deserialize without be moved
  //               }
  //             }
  //           }
  //         } else {
  //           using value_type = typename type::value_type;
  //           if constexpr (trivially_copyable_container<type>) {
  //             uint64_t mem_sz = size64 * sizeof(value_type);
  //             if constexpr (NotSkip) {
  //               if constexpr (string_view<type> || dynamic_span<type>) {
  //                 static_assert(
  //                     view_reader_t<Reader>,
  //                     "The Reader isn't a view_reader, can't deserialize "
  //                     "a string_view/span");
  //                 static_assert(is_little_endian_copyable<sizeof(value_type)>,
  //                               "zero-copy in big endian is limit.");
  //                 const char *view = reader_.read_view(mem_sz);
  //                 if SP_UNLIKELY (view == nullptr) {
  //                   return struct_pack::errc::no_buffer_space;
  //                 }
  //                 item = {(value_type *)(view), (std::size_t)size64};
  //               } else if constexpr (is_little_endian_copyable<sizeof(
  //                                        value_type)>) {
  //                 if SP_UNLIKELY (mem_sz >= PTRDIFF_MAX)
  //                   unreachable();
  //                 else {
  //                   item.resize(size64);
  //                   if SP_UNLIKELY (!read_bytes_array(
  //                                       reader_, (char *)item.data(),
  //                                       size64 * sizeof(value_type))) {
  //                     return struct_pack::errc::no_buffer_space;
  //                   }
  //                 }
  //               } else {
  //                 item.resize(size64);
  //                 for (auto &i : item) {
  //                   code = deserialize_one<size_type, version, NotSkip>(i);
  //                   if SP_UNLIKELY (code != struct_pack::errc{}) {
  //                     return code;
  //                   }
  //                 }
  //               }
  //             } else {
  //               return reader_.ignore(mem_sz) ? errc{} :
  //               errc::no_buffer_space;
  //             }
  //           } else {
  //             if constexpr (NotSkip) {
  //               if constexpr (dynamic_span<type>) {
  //                 static_assert(!dynamic_span<type>,
  //                               "It's illegal to deserialize a span<T> which
  //                               T " "is a non-trival-serializable type.");
  //               } else {
  //                 item.resize(size64);
  //                 for (auto &i : item) {
  //                   code = deserialize_one<size_type, version, NotSkip>(i);
  //                   if SP_UNLIKELY (code != struct_pack::errc{}) {
  //                     return code;
  //                   }
  //                 }
  //               }
  //             } else {
  //               value_type useless;
  //               for (size_t i = 0; i < size64; ++i) {
  //                 code = deserialize_one<size_type, version,
  //                 NotSkip>(useless); if SP_UNLIKELY (code !=
  //                 struct_pack::errc{}) {
  //                   return code;
  //                 }
  //               }
  //             }
  //           }
  //         }
  //       } else if constexpr (container_adapter<type>) {
  //         static_assert(!sizeof(type),
  //                       "the container adapter type is not supported");
  //       } else if constexpr (!pair<type> && tuple<type> &&
  //                            !is_trivial_tuple<type>) {
  //         std::apply(
  //             [&](auto &&...items) CONSTEXPR_INLINE_LAMBDA {
  //               code = deserialize_many<size_type, version,
  //               NotSkip>(items...);
  //             },
  //             item);
  //       } else if constexpr (optional<type> || expected<type>) {
  //         bool has_value{};
  //         if SP_UNLIKELY (!read_wrapper<sizeof(bool)>(reader_,
  //                                                     (char *)&has_value)) {
  //           return struct_pack::errc::no_buffer_space;
  //         }
  //         if SP_UNLIKELY (!has_value) {
  //           if constexpr (expected<type>) {
  //             item = typename type::unexpected_type{typename
  //             type::error_type{}}; deserialize_one<size_type, version,
  //             NotSkip>(item.error());
  //           } else {
  //             return {};
  //           }
  //         } else {
  //           if constexpr (expected<type>) {
  //             if constexpr (!std::is_same_v<typename type::value_type, void>)
  //               deserialize_one<size_type, version, NotSkip>(item.value());
  //           } else {
  //             item = type{std::in_place_t{}};
  //             deserialize_one<size_type, version, NotSkip>(*item);
  //           }
  //         }
  //       } else if constexpr (is_variant_v<type>) {
  //         uint8_t index{};
  //         if SP_UNLIKELY (!read_wrapper<sizeof(index)>(reader_, (char
  //         *)&index)) {
  //           return struct_pack::errc::no_buffer_space;
  //         }
  //         if SP_UNLIKELY (index >= std::variant_size_v<type>) {
  //           return struct_pack::errc::invalid_buffer;
  //         } else {
  //           template_switch<variant_construct_helper<
  //               std::integral_constant<std::size_t, size_type>,
  //               std::integral_constant<std::uint64_t, version>,
  //               std::integral_constant<bool, NotSkip>>>(index, *this, item);
  //         }
  //       } else if constexpr (std::is_class_v<type>) {
  //         if constexpr (!pair<type> && !is_trivial_tuple<type>)
  //           if constexpr (!user_defined_refl<type>)
  //             static_assert(
  //                 std::is_aggregate_v<remove_cvref_t<type>>,
  //                 "struct_pack only support aggregated type, or you should "
  //                 "add macro STRUCT_PACK_REFL(Type,field1,field2...)");
  //         if constexpr (is_trivial_serializable<type>::value &&
  //                       is_little_endian_copyable<sizeof(type)>) {
  //           if constexpr (NotSkip) {
  //             if SP_UNLIKELY (!read_wrapper<sizeof(type)>(reader_,
  //                                                         (char *)&item)) {
  //               return struct_pack::errc::no_buffer_space;
  //             }
  //           } else {
  //             return reader_.ignore(sizeof(type)) ? errc{}
  //                                                 : errc::no_buffer_space;
  //           }
  //         } else if constexpr ((is_trivial_serializable<type>::value &&
  //                               !is_little_endian_copyable<sizeof(type)>) ||
  //                              is_trivial_serializable<type, true>::value) {
  //           visit_members(item, [&](auto &&...items) CONSTEXPR_INLINE_LAMBDA
  //           {
  //             int i = 1;
  //             auto f = [&](auto &&item) {
  //               code = deserialize_one<size_type, version, NotSkip>(item);
  //               if SP_LIKELY (code == errc::ok) {
  //                 code = ignore_padding(align::padding_size<type>[i++]);
  //               }
  //               return code == errc::ok;
  //             };
  //             [[maybe_unused]] bool op = (f(items) && ...);
  //           });
  //         } else {
  //           visit_members(item, [&](auto &&...items) CONSTEXPR_INLINE_LAMBDA
  //           {
  //             code = deserialize_many<size_type, version, NotSkip>(items...);
  //           });
  //         }
  //       } else {
  //         static_assert(!sizeof(type), "the type is not supported yet");
  //       }
  //     } else if constexpr (exist_compatible_member<type, version>) {
  //       if constexpr (id == type_id::compatible_t) {
  //         if constexpr (version == type::version_number) {
  //           auto pos = reader_.tellg();
  //           if ((std::size_t)pos >= data_len_) {
  //             if (std::is_unsigned_v<decltype(pos)> || pos >= 0) {
  //               size_type_ = UCHAR_MAX;  // Just notice that this is not a
  //               real
  //                                        // error, this is a flag for exit.
  //             }
  //             return struct_pack::errc::no_buffer_space;
  //           }
  //           bool has_value{};
  //           if SP_UNLIKELY (!read_wrapper<sizeof(bool)>(reader_,
  //                                                       (char *)&has_value))
  //                                                       {
  //             return struct_pack::errc::no_buffer_space;
  //           }
  //           if (!has_value) {
  //             return code;
  //           } else {
  //             item = type{std::in_place_t{}};
  //             deserialize_one<size_type, UINT64_MAX, NotSkip>(*item);
  //           }
  //         }
  //       } else if constexpr (unique_ptr<type>) {
  //         if (item == nullptr) {
  //           return {};
  //         }
  //         if constexpr (is_base_class<typename type::element_type>) {
  //           uint32_t id = item->get_struct_pack_id();
  //           bool ok;
  //           auto index = search_type_by_md5<typename type::element_type>(id,
  //           ok); assert(ok); return
  //           template_switch<deserialize_one_derived_class_helper<
  //               derived_class_set_t<typename type::element_type>,
  //               std::integral_constant<std::size_t, size_type>,
  //               std::integral_constant<std::uint64_t, version>,
  //               std::integral_constant<std::uint64_t, NotSkip>>>(index, this,
  //                                                                item);
  //         } else {
  //           deserialize_one<size_type, version, NotSkip>(*item);
  //         }
  //       } else if constexpr (id == type_id::array_t) {
  //         for (auto &i : item) {
  //           code = deserialize_one<size_type, version, NotSkip>(i);
  //           if SP_UNLIKELY (code != struct_pack::errc{}) {
  //             return code;
  //           }
  //         }
  //       } else if constexpr (container<type>) {
  //         if constexpr (id == type_id::set_container_t) {
  //           // TODO: support it.
  //           static_assert(!sizeof(type),
  //                         "we don't support compatible field in set now.");
  //         } else if constexpr (id == type_id::map_container_t) {
  //           static_assert(
  //               !exist_compatible_member<typename type::key_type>,
  //               "we don't support compatible field in map's key_type now.");
  //           if constexpr (NotSkip) {
  //             for (auto &e : item) {
  //               code = deserialize_one<size_type, version,
  //               NotSkip>(e.second); if SP_UNLIKELY (code !=
  //               struct_pack::errc{}) {
  //                 return code;
  //               }
  //             }
  //           } else {
  //             // TODO: support it.
  //             static_assert(
  //                 !sizeof(type),
  //                 "we don't support skip compatible field in container
  //                 now.");
  //           }
  //           // how to deserialize it quickly?
  //         } else {
  //           if constexpr (NotSkip) {
  //             for (auto &i : item) {
  //               code = deserialize_one<size_type, version, NotSkip>(i);
  //               if SP_UNLIKELY (code != struct_pack::errc{}) {
  //                 return code;
  //               }
  //             }
  //           } else {
  //             // TODO: support it.
  //             static_assert(
  //                 !sizeof(type),
  //                 "we don't support skip compatible field in container
  //                 now.");
  //           }
  //         }
  //       } else if constexpr (!pair<type> && tuple<type> &&
  //                            !is_trivial_tuple<type>) {
  //         std::apply(
  //             [&](auto &&...items) CONSTEXPR_INLINE_LAMBDA {
  //               code = deserialize_many<size_type, version,
  //               NotSkip>(items...);
  //             },
  //             item);
  //       } else if constexpr (optional<type> || expected<type>) {
  //         bool has_value = item.has_value();
  //         if (!has_value) {
  //           if constexpr (expected<type>) {
  //             deserialize_one<size_type, version, NotSkip>(item.error());
  //           }
  //         } else {
  //           if constexpr (expected<type>) {
  //             if constexpr (!std::is_same_v<typename type::value_type, void>)
  //               deserialize_one<size_type, version, NotSkip>(item.value());
  //           } else {
  //             deserialize_one<size_type, version, NotSkip>(item.value());
  //           }
  //         }
  //       } else if constexpr (is_variant_v<type>) {
  //         std::visit(
  //             [this](auto &item) {
  //               deserialize_one<size_type, version, NotSkip>(item);
  //             },
  //             item);
  //       } else if constexpr (std::is_class_v<type>) {
  //         visit_members(item, [&](auto &&...items) CONSTEXPR_INLINE_LAMBDA {
  //           code = deserialize_many<size_type, version, NotSkip>(items...);
  //         });
  //       }
  //     }
  //     return code;
  //   }

 public:
  std::size_t data_len_;

 private:
  Reader &reader_;
  unsigned char size_type_;
};

}  // namespace details
}  // namespace serialize
