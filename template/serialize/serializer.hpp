/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-28 15:37:17
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-11-29 02:36:39
 */

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "endian_wapper.hpp"
#include "type_calculate.hpp"
#include "type_id.hpp"

#ifndef SERIALIZE_DETAILS_SERIALIZE_HPP_
#define SERIALIZE_DETAILS_SERIALIZE_HPP_
namespace serialize {
namespace details {

struct serialize_buffer_size {
 private:
  std::size_t len_;
  unsigned char metainfo_;

 public:
  constexpr serialize_buffer_size() : len_(0), metainfo_(0) {}
  constexpr std::size_t size() const { return len_; }
  constexpr unsigned char metainfo() const { return metainfo_; }
  constexpr operator std::size_t() const { return len_; }

  //   template <uint64_t conf, typename... Args>
  //   friend STRUCT_PACK_INLINE constexpr serialize_buffer_size
  //   struct_pack::detail::get_serialize_runtime_info(const Args&... args);
};

template <typename T, typename = void>
struct writer_t_impl : std::false_type {};

template <typename T>
struct writer_t_impl<T, std::void_t<decltype(std::declval<T>().write(
                            (const char *)nullptr, std::size_t{}))>>
    : std::true_type {};

template <typename T>
constexpr bool writer_t = writer_t_impl<T>::value;

template <typename T, typename = void>
struct reader_t_impl : std::false_type {};

template <typename T>
struct reader_t_impl<
    T, std::void_t<decltype(std::declval<T>().read((char *)nullptr,
                                                   std::size_t{})),
                   decltype(std::declval<T>().ignore(std::size_t{})),
                   decltype(std::declval<T>().tellg())>> : std::true_type {};

template <typename T>
constexpr bool reader_t = reader_t_impl<T>::value;

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
}  // namespace details
}  // namespace serialize

#endif