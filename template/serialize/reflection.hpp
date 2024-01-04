/*
 * @Description:
 * @Version: 2.0
 * @Author: chen, hua
 * @Date: 2023-11-28 16:41:13
 * @LastEditors: chen, hua
 * @LastEditTime: 2023-12-27 22:30:22
 */
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <type_traits>
#include <vector>

#include "compatible.hpp"
#include "trivial_view.hpp"
#include "utils.hpp"

#ifndef SERIALIZE_DETAILS_REFLECTION_HPP_
#define SERIALIZE_DETAILS_REFLECTION_HPP_

// clang-format off


#define STRUCT_PACK_RETURN_ELEMENT(Idx, X) \
if constexpr (Idx == I) {\
    return c.X;\
}\

#define STRUCT_PACK_GET_INDEX(Idx, Type) \
inline auto& STRUCT_PACK_GET_##Idx(Type& c) {\
    return STRUCT_PACK_GET<STRUCT_PACK_FIELD_COUNT_IMPL<Type>()-1-Idx>(c);\
}\

#define STRUCT_PACK_GET_INDEX_CONST(Idx, Type) \
inline const auto& STRUCT_PACK_GET_##Idx(const Type& c) {\
    return STRUCT_PACK_GET<STRUCT_PACK_FIELD_COUNT_IMPL<Type>()-1-Idx>(c);\
}\

#define STRUCT_PACK_REFL(Type,...) \
inline Type& STRUCT_PACK_REFL_FLAG(Type& t) {return t;} \
template<typename T> \
constexpr std::size_t STRUCT_PACK_FIELD_COUNT_IMPL(); \
template<> \
constexpr std::size_t STRUCT_PACK_FIELD_COUNT_IMPL<Type>() {return STRUCT_PACK_ARG_COUNT(__VA_ARGS__);} \
inline decltype(auto) STRUCT_PACK_FIELD_COUNT(const Type &){ \
  return std::integral_constant<std::size_t,STRUCT_PACK_ARG_COUNT(__VA_ARGS__)>{}; \
} \
template<std::size_t I> auto& STRUCT_PACK_GET(Type& c) { \
  STRUCT_PACK_EXPAND_EACH(,STRUCT_PACK_RETURN_ELEMENT,__VA_ARGS__) \
  else { \
	static_assert(I < STRUCT_PACK_FIELD_COUNT_IMPL<Type>()); \
  } \
} \
template<std::size_t I> const auto& STRUCT_PACK_GET(const Type& c) { \
  STRUCT_PACK_EXPAND_EACH(,STRUCT_PACK_RETURN_ELEMENT,__VA_ARGS__) \
  else { \
	static_assert(I < STRUCT_PACK_FIELD_COUNT_IMPL<Type>()); \
  } \
} \
STRUCT_PACK_EXPAND_EACH(,STRUCT_PACK_GET_INDEX,STRUCT_PACK_MAKE_ARGS(Type,STRUCT_PACK_ARG_COUNT(__VA_ARGS__))) \
STRUCT_PACK_EXPAND_EACH(,STRUCT_PACK_GET_INDEX_CONST,STRUCT_PACK_MAKE_ARGS(Type,STRUCT_PACK_ARG_COUNT(__VA_ARGS__))) \

#define STRUCT_PACK_FRIEND_DECL(Type) \
template <std::size_t I> \
friend auto& STRUCT_PACK_GET(Type& c); \
template <std::size_t I> \
friend const auto& STRUCT_PACK_GET(const Type& c);

// clang-format on

namespace serialize {

enum sp_config {
  DEFAULT = 0,
  DISABLE_TYPE_INFO = 0b1,
  ENABLE_TYPE_INFO = 0b10,
  DISABLE_ALL_META_INFO = 0b11
};

namespace details {
template <typename... Args>
using get_args_type = remove_cvref_t<typename std::conditional<
    sizeof...(Args) == 1, std::tuple_element_t<0, std::tuple<Args...>>,
    std::tuple<Args...>>::type>;

// template <typename T>
// constexpr std::size_t members_count();
// template <typename T>
// constexpr std::size_t pack_align();
// template <typename T>
// constexpr std::size_t alignment();
}  // namespace details

namespace details {

template <typename T, typename = void>
struct deserialize_view_impl : std::false_type {};
template <typename T>
struct deserialize_view_impl<T, std::void_t<decltype(std::declval<T>().size()),
                                            decltype(std::declval<T>().data())>>
    : std::true_type {};

template <typename Type>
constexpr bool deserialize_view = deserialize_view_impl<Type>::value;

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
struct has_user_defined_id_impl : std::false_type {};

template <typename T>
struct has_user_defined_id_impl<
    T, std::void_t<std::integral_constant<std::size_t, T::struct_pack_id>>>
    : std::true_type {};

template <typename T>
constexpr bool has_user_defined_id = has_user_defined_id_impl<T>::value;

template <std::size_t sz>
struct constant_checker {};

template <typename T, typename = void>
struct has_user_defined_id_ADL_impl : std::false_type {};

template <typename T>
struct has_user_defined_id_ADL_impl<
    T, std::void_t<constant_checker<struct_pack_id((T *)nullptr)>>>
    : std::true_type {};

template <typename T>
constexpr bool has_user_defined_id_ADL = has_user_defined_id_ADL_impl<T>::value;

// is_base_class
// template <typename T, typename = void>
// struct is_base_class_impl : std::false_type {};

// template <typename T>
// struct is_base_class_impl<
//     T, std::void_t<
//            std::enable_if<std::is_same<
//                decltype(((T*)nullptr)->get_struct_pack_id()),
//                uint32_t>::value>,
//            typename struct_pack::detail::derived_class_set_t<T>>>
//     : std::true_type {};
// template <typename T>
// constexpr bool is_base_class = is_base_class_impl<T>::value;

// container_adapter
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

template <typename T, std::enable_if_t<bitset_impl<T>::value, int> = 0>
constexpr bool bitset_size_checker() {
  return (T{}.size() + 7) / 8 == sizeof(T);
}

template <typename T, std::enable_if_t<!bitset_impl<T>::value, int> = 0>
constexpr bool bitset_size_checker() {
  return false;
}

template <typename T>
constexpr bool bitset = bitset_impl<T>::value &&bitset_size_checker<T>();

// tuple
template <typename T, typename = void>
struct tuple_impl : std::false_type {};

template <typename T>
struct tuple_impl<
    T, std::void_t<decltype(std::get<0>(std::declval<T>())),
                   decltype(sizeof(std::tuple_size<remove_cvref_t<T>>::value))>>
    : std::true_type {};

template <typename T>
constexpr bool tuple = tuple_impl<T>::value;

//
template <typename T, typename = void>
struct user_defined_refl_impl : std::false_type {};

template <typename T>
struct user_defined_refl_impl<
    T, std::void_t<std::enable_if_t<
           std::is_same<decltype(std::declval<T &>()), T &>::value>>>
    : std::true_type {};

template <typename T>
constexpr bool user_defined_refl = user_defined_refl_impl<T>::value;

// tuple_size
template <typename T, typename = void>
struct tuple_size_impl : std::false_type {};

template <typename T>
struct tuple_size_impl<
    T, std::void_t<decltype(std::tuple_size<remove_cvref_t<T>>::value)>>
    : std::true_type {};

template <typename T>
constexpr bool tuple_size = tuple_size_impl<T>::value;
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

// expect
template <typename T, typename = void>
struct expected_impl : std::false_type {};

template <typename T>
struct expected_impl<T, std::void_t<typename remove_cvref_t<T>::value_type,
                                    typename remove_cvref_t<T>::error_type,
                                    typename remove_cvref_t<T>::unexpected_type,
                                    decltype(std::declval<T>().has_value()),
                                    decltype(std::declval<T>().error())>>
    : std::true_type {};
// TODO: check e.value()
template <typename T>
constexpr bool expected = expected_impl<T>::value;

// unique_ptr
template <typename T, typename = void>
struct unique_ptr_impl : std::false_type {};

template <typename T>
struct unique_ptr_impl<T, std::void_t<typename remove_cvref_t<T>::element_type,
                                      decltype(std::declval<T>().operator*())>>
    : std::true_type {};

template <typename T>
constexpr bool unique_ptr = unique_ptr_impl<T>::value;

// optional
template <typename T, typename = void>
struct optional_impl : std::false_type {};

template <typename T>
struct optional_impl<T, std::void_t<decltype(std::declval<T>().value()),
                                    decltype(std::declval<T>().has_value()),
                                    decltype(std::declval<T>().operator*()),
                                    typename remove_cvref_t<T>::value_type>>
    : std::true_type {};

template <typename T>
constexpr bool optional = !expected<T> && optional_impl<T>::value;

// template <typename Type>
// constexpr bool is_compatible_v = false;

// template <typename Type, uint64_t version>
// constexpr bool is_compatible_v<types::compatible<Type, version>> = true;

// template <typename Type>
// constexpr bool is_variant_v = false;

// template <typename... args>
// static constexpr bool is_variant_v<std::variant<args...>> = true;

template <typename T>
constexpr bool is_trivial_tuple = false;

template <typename T>
class varint;

template <typename T>
class sint;

template <typename T>
constexpr bool varintable_t = std::is_same<T, varint<int32_t>>::value ||
                              std::is_same<T, varint<int64_t>>::value ||
                              std::is_same<T, varint<uint32_t>>::value ||
                              std::is_same<T, varint<uint64_t>>::value;
template <typename T>
constexpr bool sintable_t = std::is_same<T, sint<int32_t>>::value ||
                            std::is_same<T, sint<int64_t>>::value;

template <typename T>
constexpr bool varint_t = varintable_t<T> || sintable_t<T>;

template <typename Type>
static constexpr bool is_trivial_view_v = false;

struct UniversalVectorType {
  template <typename T>
  operator std::vector<T>();
};

struct UniversalType {
  template <typename T>
  operator T();
};

struct UniversalIntegralType {
  template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
  operator T();
};

struct UniversalNullptrType {
  operator std::nullptr_t();
};

struct UniversalOptionalType {
  template <typename U, typename = std::enable_if_t<optional<U>>>
  operator U();
};

struct UniversalCompatibleType {
  template <typename U, typename = std::enable_if_t<is_compatible_v<U>>>
  operator U();
};

template <typename T, typename construct_param_t, typename = void,
          typename... Args>
struct is_constructable_impl : std::false_type {};

template <typename T, typename construct_param_t, typename... Args>
struct is_constructable_impl<
    T, construct_param_t,
    std::void_t<decltype(T{{Args{}}..., {construct_param_t{}}})>, Args...>
    : std::true_type {};

template <typename T, typename construct_param_t, typename... Args>
constexpr bool is_constructable =
    is_constructable_impl<T, construct_param_t, void, Args...>::value;

template <typename T, typename... Args,
          std::enable_if_t<is_constructable<T, UniversalVectorType, Args...>,
                           int> = 0>
constexpr std::size_t members_count_impl() {
  return members_count_impl<T, Args..., UniversalVectorType>();
}

template <typename T, typename... Args,
          std::enable_if_t<is_constructable<T, UniversalOptionalType, Args...>,
                           int> = 0>
constexpr std::size_t members_count_impl() {
  return members_count_impl<T, Args..., UniversalOptionalType>();
}

// template <
//     typename T, typename... Args,
//     std::enable_if_t<is_constructable<T, UniversalType, Args...>, int> = 0>
// constexpr std::size_t members_count_impl() {
//   return members_count_impl<T, Args..., UniversalType>();
// }

template <typename T, typename... Args,
          std::enable_if_t<is_constructable<T, UniversalIntegralType, Args...>,
                           int> = 0>
constexpr std::size_t members_count_impl() {
  return members_count_impl<T, Args..., UniversalIntegralType>();
}

template <typename T, typename... Args,
          std::enable_if_t<is_constructable<T, UniversalNullptrType, Args...>,
                           int> = 0>
constexpr std::size_t members_count_impl() {
  return members_count_impl<T, Args..., UniversalNullptrType>();
}

template <typename T, typename... Args,
          std::enable_if_t<
              is_constructable<T, UniversalCompatibleType, Args...>, int> = 0>
constexpr std::size_t members_count_impl() {
  return members_count_impl<T, Args..., UniversalCompatibleType>();
}

template <
    typename T, typename... Args,
    std::enable_if_t<!(is_constructable<T, UniversalCompatibleType, Args...> ||
                       is_constructable<T, UniversalVectorType, Args...> ||
                       is_constructable<T, UniversalType, Args...> ||
                       is_constructable<T, UniversalOptionalType, Args...> ||
                       is_constructable<T, UniversalIntegralType, Args...> ||
                       is_constructable<T, UniversalNullptrType>),
                     int> = 0>
constexpr std::size_t members_count_impl() {
  return sizeof...(Args);
}

template <typename T, typename type = remove_cvref_t<T>,
          std::enable_if_t<user_defined_refl<type>, int> = 0>
constexpr std::size_t members_count() {
  return decltype(STRUCT_PACK_FIELD_COUNT(std::declval<type>()))::value;
}

template <typename T, typename type = remove_cvref_t<T>,
          std::enable_if_t<tuple_size<type>, int> = 0>
constexpr std::size_t members_count() {
  return std::tuple_size<type>::value;
}

template <
    typename T, typename type = remove_cvref_t<T>,
    std::enable_if_t<!tuple_size<type> && !user_defined_refl<type>, int> = 0>
constexpr std::size_t members_count() {
  return members_count_impl<type>();
}

}  // namespace details

template <typename T>
constexpr std::size_t members_count = details::members_count<T>();
template <typename T>
constexpr std::size_t pack_alignment_v = 0;
template <typename T>
constexpr std::size_t alignment_v = 0;

namespace details {

constexpr static auto MaxVisitMembers = 64;

template <typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_by_structure_binding(
    Object &&object, Visitor &&visitor,
    std::enable_if_t<
        serialize::members_count<remove_cvref_t<decltype(object)>> == 0, int> =
        0) {
  return visitor();
}
template <typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_by_structure_binding(
    Object &&object, Visitor &&visitor,
    std::enable_if_t<
        serialize::members_count<remove_cvref_t<decltype(object)>> == 1, int> =
        0) {
  decltype(object.template get<0>()) a1;
  // std::tie(a1) = object;
  // return visitor(a1);
  return visitor();
}
template <typename Object, typename Visitor>
constexpr decltype(auto) inline visit_members_by_structure_binding(
    Object &&object, Visitor &&visitor,
    std::enable_if_t<
        serialize::members_count<remove_cvref_t<decltype(object)>> == 2, int> =
        0) {
  // decltype(object.template get<0>()) a1;
  // decltype(object.template get<1>()) a2;
  // std::tie(a1, a2) = object;
  // return visitor(a1, a2);
  return visitor();
}

template <typename Object, typename Visitor, std::size_t Count,
          std::enable_if_t<Count == 1, int> = 0>
constexpr decltype(auto) visit_members_by_user_defined_refl_impl(
    Object &&object, Visitor &&visitor) {
  return visitor(STRUCT_PACK_GET_0(object));
}

template <typename Object, typename Visitor, std::size_t Count,
          std::enable_if_t<Count == 2, int> = 0>
constexpr decltype(auto) visit_members_by_user_defined_refl_impl(
    Object &&object, Visitor &&visitor) {
  return visitor(STRUCT_PACK_GET_0(object));
}

template <typename Object, typename Visitor, std::size_t Count,
          std::enable_if_t<Count == 3, int> = 0>
constexpr decltype(auto) visit_members_by_user_defined_refl_impl(
    Object &&object, Visitor &&visitor) {
  return visitor(STRUCT_PACK_GET_0(object));
}

template <typename Object, typename Visitor>
constexpr decltype(auto) visit_members_by_user_defined_refl(Object &&object,
                                                            Visitor &&visitor) {
  // using type = remove_cvref_t<decltype(object)>;
  constexpr auto Count = decltype(STRUCT_PACK_FIELD_COUNT(object))::value;
  static_assert(Count <= MaxVisitMembers, "exceed max visit members");
  return visit_members_by_user_defined_refl_impl<Object, Visitor, Count>(
      object, visitor);
}

template <typename Object, typename Visitor,
          std::enable_if_t<!user_defined_refl<remove_cvref_t<Object>>, int> = 0>
constexpr decltype(auto) visit_members(Object &&object, Visitor &&visitor) {
  return visit_members_by_structure_binding(object, visitor);
}

template <typename Object, typename Visitor,
          std::enable_if_t<user_defined_refl<remove_cvref_t<Object>>, int> = 0>
constexpr decltype(auto) visit_members(Object &&object, Visitor &&visitor) {
  return visit_members_by_user_defined_refl(object, visitor);
}

template <
    typename U, typename T = remove_cvref_t<U>,
    std::enable_if_t<(std::is_fundamental<T>::value || std::is_enum<T>::value ||
                      varint_t<T> || string<T> || container<T> || optional<T> ||
                      unique_ptr<T> || is_variant_v<T> || expected<T> ||
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

template <typename U, typename T = remove_cvref_t<U>,
          std::enable_if_t<tuple<T> || is_trivial_tuple<T>, int> = 0>
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

template <typename Ty, bool ignore_compatible_field = false>
struct is_trivial_serializable {
 private:
  template <typename U, std::size_t... I>
  static constexpr bool class_visit_helper(std::index_sequence<I...>) {
    std::initializer_list<bool> list{
        (0, is_trivial_serializable<std::tuple_element_t<I, U>,
                                    ignore_compatible_field>::value)...};
    bool ret = true;
    for (auto i = list.begin(); i != list.end(); ++i) {
      ret = ret & (*i);
    }
    return ret;
  }

  template <typename T = Ty,
            std::enable_if_t<std::is_abstract<T>::value, int> = 0>
  static constexpr bool solve() {
    return false;
  }
  template <
      typename T = Ty,
      std::enable_if_t<is_compatible_v<T> || is_trivial_view_v<T>, int> = 0>
  static constexpr bool solve() {
    return ignore_compatible_field;
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
    return is_trivial_serializable<typename T::value_type,
                                   ignore_compatible_field>::value;
  }
  template <typename T = Ty, std::enable_if_t<c_array<T>, int> = 0>
  static constexpr bool solve() {
    return is_trivial_serializable<typename std::remove_all_extents<T>::type,
                                   ignore_compatible_field>::value;
  }
  template <
      typename T = Ty,
      std::enable_if_t<
          !pair<T> && tuple<T> && !is_trivial_tuple<T> && !array<T>, int> = 0>
  static constexpr bool solve() {
    return false;
  }
  template <typename T = Ty, std::enable_if_t<user_defined_refl<T>>>
  static constexpr bool solve() {
    return false;
  }
  template <
      typename T = Ty,
      std::enable_if_t<(container<T> && !array<T>) || optional<T> ||
                           is_variant_v<T> || unique_ptr<T> || expected<T> ||
                           container_adapter<T> || varint_t<T>,
                       int> = 0>
  static constexpr bool solve() {
    return false;
  }
  template <typename T = Ty, std::enable_if_t<pair<T>, int> = 0>
  static constexpr bool solve() {
    return is_trivial_serializable<typename T::first_type,
                                   ignore_compatible_field>::value &&
           is_trivial_serializable<typename T::second_type,
                                   ignore_compatible_field>::value;
  }
  template <typename T = Ty, std::enable_if_t<is_trivial_tuple<T>, int> = 0>
  static constexpr bool solve() {
    return class_visit_helper<T>(
        std::make_index_sequence<std::tuple_size<T>::value>{});
  }

  template <typename T = Ty,
            std::enable_if_t<std::is_class<T>::value &&
                                 !(array<T> || map_container<T> || pair<T> ||
                                   tuple<T>),
                             int> = 0>
  static constexpr bool solve() {
    using U = decltype(get_types<T>());
    return class_visit_helper<U>(
        std::make_index_sequence<std::tuple_size<U>::value>{});
  }

 public:
  static constexpr bool value = is_trivial_serializable::solve();
};

// trivially_copyable_container
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

template <typename Type>
constexpr bool is_trivial_view_v<serialize::trivial_view<Type>> = true;

template <typename T, template <typename, typename, std::size_t> class Op,
          typename... Contexts, std::size_t... I>
constexpr void for_each_impl(std::index_sequence<I...>, Contexts &...contexts) {
  using type = decltype(get_types<T>());
  std::initializer_list<int>{
      (Op<T, std::tuple_element_t<I, type>, I>{}(contexts...), 0)...};
  // (Op<T, std::tuple_element_t<I, type>, I>{}(contexts...), ...);
}

template <typename T, template <typename, typename, std::size_t> class Op,
          typename... Contexts>
constexpr void for_each(Contexts &...contexts) {
  using type = decltype(get_types<T>());
  for_each_impl<T, Op>(std::make_index_sequence<std::tuple_size<type>::value>(),
                       contexts...);
}
}  // namespace details
}  // namespace serialize

// co

#endif
