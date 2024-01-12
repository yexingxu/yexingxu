/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-12-27 20:32:47
 * @LastEditors: chen, hua
 * @LastEditTime: 2024-01-11 14:28:23
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "append_types/expected.hpp"
#include "append_types/optional.hpp"
#include "append_types/variant.hpp"
#include "for_each_macro.hpp"
#include "trivial_view.hpp"
#include "utils.hpp"

namespace serialize {
namespace detail {

template <typename... Args>
using get_args_type = remove_cvref_t<typename std::conditional<
    sizeof...(Args) == 1, std::tuple_element_t<0, std::tuple<Args...>>,
    std::tuple<Args...>>::type>;

// props
template <typename T, typename = void>
struct props_t_impl : std::false_type {};

template <typename T>
struct props_t_impl<T, std::void_t<typename remove_cvref_t<T>::LengthFieldType,
                                   typename remove_cvref_t<T>::ByteOrderType,
                                   typename remove_cvref_t<T>::AlignmentType>>
    : std::true_type {};

template <typename T>
constexpr bool props_t = props_t_impl<T>::value;

// writer
template <typename T, typename = void>
struct writer_t_impl : std::false_type {};

template <typename T>
struct writer_t_impl<T, std::void_t<decltype(std::declval<T>().write(
                            (const char *)nullptr, std::size_t{}))>>
    : std::true_type {};

template <typename T>
constexpr bool writer_t = writer_t_impl<T>::value;

// reader
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

template <typename T, typename = void>
struct view_reader_t_impl : std::false_type {};

template <typename T>
struct view_reader_t_impl<
    T, std::void_t<decltype(std::declval<T>().read_view(std::size_t{}))>>
    : std::true_type {};

template <typename T>
constexpr bool view_reader_t = reader_t<T> &&view_reader_t_impl<T>::value;

template <typename T, typename = void>
struct seek_reader_t_impl : std::false_type {};

template <typename T>
struct seek_reader_t_impl<
    T, std::void_t<decltype(std::declval<T>().seekg(std::size_t{}))>>
    : std::true_type {};

template <typename T>
constexpr bool seek_reader_t = reader_t<T> &&seek_reader_t_impl<T>::value;

template <typename T, typename = void>
struct deserialize_view_impl : std::false_type {};
template <typename T>
struct deserialize_view_impl<T, std::void_t<decltype(std::declval<T>().size()),
                                            decltype(std::declval<T>().data())>>
    : std::true_type {};

template <typename Type>
constexpr bool deserialize_view = deserialize_view_impl<Type>::value;

template <typename T, typename = void>
struct container_adapter_impl : std::false_type {};

template <typename T>
struct container_adapter_impl<
    T, std::void_t<typename remove_cvref_t<T>::value_type,
                   decltype(std::declval<T>().size()),
                   decltype(std::declval<T>().pop())>> : std::true_type {};

template <typename T>
constexpr bool container_adapter = container_adapter_impl<T>::value;

// container
template <typename T, typename = void>
struct container_impl : std::false_type {};

template <typename T>
struct container_impl<T, std::void_t<typename remove_cvref_t<T>::value_type,
                                     decltype(std::declval<T>().size()),
                                     decltype(std::declval<T>().begin()),
                                     decltype(std::declval<T>().end())>>
    : std::true_type {};

template <typename T>
constexpr bool container = container_impl<T>::value;

// is_char_t
template <typename Type>
constexpr bool is_char_t =
    std::is_same<Type, signed char>::value || std::is_same<Type, char>::value ||
    std::is_same<Type, unsigned char>::value ||
    std::is_same<Type, wchar_t>::value || std::is_same<Type, char16_t>::value ||
    std::is_same<Type, char32_t>::value;

// string
template <typename T, typename = void>
struct string_impl : std::false_type {};

template <typename T>
struct string_impl<
    T, std::void_t<
           std::enable_if_t<is_char_t<typename remove_cvref_t<T>::value_type>>,
           decltype(std::declval<T>().length()),
           decltype(std::declval<T>().data())>> : std::true_type {};

template <typename T>
constexpr bool string = string_impl<T>::value &&container<T>;

// string_view
template <typename T, typename = void>
struct string_view_impl : std::true_type {};

template <typename T>
struct string_view_impl<
    T, std::void_t<decltype(std::declval<T>().resize(std::size_t{}))>>
    : std::false_type {};

template <typename T>
constexpr bool string_view = string<T> &&string_view_impl<T>::value;

// span
template <typename T, typename = void>
struct span_impl : std::false_type {};

template <typename T>
struct span_impl<
    T, std::void_t<
           decltype(T{(typename T::value_type *)nullptr, std::size_t{}}),
           decltype(std::declval<T>().subspan(std::size_t{}, std::size_t{}))>>
    : std::true_type {};

template <typename T>
constexpr bool span = container<T> &&span_impl<T>::value;

// dynamic_span
template <typename T, typename = void>
struct dynamic_span_impl : std::false_type {};

template <typename T>
struct dynamic_span_impl<T,
                         std::void_t<std::enable_if_t<(T::extent == SIZE_MAX)>>>
    : std::true_type {};

template <typename T>
constexpr bool dynamic_span = span<T> &&dynamic_span_impl<T>::value;

// static_span
template <typename Type>
constexpr bool static_span = span<Type> && !dynamic_span<Type>;

//
template <typename Type>
static constexpr bool is_std_basic_string_v = false;

template <typename... args>
static constexpr bool is_std_basic_string_v<std::basic_string<args...>> = true;

template <typename Type>
static constexpr bool is_std_vector_v = false;

template <typename... args>
static constexpr bool is_std_vector_v<std::vector<args...>> = true;

template <typename Type>
static constexpr bool continuous_container =
    string<Type> ||
    (container<Type> && (is_std_vector_v<Type> || is_std_basic_string_v<Type>));

// map
template <typename T, typename = void>
struct has_mapped_type_impl : std::false_type {};

template <typename T>
struct has_mapped_type_impl<
    T, std::void_t<typename remove_cvref_t<T>::mapped_type>> : std::true_type {
};

template <typename T, typename = void>
struct has_key_type_impl : std::false_type {};

template <typename T>
struct has_key_type_impl<T, std::void_t<typename remove_cvref_t<T>::key_type>>
    : std::true_type {};

template <typename T>
constexpr bool map_container =
    container<T> &&has_key_type_impl<T>::value &&has_mapped_type_impl<T>::value;
// set
template <typename T>
constexpr bool set_container = container<T> &&has_key_type_impl<T>::value &&
                               !has_mapped_type_impl<T>::value;

// expect
template <typename T, typename = void>
struct has_error_type_impl : std::false_type {};

template <typename T>
struct has_error_type_impl<T,
                           std::void_t<typename remove_cvref_t<T>::error_type>>
    : std::true_type {};

template <typename T, typename = void>
struct has_unexpected_type_impl : std::false_type {};

template <typename T>
struct has_unexpected_type_impl<
    T, std::void_t<typename remove_cvref_t<T>::unexpected_type>>
    : std::true_type {};

template <typename T, typename = void>
struct expected_or_optional_impl : std::false_type {};

template <typename T>
struct expected_or_optional_impl<
    T, std::void_t<typename remove_cvref_t<T>::value_type,
                   decltype(std::declval<T>().has_value()),
                   decltype(std::declval<T>().value_or()),
                   decltype(std::declval<T>().value())>> : std::true_type {};
// TODO: check e.value()
template <typename T>
constexpr bool expected = expected_or_optional_impl<T>::value
    &&has_error_type_impl<T>::value &&has_unexpected_type_impl<T>::value;

// optional
template <typename T>
constexpr bool optional =
    !has_error_type_impl<T>::value && !has_unexpected_type_impl<T>::value &&
    expected_or_optional_impl<T>::value;

// bitset
template <typename T, typename = void>
struct bitset_impl : std::false_type {};

template <typename T>
struct bitset_impl<T, std::void_t<decltype(std::declval<T>().flip()),
                                  decltype(std::declval<T>().set()),
                                  decltype(std::declval<T>().reset()),
                                  decltype(std::declval<T>().count()),
                                  decltype(std::declval<T>().size())>>
    : std::true_type {};

template <typename T>
constexpr bool bitset = bitset_impl<T>::value;

// tuple

template <template <typename...> class Template, typename T>
struct is_instantiation_of : std::false_type {};

template <template <typename...> class Template, typename... Args>
struct is_instantiation_of<Template, Template<Args...>> : std::true_type {};

template <typename T>
constexpr bool tuple = is_instantiation_of<std::tuple, T>::value;

template <typename T>
constexpr bool variant = is_instantiation_of<mpark::variant, T>::value;

// array
template <typename T, typename = void>
struct array_impl : std::false_type {};

template <typename T>
struct array_impl<T,
                  std::void_t<decltype(std::declval<T>().size()),
                              decltype(std::tuple_size<remove_cvref_t<T>>{})>>
    : std::true_type {};

template <typename T>
constexpr bool array = array_impl<T>::value;

template <class T>
constexpr bool c_array =
    std::is_array<T>::value &&std::extent<remove_cvref_t<T>>::value > 0;

// tuple_size
template <typename T, typename = void>
struct tuple_size_impl : std::false_type {};

template <typename T>
struct tuple_size_impl<
    T, std::void_t<decltype(std::tuple_size<remove_cvref_t<T>>::value)>>
    : std::true_type {};

template <typename T>
constexpr bool tuple_size = tuple_size_impl<T>::value;

// pair
template <typename T, typename = void>
struct pair_impl : std::false_type {};

template <typename T>
struct pair_impl<T, std::void_t<typename remove_cvref_t<T>::first_type,
                                typename remove_cvref_t<T>::second_type,
                                decltype(std::declval<T>().first),
                                decltype(std::declval<T>().second)>>
    : std::true_type {};

template <typename T>
constexpr bool pair = pair_impl<T>::value;

// user struct
template <typename T>
constexpr bool user_struct =
    !container<T> && !bitset<T> && !pair<T> && !tuple<T> && !array<T> &&
    !variant<T> && std::is_class<T>::value;

template <typename T, typename = void>
struct user_defined_refl_impl : std::false_type {};

template <typename T>
struct user_defined_refl_impl<
    T, std::void_t<std::enable_if_t<
           std::is_same<decltype(std::declval<T &>()), T &>::value>>>
    : std::true_type {};

template <typename T>
constexpr bool user_defined_refl = user_defined_refl_impl<T>::value;

// is trivail
template <typename T>
constexpr bool is_trivial_tuple = false;

template <typename Type>
constexpr static bool is_trivial_view_v = false;

template <typename Type>
constexpr static bool is_trivial_view_v<trivial_view<Type>> = true;

template <typename T, typename type = remove_cvref_t<T>,
          std::enable_if_t<tuple_size<type>, int> = 0>
constexpr std::size_t members_count() {
  return std::tuple_size<type>::value;
}
template <typename T, typename type = remove_cvref_t<T>,
          std::enable_if_t<!tuple_size<type> && user_defined_refl<T>, int> = 0>
constexpr std::size_t members_count() {
  return decltype(SERIALIZE_FIELD_COUNT(std::declval<type>()))::value;
}

template <typename T, typename type = remove_cvref_t<T>,
          std::enable_if_t<!tuple_size<type> && !user_defined_refl<T>, int> = 0>
constexpr std::size_t members_count() {
  static_assert(!user_defined_refl<T>, "");
  return 0;
}

// visit members
constexpr static auto MaxVisitMembers = 64;

template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 0, int> = 0) {
  using type = remove_cvref_t<decltype(object)>;
  if (Count == 0 && user_struct<type>) {
    static_assert(!sizeof(type),
                  "1. If the struct is empty, which is not allowed in "
                  "serialize type system.\n"
                  "2. If the strut is not empty, it means can't "
                  "calculate struct members' count. ");
  }
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor();
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 1, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(SERIALIZE_GET_0(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 2, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(SERIALIZE_GET_0(object), SERIALIZE_GET_1(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 3, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(SERIALIZE_GET_0(object), SERIALIZE_GET_1(object),
                 SERIALIZE_GET_2(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 4, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(SERIALIZE_GET_0(object), SERIALIZE_GET_1(object),
                 SERIALIZE_GET_2(object), SERIALIZE_GET_3(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 5, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(SERIALIZE_GET_0(object), SERIALIZE_GET_1(object),
                 SERIALIZE_GET_2(object), SERIALIZE_GET_3(object),
                 SERIALIZE_GET_4(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 6, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(SERIALIZE_GET_0(object), SERIALIZE_GET_1(object),
                 SERIALIZE_GET_2(object), SERIALIZE_GET_3(object),
                 SERIALIZE_GET_4(object), SERIALIZE_GET_5(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 7, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(SERIALIZE_GET_0(object), SERIALIZE_GET_1(object),
                 SERIALIZE_GET_2(object), SERIALIZE_GET_3(object),
                 SERIALIZE_GET_4(object), SERIALIZE_GET_5(object),
                 SERIALIZE_GET_6(object));
}

template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 8, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(SERIALIZE_GET_0(object), SERIALIZE_GET_1(object),
                 SERIALIZE_GET_2(object), SERIALIZE_GET_3(object),
                 SERIALIZE_GET_4(object), SERIALIZE_GET_5(object),
                 SERIALIZE_GET_6(object), SERIALIZE_GET_7(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor, std::enable_if_t<Count == 9, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(SERIALIZE_GET_0(object), SERIALIZE_GET_1(object),
                 SERIALIZE_GET_2(object), SERIALIZE_GET_3(object),
                 SERIALIZE_GET_4(object), SERIALIZE_GET_5(object),
                 SERIALIZE_GET_6(object), SERIALIZE_GET_7(object),
                 SERIALIZE_GET_8(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor,
    std::enable_if_t<Count == 10, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(SERIALIZE_GET_0(object), SERIALIZE_GET_1(object),
                 SERIALIZE_GET_2(object), SERIALIZE_GET_3(object),
                 SERIALIZE_GET_4(object), SERIALIZE_GET_5(object),
                 SERIALIZE_GET_6(object), SERIALIZE_GET_7(object),
                 SERIALIZE_GET_8(object), SERIALIZE_GET_9(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor,
    std::enable_if_t<Count == 11, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(
      SERIALIZE_GET_0(object), SERIALIZE_GET_1(object), SERIALIZE_GET_2(object),
      SERIALIZE_GET_3(object), SERIALIZE_GET_4(object), SERIALIZE_GET_5(object),
      SERIALIZE_GET_6(object), SERIALIZE_GET_7(object), SERIALIZE_GET_8(object),
      SERIALIZE_GET_9(object), SERIALIZE_GET_10(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor,
    std::enable_if_t<Count == 12, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(SERIALIZE_GET_0(object), SERIALIZE_GET_1(object),
                 SERIALIZE_GET_2(object), SERIALIZE_GET_3(object),
                 SERIALIZE_GET_4(object), SERIALIZE_GET_5(object),
                 SERIALIZE_GET_6(object), SERIALIZE_GET_7(object),
                 SERIALIZE_GET_8(object), SERIALIZE_GET_9(object),
                 SERIALIZE_GET_10(object), SERIALIZE_GET_11(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor,
    std::enable_if_t<Count == 13, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(
      SERIALIZE_GET_0(object), SERIALIZE_GET_1(object), SERIALIZE_GET_2(object),
      SERIALIZE_GET_3(object), SERIALIZE_GET_4(object), SERIALIZE_GET_5(object),
      SERIALIZE_GET_6(object), SERIALIZE_GET_7(object), SERIALIZE_GET_8(object),
      SERIALIZE_GET_9(object), SERIALIZE_GET_10(object),
      SERIALIZE_GET_11(object), SERIALIZE_GET_12(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor,
    std::enable_if_t<Count == 14, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(SERIALIZE_GET_0(object), SERIALIZE_GET_1(object),
                 SERIALIZE_GET_2(object), SERIALIZE_GET_3(object),
                 SERIALIZE_GET_4(object), SERIALIZE_GET_5(object),
                 SERIALIZE_GET_6(object), SERIALIZE_GET_7(object),
                 SERIALIZE_GET_8(object), SERIALIZE_GET_9(object),
                 SERIALIZE_GET_10(object), SERIALIZE_GET_11(object),
                 SERIALIZE_GET_12(object), SERIALIZE_GET_13(object));
}
template <std::size_t Count, typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_impl(
    Object &&object, Visitor &&visitor,
    std::enable_if_t<Count == 15, int> = 0) {
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visitor(
      SERIALIZE_GET_0(object), SERIALIZE_GET_1(object), SERIALIZE_GET_2(object),
      SERIALIZE_GET_3(object), SERIALIZE_GET_4(object), SERIALIZE_GET_5(object),
      SERIALIZE_GET_6(object), SERIALIZE_GET_7(object), SERIALIZE_GET_8(object),
      SERIALIZE_GET_9(object), SERIALIZE_GET_10(object),
      SERIALIZE_GET_11(object), SERIALIZE_GET_12(object),
      SERIALIZE_GET_13(object), SERIALIZE_GET_14(object));
}

template <typename Object, typename Visitor>
constexpr decltype(auto) visit_members(Object &&object, Visitor &&visitor) {
  constexpr auto Count = decltype(SERIALIZE_FIELD_COUNT(object))::value;
  return visit_members_impl<Count>(object, visitor);
}

// get_types
template <
    typename U, typename T = remove_cvref_t<U>,
    std::enable_if_t<(std::is_fundamental<T>::value || std::is_enum<T>::value ||
                      string<T> || container<T> || optional<T> || expected<T> ||
                      array<T> || c_array<T> || bitset<T>
#if (__GNUC__ || __clang__)
                      || std::is_same<__int128, T>::value ||
                      std::is_same<unsigned __int128, T>::value
#endif
                      ) &&
                         !tuple<T>,
                     int> = 0>
constexpr auto get_types() {
  return declval<std::tuple<T>>();
}

template <
    typename U, typename T = remove_cvref_t<U>,
    std::enable_if_t<(tuple<T> || is_trivial_tuple<T>)&&!pair<T>, int> = 0>
constexpr auto get_types() {
  return declval<T>();
}

template <typename U, typename T = remove_cvref_t<U>,
          std::enable_if_t<pair<T>, int> = 0>
constexpr auto get_types() {
  return declval<std::tuple<typename T::first_type, typename T::second_type>>();
}

template <typename U, typename T = remove_cvref_t<U>,
          std::enable_if_t<std::is_class<T>::value && !container<T> &&
                               !string<T> && !tuple<T>,
                           int> = 0>
constexpr auto get_types() {
  return visit_members(declval<T>(), [](auto &&...args) {
    return declval<std::tuple<remove_cvref_t<decltype(args)>...>>();
  });
}

template <typename T, template <typename, typename, std::size_t> class Op,
          typename... Contexts, std::size_t... I>
constexpr void for_each_impl(std::index_sequence<I...>, Contexts &...contexts) {
  using type = decltype(get_types<T>());
  static_cast<void>(std::initializer_list<int>{
      (Op<T, std::tuple_element_t<I, type>, I>{}(contexts...), 0)...});
}

template <typename T, template <typename, typename, std::size_t> class Op,
          typename... Contexts>
constexpr void for_each(Contexts &...contexts) {
  using type = decltype(get_types<T>());
  for_each_impl<T, Op>(std::make_index_sequence<std::tuple_size<type>::value>(),
                       contexts...);
}

template <typename Ty>
struct is_trivial_serializable {
 private:
  template <typename U, std::size_t... I>
  static constexpr bool class_visit_helper(std::index_sequence<I...>) {
    bool ret = true;
    static_cast<void>(std::initializer_list<int>{
        (ret &= is_trivial_serializable<std::tuple_element_t<I, U>>::value,
         0)...});
    return ret;
  }

  template <typename T = Ty,
            std::enable_if_t<std::is_abstract<T>::value, int> = 0>
  static constexpr bool solve() {
    return false;
  }
  template <typename T = Ty, std::enable_if_t<is_trivial_view_v<T>, int> = 0>
  static constexpr bool solve() {
    return true;
  }
  template <typename T = Ty,
            std::enable_if_t<std::is_enum<T>::value ||
                                 std::is_fundamental<T>::value || bitset<T>,
                             int> = 0>
  static constexpr bool solve() {
    return true;
  }

  template <typename T = Ty, std::enable_if_t<array<T>, int> = 0>
  static constexpr bool solve() {
    return is_trivial_serializable<typename T::value_type>::value;
  }
  template <typename T = Ty, std::enable_if_t<c_array<T>, int> = 0>
  static constexpr bool solve() {
    return is_trivial_serializable<
        typename std::remove_all_extents<T>::type>::value;
  }
  template <
      typename T = Ty,
      std::enable_if_t<
          !pair<T> && tuple<T> && !is_trivial_tuple<T> && !array<T>, int> = 0>
  static constexpr bool solve() {
    return false;
  }
  template <typename T = Ty, std::enable_if_t<variant<T>, int> = 0>
  static constexpr bool solve() {
    return false;
  }

  // map set string vector list deque queue optional expected
  template <typename T = Ty,
            std::enable_if_t<(container<T> && !array<T>) || optional<T> ||
                                 expected<T> || container_adapter<T>,
                             int> = 0>
  static constexpr bool solve() {
    return false;
  }

  // pair
  template <typename T = Ty, std::enable_if_t<pair<T>, int> = 0>
  static constexpr bool solve() {
    return is_trivial_serializable<typename T::first_type>::value &&
           is_trivial_serializable<typename T::second_type>::value;
  }

  // tuple
  template <typename T = Ty, std::enable_if_t<is_trivial_tuple<T>, int> = 0>
  static constexpr bool solve() {
    return class_visit_helper<T>(
        std::make_index_sequence<std::tuple_size<T>::value>{});
  }

  // user defined struct
  template <typename T = Ty,
            std::enable_if_t<user_struct<T> && user_defined_refl<T>, int> = 0>
  static constexpr bool solve() {
    using U = decltype(get_types<T>());
    return class_visit_helper<U>(
        std::make_index_sequence<std::tuple_size<U>::value>{});
  }

 public:
  static constexpr bool value = is_trivial_serializable::solve();
};

template <typename T, typename = void>
struct trivially_copyable_container_impl : std::false_type {};

template <typename T>
struct trivially_copyable_container_impl<
    T, std::void_t<std::enable_if_t<
           is_trivial_serializable<typename T::value_type>::value>>>
    : std::true_type {};

template <typename Type>
constexpr bool trivially_copyable_container =
    continuous_container<Type> &&trivially_copyable_container_impl<Type>::value;

template <typename T>
constexpr bool serialize_byte =
    std::is_same<char, T>::value || std::is_same<unsigned char, T>::value ||
    std::is_same<signed char, T>::value;

template <typename T>
constexpr bool serialize_buffer =
    trivially_copyable_container<T> &&serialize_byte<typename T::value_type>;

template <typename Func, typename... Args>
constexpr decltype(auto) inline template_switch(std::size_t index,
                                                Args &&...args) {
  switch (index) {
    case 0:
      return Func::template run<0>(std::forward<Args>(args)...);
    case 1:
      return Func::template run<1>(std::forward<Args>(args)...);
    case 2:
      return Func::template run<2>(std::forward<Args>(args)...);
    case 3:
      return Func::template run<3>(std::forward<Args>(args)...);
    case 4:
      return Func::template run<4>(std::forward<Args>(args)...);
    case 5:
      return Func::template run<5>(std::forward<Args>(args)...);
    case 6:
      return Func::template run<6>(std::forward<Args>(args)...);
    case 7:
      return Func::template run<7>(std::forward<Args>(args)...);
    case 8:
      return Func::template run<8>(std::forward<Args>(args)...);
    case 9:
      return Func::template run<9>(std::forward<Args>(args)...);
    case 10:
      return Func::template run<10>(std::forward<Args>(args)...);
    case 11:
      return Func::template run<11>(std::forward<Args>(args)...);
    case 12:
      return Func::template run<12>(std::forward<Args>(args)...);
    case 13:
      return Func::template run<13>(std::forward<Args>(args)...);
    case 14:
      return Func::template run<14>(std::forward<Args>(args)...);
    case 15:
      return Func::template run<15>(std::forward<Args>(args)...);
    case 16:
      return Func::template run<16>(std::forward<Args>(args)...);
    case 17:
      return Func::template run<17>(std::forward<Args>(args)...);
    case 18:
      return Func::template run<18>(std::forward<Args>(args)...);
    case 19:
      return Func::template run<19>(std::forward<Args>(args)...);
    case 20:
      return Func::template run<20>(std::forward<Args>(args)...);
    case 21:
      return Func::template run<21>(std::forward<Args>(args)...);
    case 22:
      return Func::template run<22>(std::forward<Args>(args)...);
    case 23:
      return Func::template run<23>(std::forward<Args>(args)...);
    case 24:
      return Func::template run<24>(std::forward<Args>(args)...);
    case 25:
      return Func::template run<25>(std::forward<Args>(args)...);
    case 26:
      return Func::template run<26>(std::forward<Args>(args)...);
    case 27:
      return Func::template run<27>(std::forward<Args>(args)...);
    case 28:
      return Func::template run<28>(std::forward<Args>(args)...);
    case 29:
      return Func::template run<29>(std::forward<Args>(args)...);
    case 30:
      return Func::template run<30>(std::forward<Args>(args)...);
    case 31:
      return Func::template run<31>(std::forward<Args>(args)...);
    case 32:
      return Func::template run<32>(std::forward<Args>(args)...);
    case 33:
      return Func::template run<33>(std::forward<Args>(args)...);
    case 34:
      return Func::template run<34>(std::forward<Args>(args)...);
    case 35:
      return Func::template run<35>(std::forward<Args>(args)...);
    case 36:
      return Func::template run<36>(std::forward<Args>(args)...);
    case 37:
      return Func::template run<37>(std::forward<Args>(args)...);
    case 38:
      return Func::template run<38>(std::forward<Args>(args)...);
    case 39:
      return Func::template run<39>(std::forward<Args>(args)...);
    case 40:
      return Func::template run<40>(std::forward<Args>(args)...);
    case 41:
      return Func::template run<41>(std::forward<Args>(args)...);
    case 42:
      return Func::template run<42>(std::forward<Args>(args)...);
    case 43:
      return Func::template run<43>(std::forward<Args>(args)...);
    case 44:
      return Func::template run<44>(std::forward<Args>(args)...);
    case 45:
      return Func::template run<45>(std::forward<Args>(args)...);
    case 46:
      return Func::template run<46>(std::forward<Args>(args)...);
    case 47:
      return Func::template run<47>(std::forward<Args>(args)...);
    case 48:
      return Func::template run<48>(std::forward<Args>(args)...);
    case 49:
      return Func::template run<49>(std::forward<Args>(args)...);
    case 50:
      return Func::template run<50>(std::forward<Args>(args)...);
    case 51:
      return Func::template run<51>(std::forward<Args>(args)...);
    case 52:
      return Func::template run<52>(std::forward<Args>(args)...);
    case 53:
      return Func::template run<53>(std::forward<Args>(args)...);
    case 54:
      return Func::template run<54>(std::forward<Args>(args)...);
    case 55:
      return Func::template run<55>(std::forward<Args>(args)...);
    case 56:
      return Func::template run<56>(std::forward<Args>(args)...);
    case 57:
      return Func::template run<57>(std::forward<Args>(args)...);
    case 58:
      return Func::template run<58>(std::forward<Args>(args)...);
    case 59:
      return Func::template run<59>(std::forward<Args>(args)...);
    case 60:
      return Func::template run<60>(std::forward<Args>(args)...);
    case 61:
      return Func::template run<61>(std::forward<Args>(args)...);
    case 62:
      return Func::template run<62>(std::forward<Args>(args)...);
    case 63:
      return Func::template run<63>(std::forward<Args>(args)...);
    case 64:
      return Func::template run<64>(std::forward<Args>(args)...);
    case 65:
      return Func::template run<65>(std::forward<Args>(args)...);
    case 66:
      return Func::template run<66>(std::forward<Args>(args)...);
    case 67:
      return Func::template run<67>(std::forward<Args>(args)...);
    case 68:
      return Func::template run<68>(std::forward<Args>(args)...);
    case 69:
      return Func::template run<69>(std::forward<Args>(args)...);
    case 70:
      return Func::template run<70>(std::forward<Args>(args)...);
    case 71:
      return Func::template run<71>(std::forward<Args>(args)...);
    case 72:
      return Func::template run<72>(std::forward<Args>(args)...);
    case 73:
      return Func::template run<73>(std::forward<Args>(args)...);
    case 74:
      return Func::template run<74>(std::forward<Args>(args)...);
    case 75:
      return Func::template run<75>(std::forward<Args>(args)...);
    case 76:
      return Func::template run<76>(std::forward<Args>(args)...);
    case 77:
      return Func::template run<77>(std::forward<Args>(args)...);
    case 78:
      return Func::template run<78>(std::forward<Args>(args)...);
    case 79:
      return Func::template run<79>(std::forward<Args>(args)...);
    case 80:
      return Func::template run<80>(std::forward<Args>(args)...);
    case 81:
      return Func::template run<81>(std::forward<Args>(args)...);
    case 82:
      return Func::template run<82>(std::forward<Args>(args)...);
    case 83:
      return Func::template run<83>(std::forward<Args>(args)...);
    case 84:
      return Func::template run<84>(std::forward<Args>(args)...);
    case 85:
      return Func::template run<85>(std::forward<Args>(args)...);
    case 86:
      return Func::template run<86>(std::forward<Args>(args)...);
    case 87:
      return Func::template run<87>(std::forward<Args>(args)...);
    case 88:
      return Func::template run<88>(std::forward<Args>(args)...);
    case 89:
      return Func::template run<89>(std::forward<Args>(args)...);
    case 90:
      return Func::template run<90>(std::forward<Args>(args)...);
    case 91:
      return Func::template run<91>(std::forward<Args>(args)...);
    case 92:
      return Func::template run<92>(std::forward<Args>(args)...);
    case 93:
      return Func::template run<93>(std::forward<Args>(args)...);
    case 94:
      return Func::template run<94>(std::forward<Args>(args)...);
    case 95:
      return Func::template run<95>(std::forward<Args>(args)...);
    case 96:
      return Func::template run<96>(std::forward<Args>(args)...);
    case 97:
      return Func::template run<97>(std::forward<Args>(args)...);
    case 98:
      return Func::template run<98>(std::forward<Args>(args)...);
    case 99:
      return Func::template run<99>(std::forward<Args>(args)...);
    case 100:
      return Func::template run<100>(std::forward<Args>(args)...);
    case 101:
      return Func::template run<101>(std::forward<Args>(args)...);
    case 102:
      return Func::template run<102>(std::forward<Args>(args)...);
    case 103:
      return Func::template run<103>(std::forward<Args>(args)...);
    case 104:
      return Func::template run<104>(std::forward<Args>(args)...);
    case 105:
      return Func::template run<105>(std::forward<Args>(args)...);
    case 106:
      return Func::template run<106>(std::forward<Args>(args)...);
    case 107:
      return Func::template run<107>(std::forward<Args>(args)...);
    case 108:
      return Func::template run<108>(std::forward<Args>(args)...);
    case 109:
      return Func::template run<109>(std::forward<Args>(args)...);
    case 110:
      return Func::template run<110>(std::forward<Args>(args)...);
    case 111:
      return Func::template run<111>(std::forward<Args>(args)...);
    case 112:
      return Func::template run<112>(std::forward<Args>(args)...);
    case 113:
      return Func::template run<113>(std::forward<Args>(args)...);
    case 114:
      return Func::template run<114>(std::forward<Args>(args)...);
    case 115:
      return Func::template run<115>(std::forward<Args>(args)...);
    case 116:
      return Func::template run<116>(std::forward<Args>(args)...);
    case 117:
      return Func::template run<117>(std::forward<Args>(args)...);
    case 118:
      return Func::template run<118>(std::forward<Args>(args)...);
    case 119:
      return Func::template run<119>(std::forward<Args>(args)...);
    case 120:
      return Func::template run<120>(std::forward<Args>(args)...);
    case 121:
      return Func::template run<121>(std::forward<Args>(args)...);
    case 122:
      return Func::template run<122>(std::forward<Args>(args)...);
    case 123:
      return Func::template run<123>(std::forward<Args>(args)...);
    case 124:
      return Func::template run<124>(std::forward<Args>(args)...);
    case 125:
      return Func::template run<125>(std::forward<Args>(args)...);
    case 126:
      return Func::template run<126>(std::forward<Args>(args)...);
    case 127:
      return Func::template run<127>(std::forward<Args>(args)...);
    case 128:
      return Func::template run<128>(std::forward<Args>(args)...);
    case 129:
      return Func::template run<129>(std::forward<Args>(args)...);
    case 130:
      return Func::template run<130>(std::forward<Args>(args)...);
    case 131:
      return Func::template run<131>(std::forward<Args>(args)...);
    case 132:
      return Func::template run<132>(std::forward<Args>(args)...);
    case 133:
      return Func::template run<133>(std::forward<Args>(args)...);
    case 134:
      return Func::template run<134>(std::forward<Args>(args)...);
    case 135:
      return Func::template run<135>(std::forward<Args>(args)...);
    case 136:
      return Func::template run<136>(std::forward<Args>(args)...);
    case 137:
      return Func::template run<137>(std::forward<Args>(args)...);
    case 138:
      return Func::template run<138>(std::forward<Args>(args)...);
    case 139:
      return Func::template run<139>(std::forward<Args>(args)...);
    case 140:
      return Func::template run<140>(std::forward<Args>(args)...);
    case 141:
      return Func::template run<141>(std::forward<Args>(args)...);
    case 142:
      return Func::template run<142>(std::forward<Args>(args)...);
    case 143:
      return Func::template run<143>(std::forward<Args>(args)...);
    case 144:
      return Func::template run<144>(std::forward<Args>(args)...);
    case 145:
      return Func::template run<145>(std::forward<Args>(args)...);
    case 146:
      return Func::template run<146>(std::forward<Args>(args)...);
    case 147:
      return Func::template run<147>(std::forward<Args>(args)...);
    case 148:
      return Func::template run<148>(std::forward<Args>(args)...);
    case 149:
      return Func::template run<149>(std::forward<Args>(args)...);
    case 150:
      return Func::template run<150>(std::forward<Args>(args)...);
    case 151:
      return Func::template run<151>(std::forward<Args>(args)...);
    case 152:
      return Func::template run<152>(std::forward<Args>(args)...);
    case 153:
      return Func::template run<153>(std::forward<Args>(args)...);
    case 154:
      return Func::template run<154>(std::forward<Args>(args)...);
    case 155:
      return Func::template run<155>(std::forward<Args>(args)...);
    case 156:
      return Func::template run<156>(std::forward<Args>(args)...);
    case 157:
      return Func::template run<157>(std::forward<Args>(args)...);
    case 158:
      return Func::template run<158>(std::forward<Args>(args)...);
    case 159:
      return Func::template run<159>(std::forward<Args>(args)...);
    case 160:
      return Func::template run<160>(std::forward<Args>(args)...);
    case 161:
      return Func::template run<161>(std::forward<Args>(args)...);
    case 162:
      return Func::template run<162>(std::forward<Args>(args)...);
    case 163:
      return Func::template run<163>(std::forward<Args>(args)...);
    case 164:
      return Func::template run<164>(std::forward<Args>(args)...);
    case 165:
      return Func::template run<165>(std::forward<Args>(args)...);
    case 166:
      return Func::template run<166>(std::forward<Args>(args)...);
    case 167:
      return Func::template run<167>(std::forward<Args>(args)...);
    case 168:
      return Func::template run<168>(std::forward<Args>(args)...);
    case 169:
      return Func::template run<169>(std::forward<Args>(args)...);
    case 170:
      return Func::template run<170>(std::forward<Args>(args)...);
    case 171:
      return Func::template run<171>(std::forward<Args>(args)...);
    case 172:
      return Func::template run<172>(std::forward<Args>(args)...);
    case 173:
      return Func::template run<173>(std::forward<Args>(args)...);
    case 174:
      return Func::template run<174>(std::forward<Args>(args)...);
    case 175:
      return Func::template run<175>(std::forward<Args>(args)...);
    case 176:
      return Func::template run<176>(std::forward<Args>(args)...);
    case 177:
      return Func::template run<177>(std::forward<Args>(args)...);
    case 178:
      return Func::template run<178>(std::forward<Args>(args)...);
    case 179:
      return Func::template run<179>(std::forward<Args>(args)...);
    case 180:
      return Func::template run<180>(std::forward<Args>(args)...);
    case 181:
      return Func::template run<181>(std::forward<Args>(args)...);
    case 182:
      return Func::template run<182>(std::forward<Args>(args)...);
    case 183:
      return Func::template run<183>(std::forward<Args>(args)...);
    case 184:
      return Func::template run<184>(std::forward<Args>(args)...);
    case 185:
      return Func::template run<185>(std::forward<Args>(args)...);
    case 186:
      return Func::template run<186>(std::forward<Args>(args)...);
    case 187:
      return Func::template run<187>(std::forward<Args>(args)...);
    case 188:
      return Func::template run<188>(std::forward<Args>(args)...);
    case 189:
      return Func::template run<189>(std::forward<Args>(args)...);
    case 190:
      return Func::template run<190>(std::forward<Args>(args)...);
    case 191:
      return Func::template run<191>(std::forward<Args>(args)...);
    case 192:
      return Func::template run<192>(std::forward<Args>(args)...);
    case 193:
      return Func::template run<193>(std::forward<Args>(args)...);
    case 194:
      return Func::template run<194>(std::forward<Args>(args)...);
    case 195:
      return Func::template run<195>(std::forward<Args>(args)...);
    case 196:
      return Func::template run<196>(std::forward<Args>(args)...);
    case 197:
      return Func::template run<197>(std::forward<Args>(args)...);
    case 198:
      return Func::template run<198>(std::forward<Args>(args)...);
    case 199:
      return Func::template run<199>(std::forward<Args>(args)...);
    case 200:
      return Func::template run<200>(std::forward<Args>(args)...);
    case 201:
      return Func::template run<201>(std::forward<Args>(args)...);
    case 202:
      return Func::template run<202>(std::forward<Args>(args)...);
    case 203:
      return Func::template run<203>(std::forward<Args>(args)...);
    case 204:
      return Func::template run<204>(std::forward<Args>(args)...);
    case 205:
      return Func::template run<205>(std::forward<Args>(args)...);
    case 206:
      return Func::template run<206>(std::forward<Args>(args)...);
    case 207:
      return Func::template run<207>(std::forward<Args>(args)...);
    case 208:
      return Func::template run<208>(std::forward<Args>(args)...);
    case 209:
      return Func::template run<209>(std::forward<Args>(args)...);
    case 210:
      return Func::template run<210>(std::forward<Args>(args)...);
    case 211:
      return Func::template run<211>(std::forward<Args>(args)...);
    case 212:
      return Func::template run<212>(std::forward<Args>(args)...);
    case 213:
      return Func::template run<213>(std::forward<Args>(args)...);
    case 214:
      return Func::template run<214>(std::forward<Args>(args)...);
    case 215:
      return Func::template run<215>(std::forward<Args>(args)...);
    case 216:
      return Func::template run<216>(std::forward<Args>(args)...);
    case 217:
      return Func::template run<217>(std::forward<Args>(args)...);
    case 218:
      return Func::template run<218>(std::forward<Args>(args)...);
    case 219:
      return Func::template run<219>(std::forward<Args>(args)...);
    case 220:
      return Func::template run<220>(std::forward<Args>(args)...);
    case 221:
      return Func::template run<221>(std::forward<Args>(args)...);
    case 222:
      return Func::template run<222>(std::forward<Args>(args)...);
    case 223:
      return Func::template run<223>(std::forward<Args>(args)...);
    case 224:
      return Func::template run<224>(std::forward<Args>(args)...);
    case 225:
      return Func::template run<225>(std::forward<Args>(args)...);
    case 226:
      return Func::template run<226>(std::forward<Args>(args)...);
    case 227:
      return Func::template run<227>(std::forward<Args>(args)...);
    case 228:
      return Func::template run<228>(std::forward<Args>(args)...);
    case 229:
      return Func::template run<229>(std::forward<Args>(args)...);
    case 230:
      return Func::template run<230>(std::forward<Args>(args)...);
    case 231:
      return Func::template run<231>(std::forward<Args>(args)...);
    case 232:
      return Func::template run<232>(std::forward<Args>(args)...);
    case 233:
      return Func::template run<233>(std::forward<Args>(args)...);
    case 234:
      return Func::template run<234>(std::forward<Args>(args)...);
    case 235:
      return Func::template run<235>(std::forward<Args>(args)...);
    case 236:
      return Func::template run<236>(std::forward<Args>(args)...);
    case 237:
      return Func::template run<237>(std::forward<Args>(args)...);
    case 238:
      return Func::template run<238>(std::forward<Args>(args)...);
    case 239:
      return Func::template run<239>(std::forward<Args>(args)...);
    case 240:
      return Func::template run<240>(std::forward<Args>(args)...);
    case 241:
      return Func::template run<241>(std::forward<Args>(args)...);
    case 242:
      return Func::template run<242>(std::forward<Args>(args)...);
    case 243:
      return Func::template run<243>(std::forward<Args>(args)...);
    case 244:
      return Func::template run<244>(std::forward<Args>(args)...);
    case 245:
      return Func::template run<245>(std::forward<Args>(args)...);
    case 246:
      return Func::template run<246>(std::forward<Args>(args)...);
    case 247:
      return Func::template run<247>(std::forward<Args>(args)...);
    case 248:
      return Func::template run<248>(std::forward<Args>(args)...);
    case 249:
      return Func::template run<249>(std::forward<Args>(args)...);
    case 250:
      return Func::template run<250>(std::forward<Args>(args)...);
    case 251:
      return Func::template run<251>(std::forward<Args>(args)...);
    case 252:
      return Func::template run<252>(std::forward<Args>(args)...);
    case 253:
      return Func::template run<253>(std::forward<Args>(args)...);
    case 254:
      return Func::template run<254>(std::forward<Args>(args)...);
    case 255:
      return Func::template run<255>(std::forward<Args>(args)...);
    default:
      unreachable();
      // index shouldn't bigger than 256
  }
}
}  // namespace detail
}  // namespace serialize

// clang-format off

#define SERIALIZE_RETURN_ELEMENT(Idx, Type, X) \
template<std::size_t I, std::enable_if_t<I==Idx, int> = 0> \
constexpr auto& FuncImpl(Type& c){return c.X;} \

#define SERIALIZE_RETURN_ELEMENT_CONST(Idx, Type, X) \
template<std::size_t I, std::enable_if_t<I==Idx, int> = 0> \
constexpr const auto& FuncImpl(const Type& c){return c.X;} \

#define SERIALIZE_GET_INDEX(Idx, Type,X) \
inline auto& SERIALIZE_GET_##Idx(Type& c) {\
    return SERIALIZE_GET<SERIALIZE_FIELD_COUNT_IMPL<Type>()-1-Idx,Idx>(c);\
}\

#define SERIALIZE_GET_INDEX_CONST(Idx, Type,X) \
inline const auto& SERIALIZE_GET_##Idx(const Type& c) {\
    return SERIALIZE_GET<SERIALIZE_FIELD_COUNT_IMPL<Type>()-1-Idx,Idx>(c);\
}\

#define SERIALIZE_REFL(Type,...) \
inline Type& SERIALIZE_REFL_FLAG(Type& t) {return t;} \
template<typename T> \
constexpr std::size_t SERIALIZE_FIELD_COUNT_IMPL(); \
template<> \
constexpr std::size_t SERIALIZE_FIELD_COUNT_IMPL<Type>() {return SERIALIZE_ARG_COUNT(__VA_ARGS__);} \
inline decltype(auto) SERIALIZE_FIELD_COUNT(const Type &){ \
  return std::integral_constant<std::size_t,SERIALIZE_ARG_COUNT(__VA_ARGS__)>{}; \
} \
SERIALIZE_EXPAND_EACH(,SERIALIZE_RETURN_ELEMENT, Type,__VA_ARGS__) \
SERIALIZE_EXPAND_EACH(,SERIALIZE_RETURN_ELEMENT_CONST,Type, __VA_ARGS__) \
template<std::size_t I, std::size_t Idx> auto& SERIALIZE_GET(Type& c) { \
  return FuncImpl<I>(c);\
} \
template<std::size_t I, std::size_t Idx> const auto& SERIALIZE_GET(const Type& c) { \
  return FuncImpl<I>(c);\
} \
SERIALIZE_EXPAND_EACH(,SERIALIZE_GET_INDEX,Type,__VA_ARGS__) \
SERIALIZE_EXPAND_EACH(,SERIALIZE_GET_INDEX_CONST,Type,__VA_ARGS__) \

#define SERIALIZE_FRIEND_DECL(Type) \
template <std::size_t I> \
friend auto& SERIALIZE_GET(Type& c); \
template <std::size_t I> \
friend const auto& SERIALIZE_GET(const Type& c);


