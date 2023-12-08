/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-28 15:37:17
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-08 15:53:04
 */

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "calculate_size.hpp"
#include "endian_wapper.hpp"
#include "if_constexpr.hpp"
#include "type_calculate.hpp"
#include "type_id.hpp"
#ifndef SERIALIZE_DETAILS_SERIALIZE_HPP_
#define SERIALIZE_DETAILS_SERIALIZE_HPP_
namespace serialize {
namespace details {

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

  template <typename DerivedClasses, typename size_type, typename version>
  friend struct serialize_one_derived_class_helper;

  template <std::size_t size_type, typename T, std::size_t... I,
            typename... Args>
  void serialize_expand_compatible_helper(const T &t, std::index_sequence<I...>,
                                          const Args &...args) {
    using Type = get_args_type<T, Args...>;
    static_cast<void>(std::initializer_list<int>{
        (serialize_many<size_type, compatible_version_number<Type>[I]>(t,
                                                                       args...),
         0)...});
  }
  template <uint64_t conf, std::size_t size_type, typename T, typename... Args>
  inline void serialize(const T &t, const Args &...args) {
    serialize_metainfo<conf, size_type == 1, T, Args...>();
    serialize_many<size_type, UINT64_MAX>(t, args...);
    // using Type = get_args_type<T, Args...>;
    // utils::if_<serialize_static_config<Type>::has_compatible>([&]() {
    //   constexpr std::size_t sz = compatible_version_number<Type>.size();
    //   return serialize_expand_compatible_helper<size_type, T, Args...>(
    //       t, std::make_index_sequence<sz>{}, args...);
    // });
  }

 private:
  template <typename T, typename... Args,
            std::enable_if_t<sizeof...(Args) == 0, int> = 0>
  static constexpr uint32_t inline calculate_raw_hash() {
    return get_types_code<remove_cvref_t<T>>();
  }
  template <typename T, typename... Args,
            std::enable_if_t<sizeof...(Args) != 0, int> = 0>
  static constexpr uint32_t calculate_raw_hash() {
    return get_types_code<
        std::tuple<remove_cvref_t<T>, remove_cvref_t<Args>...>>();
  }

  template <uint64_t conf, typename T, typename... Args,
            std::enable_if_t<check_has_metainfo<conf, T>(), int> = 0>
  static constexpr uint32_t inline calculate_hash_head() {
    constexpr uint32_t raw_types_code = calculate_raw_hash<T, Args...>();
    return raw_types_code + 1;
  }
  template <uint64_t conf, typename T, typename... Args,
            std::enable_if_t<!check_has_metainfo<conf, T>(), int> = 0>
  static constexpr uint32_t inline calculate_hash_head() {
    constexpr uint32_t raw_types_code = calculate_raw_hash<T, Args...>();
    return raw_types_code;
  }

  template <uint64_t conf, bool is_default_size_type, typename T,
            typename... Args>
  constexpr void inline serialize_metainfo() {
    constexpr auto hash_head = calculate_hash_head<conf, T, Args...>() |
                               (is_default_size_type ? 0 : 1);
    // if constexpr (!check_if_disable_hash_head<conf, serialize_type>()) {
    //   write_wrapper<sizeof(uint32_t)>(writer_, (char *)&hash_head);
    // }
    // if constexpr (hash_head % 2) {  // has more metainfo
    //   auto metainfo = info.metainfo();
    //   write_wrapper<sizeof(char)>(writer_, (char *)&metainfo);
    //   if constexpr (serialize_static_config<serialize_type>::has_compatible)
    //   {
    //     uint16_t len16;
    //     uint32_t len32;
    //     uint64_t len64;
    //     switch (metainfo & 0b11) {
    //       case 1:
    //         len16 = info.size();
    //         write_wrapper<2>(writer_, (char *)&len16);
    //         break;
    //       case 2:
    //         len32 = info.size();
    //         write_wrapper<4>(writer_, (char *)&len32);
    //         break;
    //       case 3:
    //         len64 = info.size();
    //         write_wrapper<8>(writer_, (char *)&len64);
    //         break;
    //       default:
    //         unreachable();
    //     }
    //   }
    //   if constexpr (check_if_add_type_literal<conf, serialize_type>()) {
    //     constexpr auto type_literal =
    //         struct_pack::get_type_literal<T, Args...>();
    //     write_bytes_array(writer_, type_literal.data(),
    //                       type_literal.size() + 1);
    //   }
    // }
  }

 private:
  template <std::size_t size_type, uint64_t version, typename First,
            typename... Args>
  constexpr void serialize_many(const First &first_item, const Args &...items) {
    serialize_one<size_type, version>(first_item);
    serialize_many<size_type, version>(items...);
  }

  template <std::size_t size_type, uint64_t version, typename T,
            std::enable_if_t<is_trivial_view_v<T>, int> = 0>
  constexpr void inline serialize_one(const T &item) {
    using type = remove_cvref_t<decltype(item)>;
    static_assert(!std::is_pointer<type>::value, "cannot serialzie a pointer");
    constexpr auto id = get_type_id<type>();
    return serialize_one<size_type, version>(item.get());
  }

  template <std::size_t size_type, uint64_t version, typename T,
            std::enable_if_t<version == UINT64_MAX, int> = 0>
  constexpr void inline serialize_one(const T &item) {
    using type = remove_cvref_t<decltype(item)>;
    static_assert(!std::is_pointer<type>::value, "cannot serialzie a pointer");
    constexpr auto id = get_type_id<type>();
    return serialize_one<size_type, version, id, type>(item.get());
  }

  template <
      std::size_t size_type, uint64_t version, type_id id, typename Type,
      typename T,
      std::enable_if_t<version == UINT64_MAX && (id == type_id::compatible_t ||
                                                 id == type_id::monostate_t),
                       int> = 0>
  constexpr void inline serialize_one(const T &) {
    // do nothing
    return;
  }

  template <
      std::size_t size_type, uint64_t version, type_id id, typename Type,
      typename T,
      std::enable_if_t<
          version == UINT64_MAX &&
              (std::is_fundamental<Type>::value || std::is_enum<Type>::value ||
               id == type_id::int128_t || id == type_id::uint128_t),
          int> = 0>
  constexpr void inline serialize_one(const T &item) {
    write_wrapper<sizeof(item)>(writer_, (char *)&item);
  }

  template <std::size_t size_type, uint64_t version, type_id id, typename Type,
            typename T,
            std::enable_if_t<version == UINT64_MAX && (id == type_id::bitset_t),
                             int> = 0>
  constexpr void inline serialize_one(const T &item) {
    write_bytes_array(writer_, (char *)&item, sizeof(item));
  }

  template <
      std::size_t size_type, uint64_t version, type_id id, typename Type,
      typename T,
      std::enable_if_t<version == UINT64_MAX && (unique_ptr<Type>), int> = 0>
  constexpr void inline serialize_one(const T &) {
    // bool has_value = (item != nullptr);
    // write_wrapper<sizeof(char)>(writer_, (char *)&has_value);
    // if (has_value) {
    //   if constexpr (is_base_class<typename type::element_type>) {
    //     bool is_ok;
    //     uint32_t id = item->get_struct_pack_id();
    //     auto index = search_type_by_md5<typename type::element_type>(
    //         item->get_struct_pack_id(), is_ok);
    //     assert(is_ok);
    //     write_wrapper<sizeof(uint32_t)>(writer_, (char *)&id);
    //     template_switch<serialize_one_derived_class_helper<
    //         derived_class_set_t<typename type::element_type>,
    //         std::integral_constant<std::size_t, size_type>,
    //         std::integral_constant<std::uint64_t, version>>>(index, this,
    //                                                          item.get());
    //   } else {
    //     serialize_one<size_type, version>(*item);
    //   }
  }

  template <typename T>
  friend constexpr serialize_buffer_size get_needed_size(const T &t);
  writer &writer_;
  const serialize_buffer_size &info_;
};

template <uint64_t conf = sp_config::DEFAULT, typename Writer, typename... Args>
void serialize_to(Writer &writer, const serialize_buffer_size &info,
                  const Args &...args) {
  static_assert(sizeof...(args) > 0, "");
  details::Serializer<Writer, details::get_args_type<Args...>> o(writer, info);
  switch ((info.metainfo() & 0b11000) >> 3) {
    case 0:
      o.template serialize<conf, 1>(args...);
      break;
    case 1:
      o.template serialize<conf, 2>(args...);
      break;
    case 2:
      o.template serialize<conf, 4>(args...);
      break;
    case 3:
      o.template serialize<conf, 8>(args...);
      break;
    default:
      details::unreachable();
      break;
  };
}

}  // namespace details
}  // namespace serialize

#endif